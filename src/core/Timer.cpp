#include <ctime>

#include "Timer.hpp"

Timer::Timer()
	: m_limit(0)
	, m_start(0) {}

void Timer::start()
{
	m_start = std::time(NULL);
}

void Timer::set(Seconds sec)
{
	m_limit = sec;
}

std::time_t Timer::passed()
{
	std::time_t now = std::time(NULL);
	return std::difftime(now, m_start) >= m_limit;
}

bool Timer::expired()
{
	std::time_t now = std::time(NULL);
	return std::difftime(now, m_start) >= m_limit;
}

void Timer::reset()
{
	start();
}
