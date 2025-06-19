#ifndef CONFIG_HPP
# define CONFIG_HPP
# include <iostream>
# include <fstream>
# include <string>

class Config 
{
	private:
		std::string	filename;
		int			workDuration;	// in minutes
		int			breakDuration;	// in minutes
		int			longBreakDuration;	// in minutes
		int			repeatCycle;
		bool		notificationEnabled;
		std::string	notificationSound;

	public:
		Config(const std::string& filename);
		void		loadSettings() const;
		void		saveSettings() const;
		int			getWorkDuration() const;
		int			getBreakDuration() const;
		int			getLongBreakDuration() const;
		int			getCycle() const;
		bool		isNotificationEnabled() const;
		std::string	getNotificationSound() const;
		void		setWorkDuration(int duration);
		void		setBreakDuration(int duration);
		void		setNotificationEnabled(bool enabled);
		void		setNotificationSound(const  std::string& soundName);
		void		setRepeatCycle(int newCycle);
		void		setLongBreakDuration(int duration);
}
#endif