#ifndef BETA2_MOVIEPARSER_H
#define BETA2_MOVIEPARSER_H

#include <vector>
#include <string>

class MovieParser {
public:
    static int parseId(const std::vector<std::string>& tokens);
    static double parseRating(const std::vector<std::string>& tokens);
    static int parseYear(const std::vector<std::string>& tokens);
    static std::vector<std::string> parseGenres(const std::vector<std::string>& tokens);
    static int parseDuration(const std::vector<std::string>& tokens);
};

#endif // BETA2_MOVIEPARSER_H


