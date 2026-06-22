QT += widgets
SOURCES += src/overlay_timer_qt.cpp

# PAM powers the overlay's "type your password to end the break early" prompt.
# It is OPTIONAL: without libpam-dev the overlay still builds and works, and the
# break simply auto-ends at the timer (no early skip). Install libpam-dev to enable it.
exists(/usr/include/security/pam_appl.h) {
    DEFINES += HAVE_PAM
    LIBS += -lpam
}
