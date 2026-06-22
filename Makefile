CC = c++
CFLAGS = -Wall -Wextra -std=c++98
# Auto-detect a qmake (Qt5 or Qt6 -- the overlay builds on both).
QMAKE := $(shell command -v qmake 2>/dev/null || command -v qmake6 2>/dev/null || command -v qmake-qt5 2>/dev/null || command -v qmake-qt6 2>/dev/null)
SRCDIR = src
SOURCES = $(SRCDIR)/main.cpp $(SRCDIR)/timer.cpp $(SRCDIR)/notification.cpp $(SRCDIR)/config.cpp
OBJECTS = $(SOURCES:.cpp=.o)
TARGET = pomodoro
OVERLAY_TARGET = overlay_timer_qt

all: $(TARGET) $(OVERLAY_TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(TARGET)

$(SRCDIR)/%.o: $(SRCDIR)/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

$(OVERLAY_TARGET): $(SRCDIR)/overlay_timer_qt.cpp
	@test -n "$(QMAKE)" || { echo "ERROR: qmake (Qt5 or Qt6) not found. Install qtbase/qt5-qtbase, or build daemon-only with 'make pomodoro'."; exit 1; }
	$(QMAKE) -o Makefile.qt overlay_timer_qt.pro
	$(MAKE) -f Makefile.qt
	test -f overlay_timer_qt && mv -f overlay_timer_qt $(OVERLAY_TARGET) || echo "overlay_timer_qt already named correctly"

notifier-image:
	docker build -t pomodoro-notifier:latest ./notifier

# Trigger the real break logic now (locks the screen ~20s) and prove background
# processes keep running. Override the duration: make test SECS=30
SECS ?= 20
test: $(TARGET)
	@chmod +x scripts/test-break.sh
	@sh scripts/test-break.sh $(SECS)

install-service: $(TARGET) $(OVERLAY_TARGET)
	chmod +x install-service.sh
	./install-service.sh

uninstall-service:
	systemctl --user stop pomodoro 2>/dev/null || true
	systemctl --user disable pomodoro-timer 2>/dev/null || true
	rm -f ~/.config/systemd/user/pomodoro-timer.service
	systemctl --user daemon-reload

clean:
	rm -f $(OBJECTS) $(TARGET) $(OVERLAY_TARGET)
	rm -f Makefile.qt overlay_timer_qt.moc moc_predefs.h .qmake.stash

fclean: clean
	rm -f $(TARGET) $(OVERLAY_TARGET)

re: fclean all

.PHONY: all clean re install-service uninstall-service notifier-image test