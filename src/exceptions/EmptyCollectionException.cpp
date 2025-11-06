#include "../../includes/exceptions/EmptyCollectionException.h"

EmptyCollectionException::EmptyCollectionException(const std::string& name)
    : MovieException("Collection is empty: " + name) {}

