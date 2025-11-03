#include "../includes/movie.h"

#include <iostream>
#include <sstream>
#include <algorithm>

// Основной конструктор с множественными жанрами
Movie::Movie(int id, const std::string& title, double rating, int year,
             const std::vector<std::string>& genres, const std::string& director,
             const std::string& description, const std::string& posterPath,
             const std::string& country, const std::string& actors, int duration)
    : id(id), title(title), rating(rating), year(year),
      genres(genres), director(director), description(description),
      posterPath(posterPath), country(country), actors(actors), duration(duration) {}

// Конструктор для обратной совместимости (один жанр)
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
    
    // Сохраняем жанры через точку с запятой
    for (size_t i = 0; i < genres.size(); ++i) {
        if (i > 0) oss << ";";
        oss << genres[i];
    }
    
    oss << "|" << director << "|" << description << "|" << posterPath
        << "|" << country << "|" << actors << "|" << duration;
    return oss.str();
}

Movie Movie::fromString(const std::string& data) {
    std::istringstream iss(data);
    std::string token;
    std::vector<std::string> tokens;

    while (std::getline(iss, token, '|')) {
        tokens.push_back(token);
    }

    if (tokens.size() >= 7) {
        int id = std::stoi(tokens[0]);
        std::string title = tokens[1];
        double rating = std::stod(tokens[2]);
        int year = std::stoi(tokens[3]);
        
        std::vector<std::string> genres;
        std::string genreStr = tokens[4];
        if (!genreStr.empty()) {
            std::istringstream genreStream(genreStr);
            std::string genre;
            // Проверяем, есть ли точка с запятой (новый формат)
            if (genreStr.find(';') != std::string::npos) {
                while (std::getline(genreStream, genre, ';')) {
                    if (!genre.empty()) {
                        genres.push_back(genre);
                    }
                }
            } else {
                // Старый формат - один жанр
                genres.push_back(genreStr);
            }
        }
        
        std::string director = tokens.size() > 5 ? tokens[5] : "";
        std::string description = tokens.size() > 6 ? tokens[6] : "";
        std::string posterPath = tokens.size() > 7 ? tokens[7] : "";
        std::string country = tokens.size() > 8 ? tokens[8] : "";
        std::string actors = tokens.size() > 9 ? tokens[9] : "";
        int duration = tokens.size() > 10 ? std::stoi(tokens[10]) : 0;
        
        return Movie(id, title, rating, year, genres, director, description,
                    posterPath, country, actors, duration);
    }

    return Movie(0, "", 0.0, 0, std::vector<std::string>(), "", "", "", "", "", 0);
}

// Перегрузка операций
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

// Дружественная функция для вывода
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