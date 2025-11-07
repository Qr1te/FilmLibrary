#ifndef BETA2_DUPLICATEFAVORITEEXCEPTION_H
#define BETA2_DUPLICATEFAVORITEEXCEPTION_H

#include "MovieException.h"

class DuplicateFavoriteException : public MovieException {
public:
    explicit DuplicateFavoriteException(int id);
};

#endif


