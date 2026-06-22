#!/bin/sh
# pomodoro-session.sh -- portable session lock/unlock helper for the Pomodoro daemon.
#
# All distro/desktop-specific logic lives here (not in the C++ daemon) so it can be
# tweaked per machine without rebuilding. Designed to be called from a `systemd --user`
# service, so it self-heals the session/display environment.
#
# Subcommands:
#   lock            Lock the session (non-blocking). Exit 0 only if a lock is CONFIRMED.
#   unlock          Ask the session to unlock. Exit 0 only if unlock is CONFIRMED.
#   status          Exit 0 if the session is currently locked, 1 otherwise.
#   can-autounlock  Exit 0 if native auto-unlock is expected to work on this desktop.
#   mark-ok         Record that native auto-unlock works (state cache).
#   mark-failed     Record that native auto-unlock does NOT work -> daemon uses overlay.

set -u

# --- environment self-heal (so this works from a bare systemd --user service) ---------
UID_NUM="$(id -u)"
: "${XDG_RUNTIME_DIR:=/run/user/$UID_NUM}"
export XDG_RUNTIME_DIR
: "${DBUS_SESSION_BUS_ADDRESS:=unix:path=$XDG_RUNTIME_DIR/bus}"
export DBUS_SESSION_BUS_ADDRESS
[ -n "${DISPLAY:-}" ] || export DISPLAY=:0

STATE_DIR="${XDG_DATA_HOME:-$HOME/.local/share}/pomodoro-timer"
STATE_FILE="$STATE_DIR/state"

# --- helpers --------------------------------------------------------------------------

# Resolve the GRAPHICAL session id (not the caller's service session, which may be empty).
session_id() {
	sid="$(loginctl show-user "$UID_NUM" -p Display --value 2>/dev/null)"
	if [ -z "$sid" ]; then
		# Fallback: first session belonging to this user.
		sid="$(loginctl list-sessions --no-legend 2>/dev/null \
			| awk -v u="$UID_NUM" '$2==u{print $1; exit}')"
	fi
	printf '%s' "$sid"
}

# Locate a usable qdbus binary (Qt5 'qdbus', Qt6 'qdbus6', or 'qdbus-qt5').
qdbus_bin() {
	for q in qdbus qdbus6 qdbus-qt5; do
		if command -v "$q" >/dev/null 2>&1; then printf '%s' "$q"; return 0; fi
	done
	return 1
}

# Is the session locked right now?  (logind LockedHint first, then freedesktop ScreenSaver.)
is_locked() {
	sid="$(session_id)"
	if [ -n "$sid" ]; then
		[ "$(loginctl show-session "$sid" -p LockedHint --value 2>/dev/null)" = "yes" ] && return 0
	fi
	q="$(qdbus_bin)" || return 1
	[ "$("$q" org.freedesktop.ScreenSaver /ScreenSaver GetActive 2>/dev/null)" = "true" ] && return 0
	return 1
}

# Poll for a desired lock state for ~3s (lockers can lag by a second or so).
#   _confirm locked   -> succeed once locked
#   _confirm unlocked -> succeed once unlocked
_confirm() {
	want="$1"; i=0
	while [ "$i" -lt 6 ]; do
		if [ "$want" = "locked" ]; then
			is_locked && return 0
		else
			is_locked || return 0
		fi
		sleep 0.5
		i=$((i + 1))
	done
	return 1
}

# Force the monitors black to save power / reinforce the break (X11; harmless elsewhere).
black_screen() {
	command -v xset >/dev/null 2>&1 && xset dpms force off 2>/dev/null || true
}

read_state() {
	key="$1"
	[ -f "$STATE_FILE" ] || return 1
	grep "^$key=" "$STATE_FILE" 2>/dev/null | tail -n1 | cut -d= -f2-
}

write_state() {
	key="$1"; val="$2"
	mkdir -p "$STATE_DIR" 2>/dev/null || true
	tmp="$STATE_FILE.tmp.$$"
	if [ -f "$STATE_FILE" ]; then
		grep -v "^$key=" "$STATE_FILE" 2>/dev/null > "$tmp" || true
	fi
	printf '%s=%s\n' "$key" "$val" >> "$tmp"
	mv "$tmp" "$STATE_FILE" 2>/dev/null || true
}

# --- actions --------------------------------------------------------------------------

do_lock() {
	sid="$(session_id)"
	q="$(qdbus_bin)"

	# Try mechanisms most-portable first; gate success on an actual confirmed lock,
	# because every one of these returns 0 even when it locks nothing.
	if [ -n "$sid" ] && loginctl lock-session "$sid" 2>/dev/null && _confirm locked; then
		black_screen; return 0
	fi
	if [ -n "$q" ] && "$q" org.freedesktop.ScreenSaver /ScreenSaver Lock 2>/dev/null && _confirm locked; then
		black_screen; return 0
	fi
	if dbus-send --session --type=method_call \
		--dest=org.freedesktop.ScreenSaver /ScreenSaver \
		org.freedesktop.ScreenSaver.Lock 2>/dev/null && _confirm locked; then
		black_screen; return 0
	fi
	if command -v xdg-screensaver >/dev/null 2>&1 \
		&& xdg-screensaver lock 2>/dev/null && _confirm locked; then
		black_screen; return 0
	fi

	# Final chance: a slow locker may have engaged after our per-method window.
	if is_locked; then black_screen; return 0; fi
	return 1
}

do_unlock() {
	sid="$(session_id)"
	# Wake the monitors back up first.
	command -v xset >/dev/null 2>&1 && xset dpms force on 2>/dev/null || true
	[ -n "$sid" ] && loginctl unlock-session "$sid" 2>/dev/null
	_confirm unlocked
}

do_can_autounlock() {
	case "$(read_state native_auto_unlock 2>/dev/null)" in
		ok)     return 0 ;;
		failed) return 1 ;;
	esac
	# Unknown: best-effort guess from the desktop. These honor logind unlock-session.
	de="$(printf '%s' "${XDG_CURRENT_DESKTOP:-}|${DESKTOP_SESSION:-}" | tr '[:upper:]' '[:lower:]')"
	case "$de" in
		*kde*|*plasma*|*gnome*|*cinnamon*|*budgie*|*unity*|*deepin*) return 0 ;;
		*) return 1 ;;
	esac
}

# --- dispatch -------------------------------------------------------------------------

case "${1:-}" in
	lock)           do_lock ;;
	unlock)         do_unlock ;;
	status)         is_locked ;;
	can-autounlock) do_can_autounlock ;;
	mark-ok)        write_state native_auto_unlock ok ;;
	mark-failed)    write_state native_auto_unlock failed ;;
	*)
		echo "usage: $0 {lock|unlock|status|can-autounlock|mark-ok|mark-failed}" >&2
		exit 2
		;;
esac
exit $?
