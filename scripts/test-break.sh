#!/bin/sh
# make test -- trigger the REAL break logic right now (no 25-minute wait), so you can
# verify two things at once:
#   1. the screen actually locks (black) for the break, and
#   2. background processes keep running the whole time (no suspend / no interruption).
#
# A 1-tick/second heartbeat runs in the background across the lock; if it keeps ticking
# while the screen is locked, nothing was paused.
#
# Usage: scripts/test-break.sh [seconds]   (default 20)

set -u

SECS="${1:-20}"
HB="$(mktemp 2>/dev/null || echo /tmp/pomodoro_heartbeat)"

BIN=./pomodoro
[ -x "$BIN" ] || BIN="$HOME/.local/share/pomodoro-timer/bin/pomodoro"
CFG=config/settings.cfg
[ -f "$CFG" ] || CFG="$HOME/.config/pomodoro-timer/settings.cfg"

echo "Heartbeat: a background process will tick once/second to /tmp during the lock."
: > "$HB"
( while : ; do date +%H:%M:%S >> "$HB"; sleep 1; done ) &
HBPID=$!

echo
echo ">>> Triggering a ${SECS}s enforced break -- the SCREEN WILL LOCK now."
echo ">>> (If it doesn't auto-unlock at the end, just type your password.)"
echo
"$BIN" --test-break "$SECS" --config "$CFG"

kill "$HBPID" 2>/dev/null
ticks=$(wc -l < "$HB" 2>/dev/null | tr -d ' ')
rm -f "$HB"

echo
echo "=================================================================="
echo "Heartbeat ticks recorded WHILE the screen was locked: ${ticks:-0}"
echo "Expected ~${SECS}. If it matches, your background processes ran the"
echo "entire time the screen was locked -- the lock did not suspend or"
echo "interrupt anything."
echo "=================================================================="
