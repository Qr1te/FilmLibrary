#include "exceptions/DuplicateMovieException.h"
#include <format>

DuplicateMovieException::DuplicateMovieException(const std::string& title, int year)
    : MovieException(std::format("Фильм \"{}\" ({}) уже существует в коллекции", title, year)) {}

