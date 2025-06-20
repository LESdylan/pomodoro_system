#include "config.hpp"
#include <cstdlib>
#include <sstream>

namespace {
	/**
	 * @brief Convert string to integer (C++98 compatible)
	 * @param str String to convert
	 * @return Integer value
	 */
	int stringToInt(const std::string& str)
	{
		std::istringstream iss(str);
		int value;
		
		iss >> value;
		return value;
	}
}

Config::Config(const std::string& filename) 
	: m_filename(filename)
	, m_workDuration(25)
	, m_breakDuration(5)
	, m_longBreakDuration(15)
	, m_repeatCycle(4)
	, m_notificationEnabled(true)
	, m_notificationSound("bell")
	, m_workMessage("Time to focus!")
	, m_breakMessage("Time to take a break!")
	, m_longBreakMessage("Take a long break!")
	, m_workSound("work_bell")
	, m_breakSound("break_bell")
	, m_longBreakSound("long_break_bell")
	, m_overlayPrompt("Really urgent to skip? (Y/N)")
{
}

Config::~Config()
{
}

void Config::loadSettings()
{
	std::ifstream file(m_filename.c_str());
	std::string line;

	if (!file.is_open()) {
		std::cerr << "Could not open config file: " << m_filename << std::endl;
		return;
	}
	
	while (std::getline(file, line)) {
		if (line.find("work_duration=") == 0)
			m_workDuration = stringToInt(line.substr(14));
		else if (line.find("break_duration=") == 0)
			m_breakDuration = stringToInt(line.substr(15));
		else if (line.find("long_break_duration=") == 0)
			m_longBreakDuration = stringToInt(line.substr(20));
		else if (line.find("sessions_before_long_break=") == 0)
			m_repeatCycle = stringToInt(line.substr(27));
		else if (line.find("notification_enabled=") == 0)
			m_notificationEnabled = (line.substr(21) == "true");
		else if (line.find("notification_sound=") == 0)
			m_notificationSound = line.substr(19);
		else if (line.find("work_message=") == 0)
			m_workMessage = line.substr(13);
		else if (line.find("break_message=") == 0)
			m_breakMessage = line.substr(14);
		else if (line.find("long_break_message=") == 0)
			m_longBreakMessage = line.substr(19);
		else if (line.find("work_sound=") == 0)
			m_workSound = line.substr(11);
		else if (line.find("break_sound=") == 0)
			m_breakSound = line.substr(12);
		else if (line.find("long_break_sound=") == 0)
			m_longBreakSound = line.substr(17);
		else if (line.find("overlay_prompt=") == 0)
			m_overlayPrompt = line.substr(15);
	}
	file.close();
}

void Config::saveSettings() const
{
	std::ofstream file(m_filename.c_str());
	
	if (!file.is_open()) {
		std::cerr << "Could not open config file for writing: " << m_filename << std::endl;
		return;
	}
	
	file << "work_duration=" << m_workDuration << std::endl;
	file << "break_duration=" << m_breakDuration << std::endl;
	file << "long_break_duration=" << m_longBreakDuration << std::endl;
	file << "sessions_before_long_break=" << m_repeatCycle << std::endl;
	file << "notification_enabled=" << (m_notificationEnabled ? "true" : "false") << std::endl;
	file << "notification_sound=" << m_notificationSound << std::endl;
	file << "work_message=" << m_workMessage << std::endl;
	file << "break_message=" << m_breakMessage << std::endl;
	file << "long_break_message=" << m_longBreakMessage << std::endl;
	file << "work_sound=" << m_workSound << std::endl;
	file << "break_sound=" << m_breakSound << std::endl;
	file << "long_break_sound=" << m_longBreakSound << std::endl;
	file << "overlay_prompt=" << m_overlayPrompt << std::endl;
	file.close();
}

std::string Config::constructSoundPath(const std::string& soundName) const
{
	const char* home = getenv("HOME");
	if (home) {
		// Try installed sound directory first (for daemon mode)
		std::string installedPath = std::string(home) + "/.local/share/pomodoro-timer/sounds/" + soundName + ".wav";
		std::ifstream testFile1(installedPath.c_str());
		if (testFile1.good()) {
			testFile1.close();
			return installedPath;
		}
		testFile1.close();
		
		// Try project directory (for development mode)
		std::string projectPath = std::string(home) + "/Documents/system/pomodoro_system/sounds/" + soundName + ".wav";
		std::ifstream testFile2(projectPath.c_str());
		if (testFile2.good()) {
			testFile2.close();
			return projectPath;
		}
		testFile2.close();
	}
	
	// Try relative paths from working directory
	std::string relativePath = "sounds/" + soundName + ".wav";
	std::ifstream testFile3(relativePath.c_str());
	if (testFile3.good()) {
		testFile3.close();
		return relativePath;
	}
	testFile3.close();
	
	// Try current directory
	std::string currentDir = soundName + ".wav";
	std::ifstream testFile4(currentDir.c_str());
	if (testFile4.good()) {
		testFile4.close();
		return currentDir;
	}
	testFile4.close();
	
	// Return a non-existent path so notification.cpp can handle fallback
	return soundName + ".wav";
}

// Getters
int Config::getWorkDuration() const { return m_workDuration; }
int Config::getBreakDuration() const { return m_breakDuration; }
int Config::getLongBreakDuration() const { return m_longBreakDuration; }
int Config::getCycle() const { return m_repeatCycle; }
bool Config::isNotificationEnabled() const { return m_notificationEnabled; }
std::string Config::getNotificationSound() const { return constructSoundPath(m_notificationSound); }
std::string Config::getWorkMessage() const { return m_workMessage; }
std::string Config::getBreakMessage() const { return m_breakMessage; }
std::string Config::getLongBreakMessage() const { return m_longBreakMessage; }
std::string Config::getWorkSound() const { return constructSoundPath(m_workSound); }
std::string Config::getBreakSound() const { return constructSoundPath(m_breakSound); }
std::string Config::getLongBreakSound() const { return constructSoundPath(m_longBreakSound); }
std::string Config::getOverlayPrompt() const { return m_overlayPrompt; }

// Setters
void Config::setWorkDuration(int duration) { m_workDuration = duration; }
void Config::setBreakDuration(int duration) { m_breakDuration = duration; }
void Config::setLongBreakDuration(int duration) { m_longBreakDuration = duration; }
void Config::setRepeatCycle(int newCycle) { m_repeatCycle = newCycle; }
void Config::setNotificationEnabled(bool enabled) { m_notificationEnabled = enabled; }
void Config::setNotificationSound(const std::string& soundName) { m_notificationSound = soundName; }