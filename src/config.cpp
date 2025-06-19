#include "config.hpp"


Config::Config(const std::string& filename) : filename(filename), workDuration(25), breakDuration(5), notificationEnabled(true), notificationsound("bell") {}

/**
 * 
 */
void		Config::loadSettings() const
{
	std::ifstream	file(filename);
	std::string		line;

	if (!file.is_open())
	{
		std::cerr << "Could not open config file" << filename << std::endl;
		return ;
	}
	while (std::getline(file, line))
	{
		if (line.find("work_duration=") == 0)
			workDuration = std::stoi(line.substr(14));
		if (line.find("break_duration=" == 0))
			breakDuration = std::stoi(line.substr(15));
		if (line.find("long_break_duration=") == 0)
			longBreakDuration = std::stoi(line.substr(19));
		if (line.find("sessions_before_long_break="))
			repeatCycle = std::stoi(line.substr(27));
		if (line.find("notification_enabled=") == 0)
			notificationEnabled = (line.substr(21) == "true");
		if (line.find("notification_sound=") == 0)
			notificationSound = line.substr(19);
	}
	file.close();
}

/**
 * 
 */
void		Config::saveSettings() const
{
	std::ofstream file(filename);
	if (!file.is_open())
	{
		std::cerr << "Could not open config file for writing: " << filename << std::endl;
		return ;
	}
	file << "work_duration=" << workDuration << std::endl;
	file << "break_duration=" << breakDuration << std::endl;
	file << "long_break_duration=" << longBreakDuration << std::endl;
	file << "sessions_before_long_break=" << repeatCycle << std::endl;
	file << "notification_enabled=" << notificationEnabled << std::endl;
	file << "notification_sound=" << notificationSound << std::endl;
	file.close();
}

/* GETTERS */
int			Config::getWorkDuration()		const	{	return (workDuration);			}
int			Config::getBreakDuration()		const	{	return (breakDuration);			}
int			Config::getLongBreakDuration()	const	{	return (longBreakDuration);		}
int			Config::getCycle()				const	{	return (repeatCycle);			}
bool		Config::isNotificationEnabled() const	{	return (notificationEnabled);	}
std::string Config::getNotificationSound()	const	{	return ("sounds/" + notificationSound) + ".wav";		}

/** SETTERS */
void		Config::setWorkDuration(int duration)						{	workDuration = duration;		}
void		Config::setNotificationEnabled(bool enabled)				{	notificationEnabled = enabled;	}
void		Config::setBreakDuration(int duration)						{	breakDuration = duration;		}
void		Config::setNotificationSound(const std::string& soundName)	{	notificationSound = soundName;	}
void		Config::setRepeatCycle(int newCycle)						{	repeatCycle = newCycle;			}
void		Config::setLongBreakDuration(int duration)					{	longBreakDuration = duration;	}