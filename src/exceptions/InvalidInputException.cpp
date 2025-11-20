#include "exceptions/InvalidInputException.h"

InvalidInputException::InvalidInputException(const std::string& input)
    : MovieException("Invalid input: " + input) {}









