#ifndef BETA2_FILENOTFOUNDEXCEPTION_H
#define BETA2_FILENOTFOUNDEXCEPTION_H

#include "MovieException.h"

class FileNotFoundException : public MovieException {
public:
    explicit FileNotFoundException(const std::string& filename);
};

#endif

