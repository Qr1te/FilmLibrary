#ifndef BETA2_EMPTYCOLLECTIONEXCEPTION_H
#define BETA2_EMPTYCOLLECTIONEXCEPTION_H

#include "MovieException.h"

class EmptyCollectionException : public MovieException {
public:
    explicit EmptyCollectionException(const std::string& name);
};

#endif









