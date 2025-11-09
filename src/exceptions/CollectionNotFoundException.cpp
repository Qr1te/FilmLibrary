#include "../../includes/exceptions/CollectionNotFoundException.h"

CollectionNotFoundException::CollectionNotFoundException(const std::string& name)
    : MovieException("Коллекция не найдена: " + name) {}



