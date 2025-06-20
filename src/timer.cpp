#include "timer.h"
#include <unistd.h>

Timer::Timer(int workDuration, int breakDuration) 
	: m_workDuration(workDuration)
	, m_breakDuration(breakDuration)
	, m_startTime(0)
	, m_isRunning(false)
	, m_isBreakTime(false)
{
}

Timer::~Timer()
{
}

void Timer::start()
{
	m_startTime = time(0);
	m_isRunning = true;
	m_isBreakTime = false;
}

void Timer::stop()
{
	m_isRunning = false;
	m_startTime = 0;
}

void Timer::reset()
{
	stop();
}

void Timer::startBreak()
{
	m_startTime = time(0);
	m_isRunning = true;
	m_isBreakTime = true;
}

void Timer::waitForCompletion()
{
	while (m_isRunning && getRemainingTime() > 0) {
		sleep(1);
	}
	m_isRunning = false;
}

int Timer::getRemainingTime() const
{
	if (!m_isRunning)
		return 0;
	
	const time_t currentTime = time(0);
	const int elapsed = static_cast<int>(currentTime - m_startTime);
	const int duration = m_isBreakTime ? m_breakDuration : m_workDuration;
	const int durationInSeconds = duration * 60;
	const int remaining = durationInSeconds - elapsed;
	
	return (remaining > 0 ? remaining : 0);
}