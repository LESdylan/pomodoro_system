#include "timer.h"
#include <unistd.h>

Timer::Timer(int workDuration, int breakDuration) 
	: workDuration(workDuration), breakDuration(breakDuration), startTime(0), isRunning(false), isBreakTime(false)
{
}

void Timer::start()
{
	startTime = time(0);
	isRunning = true;
	isBreakTime = false;
}

void Timer::stop()
{
	isRunning = false;
	startTime = 0;
}

void Timer::reset()
{
	stop();
}

void Timer::startBreak()
{
	startTime = time(0);
	isRunning = true;
	isBreakTime = true;
}

void Timer::waitForCompletion()
{
	while (isRunning && getRemainingTime() > 0)
	{
		sleep(1);
	}
	isRunning = false;
}

int Timer::getRemainingTime() const
{
	if (!isRunning)
		return (0);
	
	time_t	currentTime = time(0);
	int		elapsed = static_cast<int>(currentTime - startTime);
	int		duration = isBreakTime ? breakDuration : workDuration;
	int		durationInSeconds = duration * 60;
	int		remaining = durationInSeconds - elapsed;
	
	return (remaining > 0 ? remaining : 0);
}