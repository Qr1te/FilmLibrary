#ifndef BETA2_INVALIDINPUTEXCEPTION_H
#define BETA2_INVALIDINPUTEXCEPTION_H

#include "MovieException.h"

class InvalidInputException : public MovieException {
public:
    explicit InvalidInputException(const std::string& input);
};

#endif


