#include "models/movie.h"
#include "parsers/MovieParser.h"

#include <iostream>
#include <sstream>
#include <algorithm>
#include <ranges>
#include <cctype>
#include <stdexcept>
#include <string_view>

static bool string_contains(std::string_view str, char c) {
    return str.find(c) != std::string_view::npos;
}

static std::string trim(std::string_view str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string_view::npos) return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return std::string(str.substr(first, (last - first + 1)));
}

Movie::Movie(const Data& data)
    : id(data.id), title(data.title), rating(data.rating), year(data.year),
      genres(data.genres), director(data.director), description(data.description),
      posterPath(data.posterPath), country(data.country), actors(data.actors), duration(data.duration) {}

Movie::Movie(int id, const std::string& title, double rating, int year,
             const std::string& genre, const std::string& director,
             const std::string& description)
    : id(id), title(title), rating(rating), year(year),
      director(director), description(description), posterPath(""),
      country(""), actors(""), duration(0) {
    if (!genre.empty()) {
        genres.push_back(genre);
    }
}

int Movie::getId() const {
    return id;
}

const std::string& Movie::getTitle() const {
    return title;
}

double Movie::getRating() const {
    return rating;
}

int Movie::getYear() const {
    return year;
}

const std::vector<std::string>& Movie::getGenres() const {
    return genres;
}

const std::string& Movie::getDirector() const {
    return director;
}

const std::string& Movie::getDescription() const {
    return description;
}

const std::string& Movie::getPosterPath() const {
    return posterPath;
}

const std::string& Movie::getCountry() const {
    return country;
}

const std::string& Movie::getActors() const {
    return actors;
}

int Movie::getDuration() const {
    return duration;
}

void Movie::setId(int newId) {
    id = newId;
}

void Movie::setTitle(std::string_view newTitle) {
    title = std::string(newTitle);
}

void Movie::setRating(double newRating) {
    rating = newRating;
}

void Movie::setYear(int newYear) {
    year = newYear;
}

void Movie::setGenres(const std::vector<std::string>& newGenres) {
    genres = newGenres;
}

void Movie::setDirector(std::string_view newDirector) {
    director = std::string(newDirector);
}

void Movie::setDescription(std::string_view newDescription) {
    description = std::string(newDescription);
}

void Movie::setPosterPath(std::string_view newPath) {
    posterPath = std::string(newPath);
}

void Movie::setCountry(std::string_view newCountry) {
    country = std::string(newCountry);
}

void Movie::setActors(std::string_view newActors) {
    actors = std::string(newActors);
}

void Movie::setDuration(int newDuration) {
    duration = newDuration;
}

void Movie::addGenre(std::string_view genre) {
    if (!genre.empty() && std::ranges::find(genres, std::string(genre)) == genres.end()) {
        genres.emplace_back(genre);
    }
}

std::string Movie::getGenreString() const {
    if (genres.empty()) {
        return "";
    }
    std::ostringstream oss;
    for (size_t i = 0; i < genres.size(); ++i) {
        if (i > 0) oss << ", ";
        oss << genres[i];
    }
    return oss.str();
}

bool Movie::hasGenre(std::string_view genre) const {
    if (genre.empty()) return false;
    std::string lowerGenre(genre);
    std::ranges::transform(lowerGenre, lowerGenre.begin(), ::tolower);
    
    for (const auto& g : genres) {
        std::string lowerG = g;
        std::ranges::transform(lowerG, lowerG.begin(), ::tolower);
        if (lowerG == lowerGenre) {
            return true;
        }
    }
    return false;
}

void Movie::print() const {
    std::cout << "ID: " << id << "\n";
    std::cout << "Title: " << title << "\n";
    std::cout << "Rating: " << rating << "\n";
    std::cout << "Year: " << year << "\n";
    std::cout << "Genres: " << getGenreString() << "\n";
    std::cout << "Director: " << director << "\n";
    std::cout << "Description: " << description << "\n";
    std::cout << "------------------------\n";
}

std::string Movie::toString() const {
    std::ostringstream oss;
    oss << id << "|" << title << "|" << rating << "|" << year << "|";
    
    for (size_t i = 0; i < genres.size(); ++i) {
        if (i > 0) oss << ";";
        oss << genres[i];
    }
    
    std::string cleanDescription = description;
    std::ranges::replace(cleanDescription, '\n', ' ');
    std::ranges::replace(cleanDescription, '\r', ' ');
    
    std::string cleanTitle = title;
    std::ranges::replace(cleanTitle, '\n', ' ');
    std::ranges::replace(cleanTitle, '\r', ' ');
    
    std::string cleanDirector = director;
    std::ranges::replace(cleanDirector, '\n', ' ');
    std::ranges::replace(cleanDirector, '\r', ' ');
    
    std::string cleanCountry = country;
    std::ranges::replace(cleanCountry, '\n', ' ');
    std::ranges::replace(cleanCountry, '\r', ' ');
    
    std::string cleanActors = actors;
    std::ranges::replace(cleanActors, '\n', ' ');
    std::ranges::replace(cleanActors, '\r', ' ');
    
    oss << "|" << cleanDirector << "|" << cleanDescription << "|" << posterPath
        << "|" << cleanCountry << "|" << cleanActors << "|" << duration;
    
    return oss.str();
}

Movie Movie::fromString(const std::string& data) {
    std::istringstream iss(data);
    std::string token;
    std::vector<std::string> tokens;

    while (std::getline(iss, token, '|')) {
        std::string trimmed = trim(token);
        tokens.push_back(trimmed);
    }

    Movie::Data movieData;
    
    if (tokens.size() < 7) {
        return Movie(movieData);
    }
    
    movieData.id = MovieParser::parseId(tokens);
    if (movieData.id == 0) {
        return Movie(movieData);
    }
    
    movieData.title = tokens.size() > 1 ? tokens[1] : "";
    movieData.rating = MovieParser::parseRating(tokens);
    movieData.year = MovieParser::parseYear(tokens);
    movieData.genres = MovieParser::parseGenres(tokens);
    movieData.director = tokens.size() > 5 ? tokens[5] : "";
    movieData.description = tokens.size() > 6 ? tokens[6] : "";
    movieData.posterPath = tokens.size() > 7 ? tokens[7] : "";
    movieData.country = tokens.size() > 8 ? tokens[8] : "";
    movieData.actors = tokens.size() > 9 ? tokens[9] : "";
    movieData.duration = MovieParser::parseDuration(tokens);
    
    return Movie(movieData);
}

bool Movie::operator==(const Movie& other) const {
    return id == other.id;
}

std::partial_ordering Movie::operator<=>(const Movie& other) const {
    if (auto cmp = rating <=> other.rating; cmp != 0) {
        return cmp;
    }
    return id <=> other.id;
}