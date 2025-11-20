#include "exceptions/EmptyCollectionException.h"

EmptyCollectionException::EmptyCollectionException(const std::string& name)
    : MovieException("Коллекция пуста: " + name) {}



