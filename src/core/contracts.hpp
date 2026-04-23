#ifndef CONTRACT_HPP
#define CONTRACT_HPP
#include <cstdlib>

#include "core/Logger.hpp"

#ifdef NDEBUG
#define CONTRACT_ABORT()
#else
#define CONTRACT_ABORT() std::abort()
#endif

#define REQUIRE(cond, ...)                                                                                             \
    do                                                                                                                 \
    {                                                                                                                  \
        if (!(cond))                                                                                                   \
        {                                                                                                              \
            Logger::fatal("Precondition failed: %s (%s:%d) - " __VA_ARGS__, #cond, __FILE__, __LINE__);                \
            CONTRACT_ABORT();                                                                                          \
        }                                                                                                              \
    } while (0)

#define ENSURE(cond, ...)                                                                                              \
    do                                                                                                                 \
    {                                                                                                                  \
        if (!(cond))                                                                                                   \
        {                                                                                                              \
            Logger::fatal("Postcondition failed: %s (%s:%d) - " __VA_ARGS__, #cond, __FILE__, __LINE__);               \
            CONTRACT_ABORT();                                                                                          \
        }                                                                                                              \
    } while (0)

#define INVARIANT(cond, ...)                                                                                           \
    do                                                                                                                 \
    {                                                                                                                  \
        if (!(cond))                                                                                                   \
        {                                                                                                              \
            Logger::fatal("Invariant violated: %s (%s:%d) - " __VA_ARGS__, #cond, __FILE__, __LINE__);                 \
            CONTRACT_ABORT();                                                                                          \
        }                                                                                                              \
    } while (0)

#endif // CONTRACT_HPP
