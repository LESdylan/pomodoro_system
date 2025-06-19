CC = c++
CFLAGS = -Wall -Wextra -std=c++98
SRCDIR = src
SOURCES = $(SRCDIR)/main.cpp $(SRCDIR)/timer.cpp $(SRCDIR)/notification.cpp $(SRCDIR)/config.cpp
OBJECTS = $(SOURCES:.cpp=.o)
TARGET = pomodoro

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(TARGET)

$(SRCDIR)/%.o: $(SRCDIR)/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

install-service: $(TARGET)
	chmod +x install-service.sh
	./install-service.sh

uninstall-service:
	systemctl --user stop pomodoro 2>/dev/null || true
	systemctl --user disable pomodoro-timer 2>/dev/null || true
	rm -f ~/.config/systemd/user/pomodoro-timer.service
	systemctl --user daemon-reload

clean:
	rm -f $(OBJ) $(TARGET)

fclean: clean
	rm -f $(TARGET)

re: fclean all

.PHONY: all clean re install-service uninstall-service