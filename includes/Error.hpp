// ERROR.hpp
#ifndef ERROR_HPP
#define ERROR_HPP

#include <string>
#include "Server.hpp"
#include "HttpResponse.hpp"

HttpResponse handleError(int statusCode, const std::string &errorPagePath); 

#endif // ERROR_HPP