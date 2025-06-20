#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <iostream>
#include <fstream>
#include <string>

/**
 * @brief Configuration class for managing Pomodoro timer settings
 */
class Config 
{
private:
	std::string	m_filename; ///< Configuration file path
	int			m_workDuration; ///< Work session duration in minutes
	int			m_breakDuration; ///< Break session duration in minutes
	int			m_longBreakDuration; ///< Long break session duration in minutes
	int			m_repeatCycle; ///< Number of cycles before long break
	bool		m_notificationEnabled; ///< Notification enabled flag
	std::string	m_notificationSound; ///< Notification sound file path
	std::string	m_workMessage;
	std::string	m_breakMessage;
	std::string	m_longBreakMessage;
	std::string	m_workSound;
	std::string	m_breakSound;
	std::string	m_longBreakSound;
	std::string	m_overlayPrompt;

public:
	/**
	 * @brief Constructor for Config class
	 * @param filename Path to configuration file
	 */
	explicit Config(const std::string& filename);
	~Config();

	/**
	 * @brief Load settings from configuration file
	 */
	void		loadSettings();

	/**
	 * @brief Save current settings to configuration file
	 */
	void		saveSettings() const;

	/**
	 * @brief Get work session duration
	 * @return Work duration in minutes
	 */
	int			getWorkDuration() const;

	/**
	 * @brief Get break session duration
	 * @return Break duration in minutes
	 */
	int			getBreakDuration() const;

	/**
	 * @brief Get long break session duration
	 * @return Long break duration in minutes
	 */
	int			getLongBreakDuration() const;

	/**
	 * @brief Get number of cycles before long break
	 * @return Number of cycles
	 */
	int			getCycle() const;

	/**
	 * @brief Check if notifications are enabled
	 * @return True if notifications are enabled
	 */
	bool		isNotificationEnabled() const;

	/**
	 * @brief Get notification sound file path
	 * @return Path to sound file
	 */
	std::string	getNotificationSound() const;

	/**
	 * @brief Set work session duration
	 * @param duration Duration in minutes
	 */
	void		setWorkDuration(int duration);

	/**
	 * @brief Set break session duration
	 * @param duration Duration in minutes
	 */
	void		setBreakDuration(int duration);

	/**
	 * @brief Enable or disable notifications
	 * @param enabled True to enable notifications
	 */
	void		setNotificationEnabled(bool enabled);

	/**
	 * @brief Set notification sound
	 * @param soundName Name of sound file (without extension)
	 */
	void		setNotificationSound(const std::string& soundName);

	/**
	 * @brief Set number of cycles before long break
	 * @param newCycle Number of cycles
	 */
	void		setRepeatCycle(int newCycle);

	/**
	 * @brief Set long break session duration
	 * @param duration Duration in minutes
	 */
	void		setLongBreakDuration(int duration);

	// Add new getters
	std::string	getWorkMessage() const;
	std::string	getBreakMessage() const;
	std::string	getLongBreakMessage() const;
	std::string	getWorkSound() const;
	std::string	getBreakSound() const;
	std::string	getLongBreakSound() const;
	std::string	getOverlayPrompt() const;

private:
	Config(const Config& other);			// Non-copyable
	Config& operator=(const Config& other);	// Non-assignable
	
	std::string	constructSoundPath(const std::string& soundName) const;
};

#endif // CONFIG_HPP