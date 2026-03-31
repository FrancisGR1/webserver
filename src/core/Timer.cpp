#include <ctime>

#include "Timer.hpp"

Timer::Timer()
	: m_limit(0)
	, m_start(0)
	, m_counting_down(false) {}

void Timer::start()
{
	m_counting_down = true;
	m_start = std::time(NULL);
}

void Timer::stop()
{
	m_counting_down = false;
}

void Timer::set(Seconds sec)
{
	m_limit = sec;
}

std::time_t Timer::passed() const
{
	std::time_t now = std::time(NULL);
	return std::difftime(now, m_start) >= m_limit;
}

bool Timer::expired() const
{
	if (m_counting_down == false)
		return false;

	std::time_t now = std::time(NULL);
	return std::difftime(now, m_start) >= m_limit;
}

void Timer::reset()
{
	start();
}
