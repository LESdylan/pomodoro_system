#include "notification.h"

Notification::Notification(std::string messsage)::message("time to work")

/**
 * send the notification for linux desktop notifications
 * then we have made a fallback to console output
 */
void Notification::ShowNotification(const std::string& message)
{
	std::string command;

	command = "notify-send 'pomodoro TIMER' '" + message + "'";
	system(command.c_str());
	std::cout << "NOTIFICATION: " << message << std::endl;
}

/**
 * trying to play the sound using (ALSA or paplay(Pulse audio))
 */
void Notification::playSound(const std::string& soundFile)
{
	std::string	command;

	command = "aplay " + soundFile + "2>/dev/null || paplay " + soundFile + "2>/dev/null";
	system(command.c_str());
}
