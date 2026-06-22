# Pomodoro System

A Linux desktop Pomodoro timer that **enforces** your breaks. It runs as a background
daemon, cycles through work/break sessions, and at break time it **locks the screen**
(black) for the break duration — instead of firing a notification you can ignore.

Crucially, it **never suspends the machine**: a `systemd-inhibit` lock is held for the
whole break, so background jobs (AI tasks, builds, downloads) keep running while you step
away. Optionally it can send a **decorated HTML email** and an **instant phone push (ntfy)**
at the start and end of every break, from a small self-built Docker image.

---

## How a break works

```
work session ──► (notify + sound + optional email/ntfy)
              ──► SCREEN LOCKS (black), machine stays awake, jobs keep running
              ──► break ends ──► auto-unlock (or type your password to return early)
              ──► back to work (notify + sound + optional email/ntfy)
```

Enforcement is **hybrid and self-correcting** (`break_mode=hybrid`):

1. If your desktop can auto-unlock through logind (KDE, GNOME, Cinnamon, …), it uses the
   **real OS lock** (`loginctl lock-session`) and auto-unlocks at the end.
2. If a desktop turns out not to support auto-unlock, it records that and uses a
   **fullscreen Qt overlay** (which auto-closes at the end) on subsequent breaks.
3. If no locker is available at all, it falls back to the overlay so a break is *always*
   enforced.

You can force `break_mode=lock` (always OS lock) or `break_mode=overlay` (always overlay).

---

## Requirements

| Component | Needed for | Notes |
|-----------|-----------|-------|
| `g++`, `make` | the daemon | builds with `-std=c++98` |
| Qt5 **or** Qt6 (`qtbase` + Widgets) | the overlay fallback | auto-detected; build daemon-only with `make pomodoro` if absent |
| `libpam` dev headers (optional) | overlay "type password to skip" | without it the overlay just auto-ends the break |
| `systemd` (`loginctl`, `systemd-inhibit`) | real lock + no-suspend | without it, breaks still work via the overlay |
| `notify-send` (libnotify) | desktop notifications | optional; degrades silently |
| `paplay` / `aplay` | sounds | falls back to system sounds, then the terminal bell |
| `docker` (optional) | email + phone notifications | a custom Alpine image; nothing else is installed on the host |

After cloning, fetch the sound pack submodule:

```bash
git submodule update --init
```

---

## Build

```bash
make                 # daemon (pomodoro) + overlay (overlay_timer_qt)
make pomodoro        # daemon only — no Qt needed
make notifier-image  # build the Docker notifier image (needs docker)
make re              # clean rebuild
make clean / fclean  # remove objects / objects + binaries
```

---

## Run

```bash
# Dev (run from the repo root so ./config and ./scripts resolve):
./pomodoro --config config/settings.cfg

# One-time lock calibration (locks ~5s, auto-unlocks, records what your desktop supports):
./pomodoro --test-lock

# Run the overlay standalone (testing): 15s countdown
./overlay_timer_qt 15 "Enter your password to end the break early"
```

### As a systemd user service (recommended)

```bash
make install-service     # deploys binaries + helpers + sounds, builds the notifier image, starts the service
systemctl --user status  pomodoro-timer.service
journalctl   --user -u   pomodoro-timer.service -f
make uninstall-service
```

The installer deploys to `~/.local/share/pomodoro-timer/{bin,logs,sounds}` and config to
`~/.config/pomodoro-timer/settings.cfg`, and writes a unit with the `DISPLAY` / `DBUS` /
`XDG_RUNTIME_DIR` environment the daemon needs to lock the session from a background service.

> The service runs `docker` to send notifications, so your user must be able to use Docker
> (be in the `docker` group, or use rootless Docker).

---

## Configuration — `config/settings.cfg`

Flat `key=value`. Durations are in minutes.

| Key | Default | Meaning |
|-----|---------|---------|
| `work_duration` | 25 | work session length |
| `break_duration` | 5 | short break length |
| `long_break_duration` | 25 | long break length |
| `sessions_before_long_break` | 4 | work sessions per long break |
| `notification_enabled` | true | desktop notifications on/off |
| `work_message` / `break_message` / `long_break_message` | … | notification text |
| `work_sound` / `break_sound` / `long_break_sound` | sci-fi / happy_bells / soft_start | file in `collection_sounds/`, without `.wav` |
| `overlay_prompt` | "Enter your password…" | label on the overlay |
| `break_mode` | **hybrid** | `hybrid` \| `lock` \| `overlay` |
| `inhibit_sleep` | **true** | hold a sleep inhibitor so the machine never suspends during a break |
| `suspend_after_minutes` | **0** | suspend after N minutes still locked. **0 = never** (suspend would pause your jobs) |
| `enforce_break` | false | re-lock if you unlock early (off so the password can bring you back) |
| `lock_command` | *(empty)* | advanced override for the lock step; empty = use the built-in helper |

> Adding/renaming a key requires editing four places in `src/config.cpp` consistently
> (member+default, `loadSettings` with the right `substr` offset, `saveSettings`, getter).

---

## Notifications (email + phone) — optional

A small **custom Alpine image** (`notifier/Dockerfile`, just `python3` + standard library)
sends an HTML email and/or an ntfy phone push at break start and end. Nothing is installed
on the host. The daemon calls `scripts/pomodoro-notify.sh` in the background — a no-op if
docker, the image, or `.env` is missing, so it never delays a break.

### Setup

```bash
cp .env.example .env       # then edit it (it is gitignored)
make notifier-image        # or: make install-service
```

`.env` keys (do **not** wrap values in quotes — `docker --env-file` keeps quotes literally):

```ini
# HTML email (all three required to enable). SMTP_HOST must match the EMAIL domain.
EMAIL=you@example.com
EMAIL_PASSWORD=app_password_no_spaces
EMAIL_RECEIVER=you@example.com
SMTP_HOST=smtp.gmail.com
SMTP_PORT=587               # 587 = STARTTLS, 465 = SSL

# Phone push via ntfy (delivery is by TOPIC, not a phone number).
NTFY_TOPIC=pick-something-unguessable
# NTFY_SERVER=https://ntfy.sh
# NTFY_TOKEN=
```

- **Email (Gmail):** use an **App Password** (Google Account → Security → 2-Step
  Verification → App passwords), not your normal password. The sender `EMAIL` and
  `SMTP_HOST` must belong to the same provider — e.g. a `@gmail.com` address with
  `smtp.gmail.com`, or a custom domain with *its* SMTP server.
- **Phone (ntfy):** install the **ntfy** app, tap **+ → Subscribe to topic**, and enter
  your exact `NTFY_TOPIC`. No account, no phone number.

Test it end-to-end:

```bash
docker run --rm --env-file .env pomodoro-notifier:latest back-to-work "Test"
# expect: "email sent to ... via smtp.gmail.com:587" and "ntfy sent to https://ntfy.sh/<topic>"
```

---

## Sounds

The `.wav` files live in the `collection_sounds/` git submodule; a `sounds` symlink points
at it for dev mode, and `install-service.sh` copies them into the installed sounds dir.
Change the set via the `*_sound` keys in `settings.cfg` (value = file name without `.wav`).

---

## Troubleshooting

- **Email `535 BadCredentials`** — wrong password type or SMTP/host mismatch. Use a Gmail
  **App Password** (16 chars, no spaces) and make sure `SMTP_HOST` matches the `EMAIL` domain.
- **ntfy says `sent` but nothing on the phone** — the ntfy app isn't subscribed to that
  exact topic string.
- **Screen doesn't lock from the service** — run `./pomodoro --test-lock`; if it can't
  confirm a lock, breaks fall back to the overlay. Check that the unit has `DISPLAY` /
  `DBUS_SESSION_BUS_ADDRESS` (re-run `make install-service`).
- **No notifications from the service** — confirm `docker` works for your user and the
  image exists (`docker image inspect pomodoro-notifier:latest`), and that `.env` is at
  `~/.config/pomodoro-timer/.env`.

---

## Architecture (brief)

- `src/main.cpp` — daemon + session state machine; break enforcement (`runEnforcedBreak`,
  native lock + overlay fallback, sleep inhibitor) and the notifier hook.
- `src/config.{hpp,cpp}` — flat key=value config.
- `src/timer.{h,cpp}` — polling countdown (work sessions).
- `src/notification.{h,cpp}` — `notify-send` + sound fallback chain.
- `src/overlay_timer_qt.cpp` — fullscreen overlay (Qt5/Qt6), grabs input, optional PAM skip.
- `scripts/pomodoro-session.sh` — all lock/unlock/status/auto-unlock logic per desktop.
- `scripts/pomodoro-notify.sh` + `notifier/` — the Dockerized email/ntfy sender.
