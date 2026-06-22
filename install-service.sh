#!/bin/bash

set -e

APPDIR="$HOME/.local/share/pomodoro-timer"
BINDIR="$APPDIR/bin"
LOGDIR="$APPDIR/logs"
SOUNDDIR="$APPDIR/sounds"
CONFIGDIR="$HOME/.config/pomodoro-timer"
SYSTEMD_USER_DIR="$HOME/.config/systemd/user"

# Create directories
mkdir -p "$BINDIR" "$LOGDIR" "$SOUNDDIR" "$CONFIGDIR" "$SYSTEMD_USER_DIR"

# Copy binary, config, and sounds
cp ./pomodoro "$BINDIR/"
cp ./overlay_timer_qt "$BINDIR/" 2>/dev/null || echo "overlay_timer_qt not built (daemon will use native lock only)"
# Deploy the helper scripts used by the daemon
cp ./scripts/pomodoro-session.sh "$BINDIR/"
cp ./scripts/pomodoro-notify.sh "$BINDIR/"
chmod +x "$BINDIR/pomodoro-session.sh" "$BINDIR/pomodoro-notify.sh"
# Always overwrite the config file with the latest version
cp ./config/settings.cfg "$CONFIGDIR/"

# Deploy notifier credentials if present (gitignored). The wrapper looks here too.
if [ -f ./.env ]; then
    cp ./.env "$CONFIGDIR/.env"
    chmod 600 "$CONFIGDIR/.env"
    echo "Copied .env to $CONFIGDIR/.env"
else
    echo "No ./.env found -- notifications are off until you create one (see .env.example)."
fi

# Build the custom Alpine notifier image (HTML email + ntfy). Skips cleanly without docker.
if command -v docker >/dev/null 2>&1; then
    docker build -t pomodoro-notifier:latest ./notifier \
        && echo "Built notifier image: pomodoro-notifier:latest" \
        || echo "Notifier image build failed -- notifications will be skipped."
else
    echo "docker not found -- skipping notifier image (notifications disabled)."
fi

# Best-effort: install the PAM service for the overlay's password-skip (needs sudo).
# Pick the include that exists on this distro (Debian: common-auth, Fedora/Arch: system-auth).
if [ ! -f /etc/pam.d/pomodoro ]; then
    if [ -f /etc/pam.d/system-auth ]; then PAM_INC=system-auth; else PAM_INC=common-auth; fi
    if printf 'auth     include    %s\naccount  include    %s\n' "$PAM_INC" "$PAM_INC" \
        | sudo -n tee /etc/pam.d/pomodoro >/dev/null 2>&1; then
        echo "Installed /etc/pam.d/pomodoro (overlay password-skip enabled)."
    else
        echo "Skipped /etc/pam.d/pomodoro (no passwordless sudo). The overlay falls back to"
        echo "system PAM services; to enable cleanly run later:"
        echo "  sudo cp ./pam/pomodoro /etc/pam.d/pomodoro"
    fi
fi

# Copy sound assets from the collection_sounds submodule (or ./sounds symlink).
SOUND_SRC=""
if [ -d "./collection_sounds" ]; then SOUND_SRC="./collection_sounds"
elif [ -d "./sounds" ]; then SOUND_SRC="./sounds"; fi
if [ -n "$SOUND_SRC" ] && cp "$SOUND_SRC"/*.wav "$SOUNDDIR/" 2>/dev/null; then
    echo "Installed sound files from $SOUND_SRC"
else
    echo "No .wav sounds found -- run 'git submodule update --init' for the sound pack."
fi

# Create systemd user service file
SERVICE_FILE="$SYSTEMD_USER_DIR/pomodoro-timer.service"
cat > "$SERVICE_FILE" <<EOF
[Unit]
Description=Pomodoro Timer Daemon

[Service]
ExecStart=$BINDIR/pomodoro --daemon --config $CONFIGDIR/settings.cfg
Restart=on-failure
WorkingDirectory=$BINDIR
Environment="PULSE_SERVER=unix:/run/user/%U/pulse/native"
Environment="XDG_RUNTIME_DIR=/run/user/%U"
Environment="DBUS_SESSION_BUS_ADDRESS=unix:path=/run/user/%U/bus"
Environment="DISPLAY=${DISPLAY:-:0}"

[Install]
WantedBy=default.target
EOF

# Make the live graphical-session env (DISPLAY/XAUTHORITY/DBUS) available to user services,
# so the daemon can lock the session even if the baked DISPLAY ever changes.
systemctl --user import-environment DISPLAY XAUTHORITY DBUS_SESSION_BUS_ADDRESS XDG_RUNTIME_DIR 2>/dev/null || true

# Reload systemd user units and enable service
systemctl --user daemon-reload
systemctl --user enable pomodoro-timer.service
systemctl --user restart pomodoro-timer.service

echo "Pomodoro timer service installed and started for user $USER."
echo "Config copied from: ./config/settings.cfg"
echo "Config location: $CONFIGDIR/settings.cfg"
echo "Logs: $LOGDIR/pomodoro-timer.log"
echo "Sound files: $SOUNDDIR/"
echo
echo "Tip: calibrate the screen lock once with:  $BINDIR/pomodoro --test-lock"
echo "     (locks for 5s, then auto-unlocks, and records what this desktop supports)."
