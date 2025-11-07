#include "../../includes/exceptions/MovieException.h"

MovieException::MovieException(const std::string& msg) : message(msg) {}

const char* MovieException::what() const noexcept {
    return message.c_str();
}


