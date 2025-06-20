#include <iostream>
#include <csignal>
#include <unistd.h>
#include <sys/stat.h>
#include <fstream>
#include <ctime>
#include <cstring>
#include <sstream>
#include <sys/wait.h>
#include "config.hpp"
#include "notification.h"
#include "timer.h"

namespace {
	/**
	 * @brief Convert integer to string (C++98 compatible)
	 */
	std::string intToString(int value)
	{
		std::ostringstream oss;
		oss << value;
		return oss.str();
	}

	struct AppState
	{
		bool		running;
		std::string	logFile;
		
		AppState() : running(true), logFile("/var/log/pomodoro_timer.log") {}
	};

	AppState g_appState;

	/**
	 * @brief Handle system signals for graceful shutdown
	 */
	void signalHandler(int signal)
	{
		if (signal == SIGTERM || signal == SIGINT) {
			std::cout << "Received signal " << signal << ", shutting down..." << std::endl;
			g_appState.running = false;
			exit(0);
		}
	}

	/**
	 * @brief Check if application should run in daemon mode
	 */
	bool isDaemonMode(int argc, char **argv, AppState* appState)
	{
		bool daemonMode = false;
		
		for (int i = 1; i < argc; ++i) {
			const std::string arg = argv[i];
			if (arg == "--daemon" || arg == "-d") {
				daemonMode = true;
				break;
			}
		}
		
		if (daemonMode) {
			const char* home = getenv("HOME");
			if (home) {
				appState->logFile = std::string(home) + "/.local/share/pomodoro-timer/logs/pomodoro-timer.log";
				system("mkdir -p ~/.local/share/pomodoro-timer/logs");
			}
		} else {
			appState->logFile = "pomodoro-timer.log";
		}
		
		return daemonMode;
	}

	/**
	 * @brief Parse command line arguments for config path
	 */
	std::string getConfigPath(int argc, char **argv)
	{
		for (int i = 1; i < argc; ++i) {
			const std::string arg = argv[i];
			if (arg == "--config" && i + 1 < argc)
				return argv[i + 1];
		}
		return "config/settings.cfg";
	}

	/**
	 * @brief Log a message to file and console
	 */
	void logMessage(const std::string& message, const AppState* appState)
	{
		std::ofstream log(appState->logFile.c_str(), std::ios::app);
		
		if (log.is_open()) {
			time_t now = time(0);
			char* timeStr = ctime(&now);
			timeStr[strlen(timeStr) - 1] = '\0';
			log << "[" << timeStr << "] " << message << std::endl;
			log.close();
		}
		std::cout << message << std::endl;
	}

	/**
	 * @brief Run the overlay program to block interaction during breaks
	 */
	void runOverlay(int seconds, const std::string& prompt)
	{
		const pid_t pid = fork();
		if (pid == 0) {
			// Child process: run overlay
			const std::string secStr = intToString(seconds);
			
			// Try different paths for overlay binary
			execlp("overlay_timer_qt", "overlay_timer_qt", secStr.c_str(), prompt.c_str(), static_cast<char*>(0));
			execlp("./overlay_timer_qt", "overlay_timer_qt", secStr.c_str(), prompt.c_str(), static_cast<char*>(0));
			_exit(1);
		} else if (pid > 0) {
			int status;
			waitpid(pid, &status, 0);
		}
	}

	/**
	 * @brief Main daemon loop for running Pomodoro sessions
	 */
	void runDaemon(AppState* appState, Config& config, Timer& timer, Notification& notification)
	{
		int cycleCount = 0;
		
		while (appState->running) {
			// Work session
			logMessage("Starting work session for " + intToString(config.getWorkDuration()) + " minutes", appState);
			notification.showNotification(config.getWorkMessage());
			notification.playSound(config.getWorkSound());
			timer.start();
			timer.waitForCompletion();
			
			if (!appState->running)
				break;

			// Break or long break
			cycleCount++;
			const bool isLongBreak = (cycleCount % config.getCycle() == 0);
			const int breakDuration = isLongBreak ? config.getLongBreakDuration() : config.getBreakDuration();
			const std::string breakMsg = isLongBreak ? config.getLongBreakMessage() : config.getBreakMessage();
			const std::string breakSound = isLongBreak ? config.getLongBreakSound() : config.getBreakSound();

			const std::string breakType = isLongBreak ? "Long break" : "Break";
			logMessage(breakType + " starting for " + intToString(breakDuration) + " minutes", appState);
			notification.showNotification(breakMsg);
			notification.playSound(breakSound);

			// Block screen with overlay
			runOverlay(breakDuration * 60, config.getOverlayPrompt());

			timer.startBreak();
			timer.waitForCompletion();
			
			if (!appState->running) 
				break;
				
			logMessage(breakType + " completed - back to work!", appState);
			notification.showNotification("Break time is over! Get back to work!");
			notification.playSound(config.getWorkSound());
			
			if (appState->running)
				sleep(1);
		}
		logMessage("Pomodoro Timer daemon stopped", appState);
	}
}

/**
 * @brief Main function - entry point of the application
 */
int main(int argc, char **argv)
{
	isDaemonMode(argc, argv, &g_appState);
	
	signal(SIGTERM, signalHandler);
	signal(SIGINT, signalHandler);
	signal(SIGQUIT, signalHandler);
	
	logMessage("Pomodoro Timer Starting...", &g_appState);

	const std::string configPath = getConfigPath(argc, argv);
	std::cout << "Using config file: " << configPath << std::endl;

	Config config(configPath);
	config.loadSettings();
	
	logMessage("Configuration loaded - work: " + intToString(config.getWorkDuration()) +
			  "min, break: " + intToString(config.getBreakDuration()) + "min", &g_appState);
	
	Timer timer(config.getWorkDuration(), config.getBreakDuration());
	Notification notification;
	
	runDaemon(&g_appState, config, timer, notification);
	
	return 0;
}