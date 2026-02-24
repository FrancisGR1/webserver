#ifndef TIMER_HPP
#define TIMER_HPP

#include <ctime>

typedef double Seconds;

class Timer
{
	public:
		Timer();
		void start();
		void set(Seconds sec);
		std::time_t passed();
		bool expired();
		void reset();

	private:
		Seconds m_limit;
		std::time_t m_start;
};

#endif // TIMER_HPP
