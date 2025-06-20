#!/bin/bash
set -e

SYSTEMD_USER_DIR="$HOME/.config/systemd/user"
APPDIR="$HOME/.local/share/pomodoro-timer"
CONFIGDIR="$HOME/.config/pomodoro-timer"

systemctl --user stop pomodoro-timer.service || true
systemctl --user disable pomodoro-timer.service || true
rm -f "$SYSTEMD_USER_DIR/pomodoro-timer.service"
systemctl --user daemon-reload

rm -rf "$APPDIR"
rm -rf "$CONFIGDIR"

echo "Pomodoro timer daemon uninstalled for user $USER."
