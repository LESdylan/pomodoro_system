#!/bin/bash
set -e

SERVICE_FILE="$HOME/.config/systemd/user/pomodoro-timer.service"
BIN_PATH="$HOME/.local/share/pomodoro-timer/bin/pomodoro"
CONFIG_PATH="$HOME/.config/pomodoro-timer/settings.cfg"

# Ensure service file exists and has correct audio environment
mkdir -p "$(dirname "$SERVICE_FILE")"

cat > "$SERVICE_FILE" <<EOF
[Unit]
Description=Pomodoro Timer Daemon

[Service]
ExecStart=$BIN_PATH --daemon --config $CONFIG_PATH
Restart=on-failure
WorkingDirectory=/home/dlesieur/Documents/pomodoro_system
Environment="PULSE_SERVER=unix:/run/user/%U/pulse/native"
Environment="XDG_RUNTIME_DIR=/run/user/%U"

[Install]
WantedBy=default.target
EOF

# Reload and restart the service
systemctl --user daemon-reload
systemctl --user enable pomodoro-timer.service
systemctl --user restart pomodoro-timer.service

echo "Pomodoro timer daemon service fixed and restarted."
echo "If sound still does not work, check your PulseAudio/pipewire setup and journalctl logs."
