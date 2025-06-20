#ifndef TIMER_H
#define TIMER_H

#include <iostream>
#include <string>
#include <ctime>

/**
 * @brief Timer class for managing work and break sessions in Pomodoro technique
 */
class Timer
{
private:
	int		m_workDuration;
	int		m_breakDuration;
	time_t	m_startTime;
	bool	m_isRunning;
	bool	m_isBreakTime;

public:
	/**
	 * @brief Constructor for Timer class
	 * @param workDuration Duration of work session in minutes
	 * @param breakDuration Duration of break session in minutes
	 */
	Timer(int workDuration, int breakDuration);
	~Timer();

	/**
	 * @brief Start a work session timer
	 */
	void	start();

	/**
	 * @brief Stop the current timer session
	 */
	void	stop();

	/**
	 * @brief Reset the timer to initial state
	 */
	void	reset();

	/**
	 * @brief Start a break session timer
	 */
	void	startBreak();

	/**
	 * @brief Wait for the current session to complete
	 */
	void	waitForCompletion();

	/**
	 * @brief Get remaining time in current session
	 * @return Remaining time in seconds
	 */
	int		getRemainingTime() const;

private:
	Timer(const Timer& other);					// Non-copyable
	Timer& operator=(const Timer& other);		// Non-assignable
};

#endif // TIMER_H