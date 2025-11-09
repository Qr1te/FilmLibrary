#include "../../includes/exceptions/DuplicateMovieException.h"

DuplicateMovieException::DuplicateMovieException(const std::string& title, int year)
    : MovieException("Фильм \"" + title + "\" (" + std::to_string(year) + ") уже существует в коллекции") {}

