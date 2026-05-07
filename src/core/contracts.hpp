#ifndef CONTRACT_HPP
#define CONTRACT_HPP

#include "core/Logger.hpp"
#include <cstdlib>
#include <stdexcept>

#ifdef NDEBUG
#define CONTRACT_FAIL(kind, cond, msg)                                                                                 \
    throw std::runtime_error(kind " failed: " #cond " (" __FILE__ ":" + std::to_string(__LINE__) + ") - " msg)
#else
#define CONTRACT_FAIL(kind, cond, msg)                                                                                 \
    do                                                                                                                 \
    {                                                                                                                  \
        Logger::fatal(kind " failed: %s (%s:%d) - %s", #cond, __FILE__, __LINE__, msg);                                \
        std::abort();                                                                                                  \
    } while (0)
#endif

#define REQUIRE(cond, msg)                                                                                             \
    do                                                                                                                 \
    {                                                                                                                  \
        if (!(cond))                                                                                                   \
            CONTRACT_FAIL("Precondition", cond, msg);                                                                  \
    } while (0)

#define ENSURE(cond, msg)                                                                                              \
    do                                                                                                                 \
    {                                                                                                                  \
        if (!(cond))                                                                                                   \
            CONTRACT_FAIL("Postcondition", cond, msg);                                                                 \
    } while (0)

#define INVARIANT(cond, msg)                                                                                           \
    do                                                                                                                 \
    {                                                                                                                  \
        if (!(cond))                                                                                                   \
            CONTRACT_FAIL("Invariant", cond, msg);                                                                     \
    } while (0)
