# Custom, minimal notifier image -- built from Alpine, no prebuilt app images.
# Only python3 is added; the sender uses the Python standard library (smtplib +
# urllib), so there is no pip / no extra packages and nothing is installed on the host.
FROM alpine:3.20

RUN apk add --no-cache python3 ca-certificates

WORKDIR /app
COPY notify.py /app/notify.py
RUN chmod +x /app/notify.py

# Args after `docker run <image>` are appended here, e.g. ... <image> back-to-work "detail"
ENTRYPOINT ["python3", "/app/notify.py"]
CMD ["back-to-work"]
