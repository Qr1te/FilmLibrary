#include "../../includes/exceptions/MovieNotFoundException.h"

MovieNotFoundException::MovieNotFoundException(int id)
    : MovieException("Movie with ID " + std::to_string(id) + " not found") {}

MovieNotFoundException::MovieNotFoundException(const std::string& criteria)
    : MovieException("Movie not found with criteria: " + criteria) {}

