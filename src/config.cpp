#include "config.hpp"
#include <cstdlib>
#include <sstream>

/**
 * @brief Convert string to integer (C++98 compatible)
 * @param str String to convert
 * @return Integer value
 */
int stringToInt(const std::string& str)
{
	std::istringstream	iss(str);
	int					value;
	
	iss >> value;
	return (value);
}

Config::Config(const std::string& filename) 
	: filename(filename), workDuration(25), breakDuration(5), longBreakDuration(15), 
	  repeatCycle(4), notificationEnabled(true), notificationSound("bell")
{
}

void Config::loadSettings()
{
	std::ifstream	file(filename.c_str());
	std::string		line;

	if (!file.is_open())
	{
		std::cerr << "Could not open config file: " << filename << std::endl;
		return;
	}
	while (std::getline(file, line))
	{
		if (line.find("work_duration=") == 0)
			workDuration = stringToInt(line.substr(14));
		else if (line.find("break_duration=") == 0)
			breakDuration = stringToInt(line.substr(15));
		else if (line.find("long_break_duration=") == 0)
			longBreakDuration = stringToInt(line.substr(20));
		else if (line.find("sessions_before_long_break=") == 0)
			repeatCycle = stringToInt(line.substr(27));
		else if (line.find("notification_enabled=") == 0)
			notificationEnabled = (line.substr(21) == "true");
		else if (line.find("notification_sound=") == 0)
			notificationSound = line.substr(19);
	}
	file.close();
}

void Config::saveSettings() const
{
	std::ofstream file(filename.c_str());
	
	if (!file.is_open())
	{
		std::cerr << "Could not open config file for writing: " << filename << std::endl;
		return;
	}
	file << "work_duration=" << workDuration << std::endl;
	file << "break_duration=" << breakDuration << std::endl;
	file << "long_break_duration=" << longBreakDuration << std::endl;
	file << "sessions_before_long_break=" << repeatCycle << std::endl;
	file << "notification_enabled=" << (notificationEnabled ? "true" : "false") << std::endl;
	file << "notification_sound=" << notificationSound << std::endl;
	file.close();
}

int			Config::getWorkDuration() const			{ return (workDuration); }
int			Config::getBreakDuration() const		{ return (breakDuration); }
int			Config::getLongBreakDuration() const	{ return (longBreakDuration); }
int			Config::getCycle() const				{ return (repeatCycle); }
bool		Config::isNotificationEnabled() const	{ return (notificationEnabled); }
std::string Config::getNotificationSound() const	{ return ("sounds/" + notificationSound + ".wav"); }

void		Config::setWorkDuration(int duration)						{ workDuration = duration; }
void		Config::setNotificationEnabled(bool enabled)				{ notificationEnabled = enabled; }
void		Config::setBreakDuration(int duration)						{ breakDuration = duration; }
void		Config::setNotificationSound(const std::string& soundName)	{ notificationSound = soundName; }
void		Config::setRepeatCycle(int newCycle)						{ repeatCycle = newCycle; }
void		Config::setLongBreakDuration(int duration)					{ longBreakDuration = duration; }