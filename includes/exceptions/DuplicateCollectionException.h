#ifndef BETA2_DUPLICATECOLLECTIONEXCEPTION_H
#define BETA2_DUPLICATECOLLECTIONEXCEPTION_H

#include "MovieException.h"

class DuplicateCollectionException : public MovieException {
public:
    explicit DuplicateCollectionException(const std::string& name);
};

#endif









