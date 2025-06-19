#include <iostream>
#include <csignal>
#include <unistd.h>
#include <sys/stat.h>
#include <fstream>
#include <ctime>
#include <cstring>
#include <sstream>
#include "config.hpp"
#include "notification.h"
#include "timer.h"

/**
 * @brief Convert integer to string (C++98 compatible)
 * @param value Integer value to convert
 * @return String representation of the integer
 */
std::string intToString(int value)
{
	std::ostringstream oss;
	
	oss << value;
	return (oss.str());
}

typedef struct s_var
{
	bool		running;
	std::string	logFile;
}				t_var;

typedef struct s_app_state
{
	bool		running;
	std::string	logFile;
	bool		daemonMode;
	
	s_app_state() : running(true), logFile("default.log"), daemonMode(false) {}
	
	void initialize(int /* argc */, char** /* argv */) {}
} t_app_state;

/**
 * @brief Check if application should run in daemon mode
 * @param argc Number of command line arguments
 * @param argv Array of command line arguments
 * @param g_var Global variables structure
 * @return True if daemon mode is requested
 */
bool isDaemon(int argc, char **argv, t_var *g_var)
{
	bool	daemonMode = false;
	int		i = 1;

	while (i < argc)
	{
		std::string arg = argv[i];
		if (arg == "--daemon" || arg == "-d")
		{
			daemonMode = true;
			break;
		}
		i++;
	}
	if (daemonMode)
	{
		g_var->logFile = std::string(getenv("HOME")) + "/.local/share/pomodoro-timer/logs/pomodoro-timer.log";
		system("mkdir -p ~/.local/share/pomodoro-timer/logs");
	}
	else
		g_var->logFile = "pomodoro-timer.log";
	return (daemonMode);
}

/**
 * @brief Log a message to file and console
 * @param message Message to log
 * @param g_var Global variables structure
 */
void logMessage(const std::string& message, t_var *g_var)
{
	std::ofstream	log(g_var->logFile.c_str(), std::ios::app);
	
	if (log.is_open())
	{
		time_t	now = time(0);
		char*	timeStr = ctime(&now);
		
		timeStr[strlen(timeStr) - 1] = '\0';
		log << "[" << timeStr << "] " << message << std::endl;
		log.close();
	}
	std::cout << message << std::endl;
}

/**
 * @brief Handle system signals for graceful shutdown
 * @param signal Signal number received
 */
void signal_handler(int signal)
{
	if (signal == SIGTERM || signal == SIGINT)
	{
		std::cout << "Received signal " << signal << ", shutting down..." << std::endl;
		exit(0);
	}
}

/**
 * @brief Main daemon loop for running Pomodoro sessions
 * @param g_var Global variables structure
 * @param config Configuration object
 * @param timer Timer object
 * @param notification Notification object
 */
void run_daemon(t_var *g_var, Config& config, Timer& timer, Notification& notification)
{
	while (g_var->running)
	{
		logMessage("Starting work session for " + intToString(config.getWorkDuration()) + " minutes", g_var);
		timer.start();
		timer.waitForCompletion();
		if (!g_var->running)
			break;
		logMessage("Work session completed - break time!", g_var);
		notification.showNotification("Time to take a break!");
		notification.playSound(config.getNotificationSound());
		timer.startBreak();
		timer.waitForCompletion();
		if (!g_var->running) 
			break;
		logMessage("Break completed - back to work!", g_var);
		notification.showNotification("Break time is over! Get back to work!");
		notification.playSound(config.getNotificationSound());
		if (g_var->running)
			sleep(1); 
	}
	logMessage("Pomodoro Timer daemon stopped", g_var);
}

/**
 * @brief Main function - entry point of the application
 * @param argc Number of command line arguments
 * @param argv Array of command line arguments
 * @return Exit status
 */
int main(int argc, char **argv)
{
	t_var	g_var;
	
	g_var.running = true;
	g_var.logFile = "/var/log/pomodoro_timer.log";

	bool daemonMode = isDaemon(argc, argv, &g_var);
	
	signal(SIGTERM, signal_handler);
	signal(SIGINT, signal_handler);
	signal(SIGQUIT, signal_handler);
	
	logMessage("Pomodoro Timer Starting...", &g_var);
	
	std::string configPath = daemonMode ?
		std::string(getenv("HOME")) + "/Documents/Personal_project/pomodoro_system/config/settings.cfg" :
		"config/settings.cfg";
	
	Config config(configPath);
	config.loadSettings();
	
	logMessage("Configuration loaded - work: " + intToString(config.getWorkDuration()) +
				"min, break: " + intToString(config.getBreakDuration()) + "min", &g_var);
	
	Timer timer(config.getWorkDuration(), config.getBreakDuration());
	Notification notification;
	
	run_daemon(&g_var, config, timer, notification);
	
	return (0);
}