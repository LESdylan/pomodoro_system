#ifndef TIMER_H
# define	TIMER_H

#include <iostream>
#include <string>
#include <chrono>
#include <thread>

class Timer
{
	public:
		int	workDuration;
		int breakDuration;
		int remainingTime;
		bool isRunning;
		int	isBreakTime;
	public:
		Timer(int workDuration, int breakDuration);
		void	start();
		void	stop();
		void	reset();
		void	startBreak();
		void	waitForCompletion();
		int		getRemainingTime() const;
};
#endif