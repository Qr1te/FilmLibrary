#include "parsers/MovieParser.h"

#include <sstream>
#include <stdexcept>
#include <string_view>

static bool string_contains(std::string_view str, char c) {
    return str.contains(c);
}

int MovieParser::parseId(const std::vector<std::string>& tokens) {
    if (tokens.empty() || tokens[0].empty()) {
        return 0;
    }
    
    try {
        return std::stoi(tokens[0]);
    } catch (const std::invalid_argument&) {
        return 0;
    } catch (const std::out_of_range&) {
        return 0;
    }
}

double MovieParser::parseRating(const std::vector<std::string>& tokens) {
    if (tokens.size() <= 2 || tokens[2].empty()) {
        return 0.0;
    }
    
    try {
        return std::stod(tokens[2]);
    } catch (const std::invalid_argument&) {
        return 0.0;
    } catch (const std::out_of_range&) {
        return 0.0;
    }
}

int MovieParser::parseYear(const std::vector<std::string>& tokens) {
    if (tokens.size() <= 3 || tokens[3].empty()) {
        return 0;
    }
    
    try {
        return std::stoi(tokens[3]);
    } catch (const std::invalid_argument&) {
        return 0;
    } catch (const std::out_of_range&) {
        return 0;
    }
}

std::vector<std::string> MovieParser::parseGenres(const std::vector<std::string>& tokens) {
    std::vector<std::string> genres;
    
    if (tokens.size() <= 4) {
        return genres;
    }
    
    std::string genreStr = tokens[4];
    if (genreStr.empty()) {
        return genres;
    }
    
    if (!string_contains(genreStr, ';')) {
        genres.push_back(genreStr);
        return genres;
    }
    
    std::istringstream genreStream(genreStr);
    std::string genre;
    while (std::getline(genreStream, genre, ';')) {
        if (!genre.empty()) {
            genres.push_back(genre);
        }
    }
    
    return genres;
}

int MovieParser::parseDuration(const std::vector<std::string>& tokens) {
    if (tokens.size() <= 10 || tokens[10].empty()) {
        return 0;
    }
    
    try {
        return std::stoi(tokens[10]);
    } catch (const std::invalid_argument&) {
        return 0;
    } catch (const std::out_of_range&) {
        return 0;
    }
}

