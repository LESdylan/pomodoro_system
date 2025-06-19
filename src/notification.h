#ifndef NOTIFICATION_H
#define NOTIFICATION_H

# include <string>
#include <iostream>
#include <cstdlib>

class Notification
{   
	private:
		std::string message;
		std::string soundFile;

	public:
		void	showNotification(const std::string& message);
		void	playSound(const std::string& soundFile);
};
#endif