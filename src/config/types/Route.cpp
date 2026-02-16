#include <string>

#include "http/StatusCode.hpp"
#include "Route.hpp"

Route::Route(StatusCode::Code code, std::string path)
	: code(code)
	, path(path) {}
