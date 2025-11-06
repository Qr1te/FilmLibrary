#include "../../includes/exceptions/InvalidMovieDataException.h"

InvalidMovieDataException::InvalidMovieDataException(const std::string& data)
    : MovieException("Invalid movie data: " + data) {}

