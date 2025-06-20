#!/bin/bash
set -e

CONFIGDIR="$HOME/.config/pomodoro-timer"

echo "Updating daemon configuration..."

# Copy the updated config file
cp ./config/settings.cfg "$CONFIGDIR/"

# Restart the daemon to pick up changes
systemctl --user restart pomodoro-timer.service

echo "Configuration updated and daemon restarted."
echo "Current config values:"
cat "$CONFIGDIR/settings.cfg" | grep duration
