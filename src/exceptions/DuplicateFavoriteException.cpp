#include "../../includes/exceptions/DuplicateFavoriteException.h"

DuplicateFavoriteException::DuplicateFavoriteException(int id)
    : MovieException("Movie with ID " + std::to_string(id) + " is already in favorites") {}

