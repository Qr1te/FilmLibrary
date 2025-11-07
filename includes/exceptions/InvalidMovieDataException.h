#ifndef BETA2_INVALIDMOVIEDATAEXCEPTION_H
#define BETA2_INVALIDMOVIEDATAEXCEPTION_H

#include "MovieException.h"

class InvalidMovieDataException : public MovieException {
public:
    explicit InvalidMovieDataException(const std::string& data);
};

#endif


