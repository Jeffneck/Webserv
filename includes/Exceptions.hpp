// Exceptions.hpp
#ifndef EXCEPTIONS_HPP
#define EXCEPTIONS_HPP

#include <exception>
#include <string>

class ParsingException : public std::exception
{
public:
    ParsingException(const std::string &message);
    virtual ~ParsingException() throw();
    virtual const char *what() const throw();

private:
    std::string message_;
};

#endif // EXCEPTIONS_HPP
