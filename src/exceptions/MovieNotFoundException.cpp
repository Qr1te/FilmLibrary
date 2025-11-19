#include "../../includes/exceptions/MovieNotFoundException.h"
#include <format>

MovieNotFoundException::MovieNotFoundException(int id)
    : MovieException(std::format("Movie with ID {} not found", id)) {}

MovieNotFoundException::MovieNotFoundException(const std::string& criteria)
    : MovieException(std::format("Movie not found with criteria: {}", criteria)) {}


