#include "../../includes/exceptions/DuplicateCollectionException.h"

DuplicateCollectionException::DuplicateCollectionException(const std::string& name)
    : MovieException("Collection with this name already exists: " + name) {}


