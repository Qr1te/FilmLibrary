#include "../includes/movie.h"

#include <iostream>
#include <sstream>
#include <algorithm>
#include <cctype>

static std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}

Movie::Movie(int id, const std::string& title, double rating, int year,
             const std::vector<std::string>& genres, const std::string& director,
             const std::string& description, const std::string& posterPath,
             const std::string& country, const std::string& actors, int duration)
    : id(id), title(title), rating(rating), year(year),
      genres(genres), director(director), description(description),
      posterPath(posterPath), country(country), actors(actors), duration(duration) {}

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

void Movie::setId(int id) {
    this->id = id;
}

void Movie::setTitle(const std::string& title) {
    this->title = title;
}

void Movie::setRating(double rating) {
    this->rating = rating;
}

void Movie::setYear(int year) {
    this->year = year;
}

void Movie::setGenres(const std::vector<std::string>& genres) {
    this->genres = genres;
}

void Movie::setDirector(const std::string& director) {
    this->director = director;
}

void Movie::setDescription(const std::string& description) {
    this->description = description;
}

void Movie::setPosterPath(const std::string& path) {
    this->posterPath = path;
}

void Movie::setCountry(const std::string& country) {
    this->country = country;
}

void Movie::setActors(const std::string& actors) {
    this->actors = actors;
}

void Movie::setDuration(int duration) {
    this->duration = duration;
}

void Movie::addGenre(const std::string& genre) {
    if (!genre.empty() && std::find(genres.begin(), genres.end(), genre) == genres.end()) {
        genres.push_back(genre);
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

bool Movie::hasGenre(const std::string& genre) const {
    if (genre.empty()) return false;
    std::string lowerGenre = genre;
    std::transform(lowerGenre.begin(), lowerGenre.end(), lowerGenre.begin(), ::tolower);
    
    for (const auto& g : genres) {
        std::string lowerG = g;
        std::transform(lowerG.begin(), lowerG.end(), lowerG.begin(), ::tolower);
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
    std::replace(cleanDescription.begin(), cleanDescription.end(), '\n', ' ');
    std::replace(cleanDescription.begin(), cleanDescription.end(), '\r', ' ');
    
    std::string cleanTitle = title;
    std::replace(cleanTitle.begin(), cleanTitle.end(), '\n', ' ');
    std::replace(cleanTitle.begin(), cleanTitle.end(), '\r', ' ');
    
    std::string cleanDirector = director;
    std::replace(cleanDirector.begin(), cleanDirector.end(), '\n', ' ');
    std::replace(cleanDirector.begin(), cleanDirector.end(), '\r', ' ');
    
    std::string cleanCountry = country;
    std::replace(cleanCountry.begin(), cleanCountry.end(), '\n', ' ');
    std::replace(cleanCountry.begin(), cleanCountry.end(), '\r', ' ');
    
    std::string cleanActors = actors;
    std::replace(cleanActors.begin(), cleanActors.end(), '\n', ' ');
    std::replace(cleanActors.begin(), cleanActors.end(), '\r', ' ');
    
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

    if (tokens.size() >= 7) {
        int id = 0;
        double rating = 0.0;
        int year = 0;
        int duration = 0;
        
        try {
            if (!tokens[0].empty()) {
                id = std::stoi(tokens[0]);
            }
        } catch (...) {
            return Movie(0, "", 0.0, 0, std::vector<std::string>(), "", "", "", "", "", 0);
        }
        
        std::string title = tokens.size() > 1 ? tokens[1] : "";
        
        try {
            if (tokens.size() > 2 && !tokens[2].empty()) {
                rating = std::stod(tokens[2]);
            }
        } catch (...) {
            rating = 0.0;
        }
        
        try {
            if (tokens.size() > 3 && !tokens[3].empty()) {
                year = std::stoi(tokens[3]);
            }
        } catch (...) {
            year = 0;
        }
        
        std::vector<std::string> genres;
        std::string genreStr = tokens.size() > 4 ? tokens[4] : "";
        if (!genreStr.empty()) {
            std::istringstream genreStream(genreStr);
            std::string genre;
            if (genreStr.find(';') != std::string::npos) {
                while (std::getline(genreStream, genre, ';')) {
                    if (!genre.empty()) {
                        genres.push_back(genre);
                    }
                }
            } else {
                genres.push_back(genreStr);
            }
        }
        
        std::string director = tokens.size() > 5 ? tokens[5] : "";
        std::string description = tokens.size() > 6 ? tokens[6] : "";
        std::string posterPath = tokens.size() > 7 ? tokens[7] : "";
        std::string country = tokens.size() > 8 ? tokens[8] : "";
        std::string actors = tokens.size() > 9 ? tokens[9] : "";
        
        if (tokens.size() > 10) {
            if (!tokens[10].empty()) {
                try {
                    duration = std::stoi(tokens[10]);
                } catch (...) {
                    duration = 0;
                }
            } else {
                duration = 0;
            }
        } else {
            duration = 0;
        }
        
        return Movie(id, title, rating, year, genres, director, description,
                    posterPath, country, actors, duration);
    }

    return Movie(0, "", 0.0, 0, std::vector<std::string>(), "", "", "", "", "", 0);
}

bool Movie::operator==(const Movie& other) const {
    return id == other.id;
}

bool Movie::operator!=(const Movie& other) const {
    return !(*this == other);
}

bool Movie::operator<(const Movie& other) const {
    return rating < other.rating || (rating == other.rating && id < other.id);
}

bool Movie::operator>(const Movie& other) const {
    return other < *this;
}

std::ostream& operator<<(std::ostream& os, const Movie& movie) {
    os << movie.toString();
    return os;
}

std::istream& operator>>(std::istream& is, Movie& movie) {
    std::string line;
    if (std::getline(is, line)) {
        movie = Movie::fromString(line);
    }
    return is;
}