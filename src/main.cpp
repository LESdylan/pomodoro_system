#include <iostream>
#include <csignal>
#include <unistd.h>
#include <sys/stat.h>
#include <fstream>
#include <ctime>
#include <cstring>
#include <sstream>
#include <cstdio>
#include <cstdlib>
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

	// PID of the systemd-inhibit child held during a break (-1 when none).
	// Read in signalHandler so a shutdown mid-break releases the sleep inhibitor.
	volatile pid_t g_inhibitorPid = -1;

	/**
	 * @brief Handle system signals for graceful shutdown
	 */
	void signalHandler(int signal)
	{
		if (signal == SIGTERM || signal == SIGINT) {
			std::cout << "Received signal " << signal << ", shutting down..." << std::endl;
			g_appState.running = false;
			if (g_inhibitorPid > 0)
				kill(g_inhibitorPid, SIGTERM); // release the sleep inhibitor before exiting
			_exit(0); // async-signal-safe; the endl above already flushed

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
	 * @brief Run a shell command and return its exit status (-1 on failure)
	 */
	int runStatus(const std::string& command)
	{
		const int rc = system(command.c_str());
		if (rc == -1 || !WIFEXITED(rc))
			return -1;
		return WEXITSTATUS(rc);
	}

	/**
	 * @brief Build a quoted invocation of the session helper: "<helper>" <sub>
	 */
	std::string sessionCmd(const std::string& helper, const std::string& sub)
	{
		return "\"" + helper + "\" " + sub;
	}

	/**
	 * @brief Locate a helper script (installed bin path first, then dev ./scripts)
	 */
	std::string findScript(const std::string& name)
	{
		const char* home = getenv("HOME");
		if (home) {
			const std::string installed =
				std::string(home) + "/.local/share/pomodoro-timer/bin/" + name;
			std::ifstream f(installed.c_str());
			if (f.good())
				return installed;
		}
		const std::string dev = "scripts/" + name;
		std::ifstream df(dev.c_str());
		if (df.good())
			return dev;
		if (home)
			return std::string(home) + "/.local/share/pomodoro-timer/bin/" + name;
		return dev;
	}

	/**
	 * @brief Fire the Dockerized notifier (HTML email + ntfy push) in the background.
	 *
	 * Non-blocking and best-effort: the wrapper is a no-op when docker / the image /
	 * the .env file are missing, so the daemon is never delayed or affected.
	 *   event : "break-start" or "back-to-work"
	 */
	void runNotifier(const std::string& event, const std::string& detail)
	{
		const std::string path = findScript("pomodoro-notify.sh");
		const std::string cmd =
			"\"" + path + "\" " + event + " \"" + detail + "\" >/dev/null 2>&1 &";
		system(cmd.c_str());
	}

	/**
	 * @brief True if the session is currently locked
	 */
	bool isLocked(const std::string& helper)
	{
		return runStatus(sessionCmd(helper, "status")) == 0;
	}

	/**
	 * @brief Fork a systemd-inhibit child blocking sleep+idle for holdSecs.
	 *
	 * This is what guarantees the machine never suspends during a break and that
	 * background processes (AI jobs, builds, ...) keep running. The pid is stored in
	 * g_inhibitorPid so the signal handler can release it on shutdown.
	 */
	void startInhibitor(int holdSecs)
	{
		const pid_t pid = fork();
		if (pid == 0) {
			const std::string secStr = intToString(holdSecs);
			char* const argv[] = {
				const_cast<char*>("systemd-inhibit"),
				const_cast<char*>("--what=sleep:idle"),
				const_cast<char*>("--who=pomodoro-timer"),
				const_cast<char*>("--why=Pomodoro break in progress"),
				const_cast<char*>("--mode=block"),
				const_cast<char*>("sleep"),
				const_cast<char*>(secStr.c_str()),
				static_cast<char*>(0)
			};
			execvp("systemd-inhibit", argv);
			_exit(127); // exec failed
		} else if (pid > 0) {
			g_inhibitorPid = pid;
		}
	}

	/**
	 * @brief Release the sleep inhibitor held by startInhibitor (if any)
	 */
	void stopInhibitor()
	{
		if (g_inhibitorPid > 0) {
			kill(g_inhibitorPid, SIGTERM);
			waitpid(g_inhibitorPid, 0, 0);
			g_inhibitorPid = -1;
		}
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
	 * @brief Native session-lock break: lock, hold for the duration, then auto-unlock.
	 *
	 * The OS lock screen still accepts the user's password for an early return. On a
	 * natural end we ask the desktop to auto-unlock and remember whether that worked,
	 * so a desktop that cannot auto-unlock switches to the overlay on the next break.
	 */
	void runNativeBreak(int seconds, const std::string& helper,
						const std::string& lockStep, const std::string& prompt,
						AppState* appState)
	{
		bool locked = (runStatus(lockStep) == 0);
		if (!locked) {
			// A custom lock_command may not self-confirm; poll the state briefly.
			for (int i = 0; i < 6 && appState->running; ++i) {
				if (isLocked(helper)) { locked = true; break; }
				sleep(1);
			}
		}
		if (!locked) {
			logMessage("Native lock not confirmed; falling back to overlay", appState);
			runOverlay(seconds, prompt);
			return;
		}

		const time_t start = time(0);
		const int grace = 3; // ignore "unlocked" right after locking (locker arm-up race)
		bool earlySkip = false;
		while (appState->running) {
			const int elapsed = static_cast<int>(time(0) - start);
			if (elapsed >= seconds)
				break;
			if (elapsed > grace && !isLocked(helper)) {
				earlySkip = true; // user authenticated to come back early
				break;
			}
			sleep(2);
		}

		if (!appState->running)
			return;
		if (earlySkip) {
			logMessage("Break ended early (user unlocked the session)", appState);
			return;
		}

		// Natural end: ask the desktop to auto-unlock and learn the result.
		if (runStatus(sessionCmd(helper, "unlock")) == 0) {
			runStatus(sessionCmd(helper, "mark-ok"));
			logMessage("Break complete; session auto-unlocked", appState);
		} else {
			runStatus(sessionCmd(helper, "mark-failed"));
			logMessage("Desktop did not auto-unlock (will use overlay next break). "
					   "Enter your password to return.", appState);
		}
	}

	/**
	 * @brief Enforce a break: dispatch to native lock or overlay, holding a sleep
	 *        inhibitor for the whole duration so nothing suspends and jobs keep running.
	 */
	void runEnforcedBreak(int seconds, Config& config, AppState* appState,
						  const std::string& prompt)
	{
		const std::string helper = findScript("pomodoro-session.sh");
		const std::string mode = config.getBreakMode();
		const std::string lockStep = config.getLockCommand().empty()
			? sessionCmd(helper, "lock")
			: config.getLockCommand();

		if (config.getInhibitSleep())
			startInhibitor(seconds + 5);

		bool useNative;
		if (mode == "overlay")
			useNative = false;
		else if (mode == "lock")
			useNative = true;
		else // "hybrid" (default): native only when the desktop can auto-unlock
			useNative = (runStatus(sessionCmd(helper, "can-autounlock")) == 0);

		if (useNative)
			runNativeBreak(seconds, helper, lockStep, prompt, appState);
		else
			runOverlay(seconds, prompt);

		stopInhibitor();
	}

	/**
	 * @brief --test-lock: lock, wait, auto-unlock; calibrate the auto-unlock state.
	 */
	int runTestLock()
	{
		const std::string helper = findScript("pomodoro-session.sh");
		std::cout << "Using session helper: " << helper << std::endl;
		std::cout << "Auto-unlock guess for this desktop: "
				  << (runStatus(sessionCmd(helper, "can-autounlock")) == 0 ? "yes" : "no")
				  << std::endl;

		std::cout << "Locking session..." << std::endl;
		if (runStatus(sessionCmd(helper, "lock")) != 0) {
			std::cout << "ERROR: could not confirm a lock. "
						 "Breaks will use the overlay fallback." << std::endl;
			return 1;
		}
		std::cout << "Locked. Waiting 5s before attempting auto-unlock..." << std::endl;
		sleep(5);

		if (runStatus(sessionCmd(helper, "unlock")) == 0) {
			runStatus(sessionCmd(helper, "mark-ok"));
			std::cout << "SUCCESS: native lock + auto-unlock works. "
						 "Breaks will use the real session lock." << std::endl;
		} else {
			runStatus(sessionCmd(helper, "mark-failed"));
			std::cout << "Native auto-unlock is NOT supported here. "
						 "Breaks will use the fullscreen overlay instead.\n"
						 "Enter your password to unlock now." << std::endl;
		}
		return 0;
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
			runNotifier("break-start", breakType + " - " + intToString(breakDuration) + " min");

			// Enforce the break: lock the session (or overlay) while holding a sleep
			// inhibitor so the machine never suspends and background jobs keep running.
			runEnforcedBreak(breakDuration * 60, config, appState, config.getOverlayPrompt());

			if (!appState->running)
				break;

			logMessage(breakType + " completed - back to work!", appState);
			notification.showNotification("Break time is over! Get back to work!");
			notification.playSound(config.getWorkSound());
			runNotifier("back-to-work", "Focus for " + intToString(config.getWorkDuration()) + " min");
			
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

	// Calibration mode: lock, wait, auto-unlock, and record what this desktop supports.
	for (int i = 1; i < argc; ++i) {
		if (std::string(argv[i]) == "--test-lock")
			return runTestLock();
	}

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