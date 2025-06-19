#include <iostream>
#include <csignal>
#include <unistd.h>
#include <sys/stat.h>
#include <fstream>
#include <ctime>
#include <cstring>
#include "config.h"
#include "notification.h"
#include "timer.h"

typedef struct s_var
{
	bool		running;
	std::string	logFile;
}				t_var;

bool    isDaemon(int argc, char **argv, t_var *g_var)
{
	bool    daemonMode = false;
	int i;
	int size;

	size = argc - 1;
	std::string daemon;
	while (i < size)
	{
		daemon = argv[i];
		if (daemon == "--daemon" || daemon == "-d")
		{
			daemonMode = true;
			break ;
		}
		argv++;
	}
	if (daemonMode)
		g_var->logFile = std::string(getenv("HOME")) + "/.local/share/pomodoro-timer/logs/pomodoro-timer.log";
		system("mkdir -p ~/.local/share/pomodoro-timer/logs");
	else
		g_var->logFile = "pomodoro-timer.log";
}

void    signal_handler(t_var *var, int signal)
{   
	if (signal == SIGTERM || signal == SIGINT)
	{
		var->running = false;
		std::cout << "Received signal" << signal << ", shutting down..." << std::endl;
	}
}

void	set_signal(int sign, void *(fn)(t_var, int))
{
	signal(sign, fn);
}

void run_daemon(t_var	*g_var)
{
	while(g_var->running)
	{
		logMessage("Starting work session for " + std::to_string(config.getWorkDuration()) + " minutes");
		timer.start();
		timer.waitForCompletion();
		if (!g_var->running)
			break ;
		logMessage("work session completed - break time!");
		notification.showNotification("time to take a break!");
		notification.playsound(config.getNotificationSound());
		timer.waitForCompletion();
		if (!g_var->running) 
			break ;
		logMessage("Break Completed - back to work!");
		notification.showNotification("Brak time is over! Get back to work!");
		notification.playSound(config.getNotificationSound());
		if (g_var->running)
			sleep(1); 
	}
	logMessage("Pomodoro Timer daemon stopped");
}

int main(int argc, char **argv)
{
	bool    daemonMode;
	t_var	*g_var;
	std::string configPath;

	daemonMode = false;
	g_var->running = true;
	g_var->logFile = "/var/log/pomodoro_timer.log";
	std::string daemon;

	daemon = argv[1];
	signal(SIGTERM, signalHandler);
	signal(SIGINT, signalHandler);
	signal(SIGQUIT, signalHandler);
	logMessage("Pomodoro Timer Starting...");
	configPath = daemon_mode ?
		std::string(getenv("HOME")) + "/Document/Personal_project/pomodoro_timer/config/settings.cfg" :
		"config/settings.cfg";
	Config config(configPath);
	config.loadSettings();
	logMessage("Configuration loaded - work: " + std::to_string(config.getWorkDuration()) +
				"min, break: " + std::to_string(config.getBreakDuration()) + "min");
	Timer timer(config.getWorkDuration(), config. getBreakDuration());
	Notification notification;
	run_daemon(g_var);
}