#ifndef ROUTE_HPP
# define ROUTE_HPP

#include <string>

#include "http/StatusCode.hpp"

struct Route
{
	Route(StatusCode::Code code, std::string path);

	StatusCode::Code code;
	std::string path;
};

#endif // ROUTE_HPP
