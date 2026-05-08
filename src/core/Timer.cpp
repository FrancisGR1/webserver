#include <ctime>

#include "Timer.hpp"

std::time_t Timer::now(void)
{
    return std::time(NULL);
}

Timer::Timer()
    : m_limit(0)
    , m_start(0)
    , m_counting_down(false)
{
}

void Timer::start()
{
    m_counting_down = true;
    m_start = now();
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
    return std::difftime(now(), m_start) >= m_limit;
}

bool Timer::expired() const
{
    if (m_counting_down == false)
        return false;

    return std::difftime(now(), m_start) >= m_limit;
}

void Timer::reset()
{
    start();
}
