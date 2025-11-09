#ifndef BETA2_DUPLICATEMOVIEEXCEPTION_H
#define BETA2_DUPLICATEMOVIEEXCEPTION_H

#include "MovieException.h"
#include <string>

class DuplicateMovieException : public MovieException {
public:
    explicit DuplicateMovieException(const std::string& title, int year);
};

#endif

