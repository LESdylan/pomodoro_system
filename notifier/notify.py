#!/usr/bin/env python3
"""Pomodoro notifier -- HTML email (smtplib) + ntfy phone push (urllib).

Standard library only, so the Alpine image just needs `python3` (no pip).

Usage:
    notify.py <event> [detail]
      event : break-start | back-to-work   (default: back-to-work)
      detail: optional one-line subtitle shown in the message

Each channel fires only when its env vars are present:
    Email : EMAIL, EMAIL_PASSWORD, EMAIL_RECEIVER   (+ SMTP_HOST, SMTP_PORT)
    ntfy  : NTFY_TOPIC                              (+ NTFY_SERVER, NTFY_TOKEN)
"""

import os
import ssl
import sys
import smtplib
import urllib.request
from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText


def event_copy(event, detail):
    """Return the per-event presentation data."""
    if event == "break-start":
        return {
            "emoji": "\U0001F345",  # tomato
            "title": "Break time",
            "headline": "Time to step away",
            "sub": detail or "Take a real break -- stretch, breathe, hydrate.",
            "c1": "#0ea5e9",
            "c2": "#22c55e",
            "ntfy_title": "Break time",
            "ntfy_tags": "tomato,coffee",
            "ntfy_priority": "default",
        }
    return {  # back-to-work (default)
        "emoji": "⏰",  # alarm clock
        "title": "Back to work",
        "headline": "Break's over -- back to focus",
        "sub": detail or "Your break is complete. Let's get back to it!",
        "c1": "#f59e0b",
        "c2": "#ef4444",
        "ntfy_title": "Back to work",
        "ntfy_tags": "alarm_clock,rocket",
        "ntfy_priority": "high",
    }


def html_email(c):
    """A self-contained, inline-styled HTML card (email-client friendly)."""
    return f"""\
<!DOCTYPE html>
<html lang="en">
<body style="margin:0;padding:0;background:#0b1020;font-family:'Segoe UI',Roboto,Helvetica,Arial,sans-serif;">
  <table role="presentation" width="100%" cellpadding="0" cellspacing="0" style="background:#0b1020;padding:32px 12px;">
    <tr><td align="center">
      <table role="presentation" width="480" cellpadding="0" cellspacing="0"
             style="max-width:480px;width:100%;background:#11162a;border-radius:20px;overflow:hidden;
                    box-shadow:0 20px 60px rgba(0,0,0,0.45);">
        <tr>
          <td style="background:linear-gradient(135deg,{c['c1']},{c['c2']});padding:40px 24px;text-align:center;">
            <div style="font-size:64px;line-height:1;">{c['emoji']}</div>
            <div style="margin-top:14px;font-size:13px;letter-spacing:3px;text-transform:uppercase;
                        color:rgba(255,255,255,0.85);font-weight:600;">Pomodoro</div>
          </td>
        </tr>
        <tr>
          <td style="padding:32px 32px 12px;text-align:center;">
            <h1 style="margin:0;font-size:26px;color:#f8fafc;font-weight:700;">{c['headline']}</h1>
            <p style="margin:14px 0 0;font-size:16px;line-height:1.5;color:#aab4cf;">{c['sub']}</p>
          </td>
        </tr>
        <tr>
          <td style="padding:8px 32px 36px;text-align:center;">
            <div style="display:inline-block;padding:12px 26px;border-radius:999px;
                        background:linear-gradient(135deg,{c['c1']},{c['c2']});
                        color:#ffffff;font-weight:700;font-size:15px;">{c['title']}</div>
          </td>
        </tr>
        <tr>
          <td style="padding:18px 24px;background:#0d1222;text-align:center;
                     font-size:12px;color:#64748b;">
            Sent by your Pomodoro daemon
          </td>
        </tr>
      </table>
    </td></tr>
  </table>
</body>
</html>"""


def send_email(c):
    sender = os.environ.get("EMAIL")
    password = os.environ.get("EMAIL_PASSWORD")
    receiver = os.environ.get("EMAIL_RECEIVER")
    if not (sender and password and receiver):
        return False, "email skipped (EMAIL / EMAIL_PASSWORD / EMAIL_RECEIVER not all set)"

    host = os.environ.get("SMTP_HOST", "smtp.gmail.com")
    port = int(os.environ.get("SMTP_PORT", "587"))

    msg = MIMEMultipart("alternative")
    msg["Subject"] = f"{c['emoji']} {c['title']} -- Pomodoro"
    msg["From"] = sender
    msg["To"] = receiver
    msg.attach(MIMEText(c["sub"], "plain", "utf-8"))
    msg.attach(MIMEText(html_email(c), "html", "utf-8"))

    ctx = ssl.create_default_context()
    try:
        if port == 465:
            with smtplib.SMTP_SSL(host, port, context=ctx, timeout=20) as s:
                s.login(sender, password)
                s.sendmail(sender, [receiver], msg.as_string())
        else:
            with smtplib.SMTP(host, port, timeout=20) as s:
                s.ehlo()
                s.starttls(context=ctx)
                s.login(sender, password)
                s.sendmail(sender, [receiver], msg.as_string())
        return True, f"email sent to {receiver} via {host}:{port}"
    except Exception as e:  # noqa: BLE001 - report any failure, never crash the daemon
        return False, f"email FAILED: {e}"


def send_ntfy(c):
    topic = os.environ.get("NTFY_TOPIC")
    if not topic:
        return False, "ntfy skipped (NTFY_TOPIC not set)"

    server = os.environ.get("NTFY_SERVER", "https://ntfy.sh").rstrip("/")
    url = f"{server}/{topic}"
    req = urllib.request.Request(url, data=c["sub"].encode("utf-8"), method="POST")
    # NOTE: HTTP headers must be latin-1, so keep Title ASCII and let Tags carry emoji.
    req.add_header("Title", c["ntfy_title"])
    req.add_header("Tags", c["ntfy_tags"])
    req.add_header("Priority", c["ntfy_priority"])
    token = os.environ.get("NTFY_TOKEN")
    if token:
        req.add_header("Authorization", f"Bearer {token}")
    try:
        urllib.request.urlopen(req, timeout=15)
        return True, f"ntfy sent to {url}"
    except Exception as e:  # noqa: BLE001
        return False, f"ntfy FAILED: {e}"


def main():
    event = sys.argv[1] if len(sys.argv) > 1 else "back-to-work"
    detail = sys.argv[2] if len(sys.argv) > 2 else ""
    c = event_copy(event, detail)

    sent_any = False
    for fn in (send_email, send_ntfy):
        ok, info = fn(c)
        print(info, flush=True)
        sent_any = sent_any or ok
    return 0 if sent_any else 1


if __name__ == "__main__":
    sys.exit(main())
