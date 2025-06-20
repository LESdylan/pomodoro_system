CC = c++
CFLAGS = -Wall -Wextra -std=c++98
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
	qmake -o Makefile.qt overlay_timer_qt.pro
	make -f Makefile.qt
	test -f overlay_timer_qt && mv overlay_timer_qt $(OVERLAY_TARGET) || echo "overlay_timer_qt already named correctly"

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

.PHONY: all clean re install-service uninstall-service