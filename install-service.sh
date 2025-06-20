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
cp ./overlay_timer_qt "$BINDIR/"
# Always overwrite the config file with the latest version
cp ./config/settings.cfg "$CONFIGDIR/"

# Copy sound files if they exist
if [ -d "./sounds" ]; then
    cp -r ./sounds/* "$SOUNDDIR/" 2>/dev/null || echo "No sound files found in ./sounds/"
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

[Install]
WantedBy=default.target
EOF

# Reload systemd user units and enable service
systemctl --user daemon-reload
systemctl --user enable pomodoro-timer.service
systemctl --user restart pomodoro-timer.service

echo "Pomodoro timer service installed and started for user $USER."
echo "Config copied from: ./config/settings.cfg"
echo "Config location: $CONFIGDIR/settings.cfg"
echo "Logs: $LOGDIR/pomodoro-timer.log"
echo "Sound files: $SOUNDDIR/"
