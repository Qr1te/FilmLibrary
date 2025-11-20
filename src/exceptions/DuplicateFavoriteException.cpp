#include "exceptions/DuplicateFavoriteException.h"

DuplicateFavoriteException::DuplicateFavoriteException(const std::string& title)
    : MovieException("Фильм \"" + title + "\" уже находится в избранном") {}


