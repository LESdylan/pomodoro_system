#!/bin/bash
set -e

SERVICE_FILE="$HOME/.config/systemd/user/pomodoro-timer.service"
BIN_PATH="$HOME/.local/share/pomodoro-timer/bin/pomodoro"
CONFIG_PATH="$HOME/.config/pomodoro-timer/settings.cfg"

# Ensure service file exists and has correct audio environment
mkdir -p "$(dirname "$SERVICE_FILE")"

# Keep this unit identical to the one install-service.sh generates so re-running one
# script never undoes the other's environment fixes.
cat > "$SERVICE_FILE" <<EOF
[Unit]
Description=Pomodoro Timer Daemon

[Service]
ExecStart=$BIN_PATH --daemon --config $CONFIG_PATH
Restart=on-failure
WorkingDirectory=$(dirname "$BIN_PATH")
Environment="PULSE_SERVER=unix:/run/user/%U/pulse/native"
Environment="XDG_RUNTIME_DIR=/run/user/%U"
Environment="DBUS_SESSION_BUS_ADDRESS=unix:path=/run/user/%U/bus"
Environment="DISPLAY=${DISPLAY:-:0}"

[Install]
WantedBy=default.target
EOF

# Make the live graphical-session env available to user services (needed for locking).
systemctl --user import-environment DISPLAY XAUTHORITY DBUS_SESSION_BUS_ADDRESS XDG_RUNTIME_DIR 2>/dev/null || true

# Reload and restart the service
systemctl --user daemon-reload
systemctl --user enable pomodoro-timer.service
systemctl --user restart pomodoro-timer.service

echo "Pomodoro timer daemon service fixed and restarted."
echo "If sound still does not work, check your PulseAudio/pipewire setup and journalctl logs."
