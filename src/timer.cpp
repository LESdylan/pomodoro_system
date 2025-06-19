#include "timer.h"

/**
 * Define the constructor parameter
 * and the default value for private var
 */
Timer::Timer(int workDuration, int breakDuration)
	: workDuration(workDuration), breakDuration(breakDuration), isRunning(false), isBreakTime(false) {}

void    Timer::start()
{
	isRunning = true;
	isBreakTime = false;
	std::cout << "Pomodoro timer started for " << workDuration << " minutes." << std::endl; 
}

void    Timer::stop()
{
	isRunning = false;
	std::cout << "Pomodoro timer stopped." << std::endl;
}

void	Timer::reset()
{
	stop();
	std::cout << "Pomodoro timer reset." << std::endl;
}

void	Timer::startBreak()
{
	isRunning = true;
	isBreakTime = true;
	std::cout << "Break time startef for " << breakDuration << " minutes." << std::endl;
}

void	Timer::waitForCompletion()
{
	if (isRunning)
	{
		if (isBreakTime)
			std::this_thread::sleep_for(std::chrono::minutes(breakDuration));
		else
			std::this_thread::sleep_for(std::chrono::minutes(workDuration));
		isRunning = false;
	}
}
/**GETTERS */
int	Timer::getRemainingTime() const
{
	return (remainingTime);
}