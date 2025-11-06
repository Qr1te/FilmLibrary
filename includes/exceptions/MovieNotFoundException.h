#ifndef BETA2_MOVIENOTFOUNDEXCEPTION_H
#define BETA2_MOVIENOTFOUNDEXCEPTION_H

#include "MovieException.h"
#include <string>

class MovieNotFoundException : public MovieException {
public:
    explicit MovieNotFoundException(int id);
    explicit MovieNotFoundException(const std::string& criteria);
};

#endif

