#ifndef BETA2_DUPLICATEFAVORITEEXCEPTION_H
#define BETA2_DUPLICATEFAVORITEEXCEPTION_H

#include "MovieException.h"
#include <string>

class DuplicateFavoriteException : public MovieException {
public:
    explicit DuplicateFavoriteException(const std::string& title);
};

#endif


