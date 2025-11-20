#include "exceptions/FileNotFoundException.h"

FileNotFoundException::FileNotFoundException(const std::string& filename)
    : MovieException("File not found: " + filename) {}


