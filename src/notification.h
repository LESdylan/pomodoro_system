#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include <string>
#include <iostream>
#include <cstdlib>

/**
 * @brief Notification class for handling system notifications and sound alerts
 */
class Notification
{   
private:
	std::string message;
	std::string soundFile;

public:
	/**
	 * @brief Display a desktop notification
	 * @param message The message to display in the notification
	 */
	void	showNotification(const std::string& message);

	/**
	 * @brief Play a sound file
	 * @param soundFile Path to the sound file to play
	 */
	void	playSound(const std::string& soundFile);
};

#endif