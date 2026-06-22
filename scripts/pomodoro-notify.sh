#!/bin/sh
# pomodoro-notify.sh <event> [detail]
#
# Fire the Dockerized notifier (HTML email + ntfy phone push) in the BACKGROUND.
# It is a safe no-op when docker, the image, or the .env file is missing, so it can
# never delay or break the Pomodoro daemon.
#
#   event : break-start | back-to-work
#   detail: optional one-line subtitle

set -u

EVENT="${1:-back-to-work}"
DETAIL="${2:-}"
IMAGE="${POMODORO_NOTIFIER_IMAGE:-pomodoro-notifier:latest}"

# No docker -> nothing to do.
command -v docker >/dev/null 2>&1 || exit 0

SELF_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd 2>/dev/null)" || SELF_DIR="."

# Locate the .env holding the credentials (first match wins).
ENV_FILE=""
for c in "${POMODORO_ENV:-}" \
         "$SELF_DIR/.env" \
         "$SELF_DIR/../.env" \
         "$HOME/.config/pomodoro-timer/.env" \
         "./.env"; do
	[ -n "$c" ] && [ -f "$c" ] && { ENV_FILE="$c"; break; }
done
[ -n "$ENV_FILE" ] || exit 0   # no credentials -> nothing to send

# Image must already be built (done at install time / `make notifier-image`).
docker image inspect "$IMAGE" >/dev/null 2>&1 || exit 0

# Run the notifier in the FOREGROUND. The caller (the daemon) backgrounds this whole
# script, so it stays non-blocking -- but keeping docker in the foreground here means a
# single level of backgrounding, so the send always completes even if the caller exits
# immediately (e.g. `make test`). Output goes to the caller, which decides redirection.
docker run --rm --env-file "$ENV_FILE" "$IMAGE" "$EVENT" "$DETAIL"

exit 0
