#ifndef BETA2_MOVIEEXCEPTION_H
#define BETA2_MOVIEEXCEPTION_H

#include <string>
#include <exception>

class MovieException : public std::exception {
private:
    std::string message;

public:
    explicit MovieException(const std::string& msg);
    virtual const char* what() const noexcept override;
};

#endif


