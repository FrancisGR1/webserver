#ifndef LISTENER_HPP
#define LISTENER_HPP

#include <string>

struct Listener
{
    // construct
    Listener(const char* host, const char* port);

    // api
    bool operator==(const Listener& other) const;

    // data
    std::string host;
    std::string port;
};

#endif // LISTENER_HPP
