#include "notification.h"

void Notification::showNotification(const std::string& message)
{
	std::string command;

	command = "notify-send 'Pomodoro Timer' '" + message + "'";
	system(command.c_str());
	std::cout << "NOTIFICATION: " << message << std::endl;
}

void Notification::playSound(const std::string& soundFile)
{
	std::string	command;

	command = "aplay " + soundFile + " 2>/dev/null || paplay " + soundFile + " 2>/dev/null";
	system(command.c_str());
}