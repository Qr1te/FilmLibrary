#include "exceptions/DuplicateCollectionException.h"

DuplicateCollectionException::DuplicateCollectionException(const std::string& name)
    : MovieException("Коллекция с таким именем уже существует: " + name) {}



