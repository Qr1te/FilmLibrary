#ifndef BETA2_EXCEPTIONS_H
#define BETA2_EXCEPTIONS_H

#include <string>
#include <exception>

class MovieException : public std::exception {
private:
    std::string message;

public:
    explicit MovieException(const std::string& msg) : message(msg) {}
    virtual const char* what() const noexcept override {
        return message.c_str();
    }
};

class FileNotFoundException : public MovieException {
public:
    explicit FileNotFoundException(const std::string& filename)
        : MovieException("File not found: " + filename) {}
};

class InvalidMovieDataException : public MovieException {
public:
    explicit InvalidMovieDataException(const std::string& data)
        : MovieException("Invalid movie data: " + data) {}
};

class MovieNotFoundException : public MovieException {
public:
    explicit MovieNotFoundException(int id)
        : MovieException("Movie with ID " + std::to_string(id) + " not found") {}

    explicit MovieNotFoundException(const std::string& criteria)
        : MovieException("Movie not found with criteria: " + criteria) {}
};

class DuplicateFavoriteException : public MovieException {
public:
    explicit DuplicateFavoriteException(int id)
        : MovieException("Movie with ID " + std::to_string(id) + " is already in favorites") {}
};

class InvalidInputException : public MovieException {
public:
    explicit InvalidInputException(const std::string& input)
        : MovieException("Invalid input: " + input) {}
};


#endif//BETA2_EXCEPTIONS_H
