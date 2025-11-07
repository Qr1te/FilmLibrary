#include "../../includes/exceptions/CollectionNotFoundException.h"

CollectionNotFoundException::CollectionNotFoundException(const std::string& name)
    : MovieException("Collection not found: " + name) {}


