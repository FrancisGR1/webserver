#ifndef TIMER_HPP
#define TIMER_HPP

#include <ctime>

typedef double Seconds;

class Timer
{
	public:
		Timer();
		void start();
		void stop();
		void set(Seconds sec);
		std::time_t passed() const;
		bool expired() const;
		void reset();

	private:
		Seconds m_limit;
		std::time_t m_start;
		bool m_counting_down;

};

#endif // TIMER_HPP
