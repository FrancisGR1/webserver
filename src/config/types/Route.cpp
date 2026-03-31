#include <string>

#include "Route.hpp"
#include "http/StatusCode.hpp"

Route::Route(StatusCode::Code code, std::string path)
    : code(code)
    , raw_path(path)
{
}
