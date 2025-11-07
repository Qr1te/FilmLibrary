#ifndef BETA2_COLLECTIONNOTFOUNDEXCEPTION_H
#define BETA2_COLLECTIONNOTFOUNDEXCEPTION_H

#include "MovieException.h"

class CollectionNotFoundException : public MovieException {
public:
    explicit CollectionNotFoundException(const std::string& name);
};

#endif


