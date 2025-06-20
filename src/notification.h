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
public:
	Notification();
	~Notification();

	void showNotification(const std::string& message);
	void playSound(const std::string& soundFile);

private:
	Notification(const Notification& other);			// Non-copyable
	Notification& operator=(const Notification& other);	// Non-assignable
};

#endif // NOTIFICATION_H