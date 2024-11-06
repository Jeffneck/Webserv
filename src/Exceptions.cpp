// Exceptions.cpp
#include "../includes/Exceptions.hpp"

ParsingException::ParsingException(const std::string &message)
    : message_(message)
{
}

ParsingException::~ParsingException() throw()
{
}

const char *ParsingException::what() const throw()
{
    return message_.c_str();
}
