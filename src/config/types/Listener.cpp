#include "config/types/Listener.hpp"

Listener::Listener(const char* host, const char* port)
    : host(host)
    , port(port)
{
}

bool Listener::operator==(const Listener& other) const
{
    return host == other.host && port == other.port;
}
