#include "../../includes/repositories/MovieRepository.h"
#include "../../includes/exceptions/InvalidMovieDataException.h"
#include <fstream>
#include <algorithm>
#include <ranges>
#include <stdexcept>
#include <iostream>

MovieRepository::MovieRepository(const std::string& moviesFile)
    : moviesFile(moviesFile) {
}

std::vector<Movie> MovieRepository::loadAll() const {
    std::vector<Movie> movies;
    std::ifstream file(moviesFile);

    if (!file.is_open()) {
        std::ofstream createFile(moviesFile);
        createFile.close();
        return movies;
    }

    std::string line;
    std::string fullLine;
    int lineNumber = 0;

    while (std::getline(file, line)) {
        lineNumber++;
        if (line.empty()) {
            if (!fullLine.empty()) {
                fullLine += " ";
            }
            continue;
        }
        
        if (fullLine.empty()) {
            fullLine = line;
        } else {
            fullLine += " " + line;
        }
        
        int pipeCount = std::ranges::count(fullLine, '|');
        if (pipeCount >= 10) {
            try {
                Movie movie = Movie::fromString(fullLine);
                if (movie.getId() != 0) {
                    if (movie.getPosterPath().empty() && movie.getId() > 0) {
                        std::string possiblePath = "posters/" + std::to_string(movie.getId()) + ".jpg";
                        movie.setPosterPath(possiblePath);
                    }
                    movies.push_back(movie);
                    fullLine.clear();
                } else {
                    throw InvalidMovieDataException("Line " + std::to_string(lineNumber));
                }
            } catch (const InvalidMovieDataException& e) {
                std::cerr << "Warning: Invalid movie data on line " << lineNumber << ": " << e.what() << std::endl;
                fullLine.clear();
            } catch (const std::invalid_argument& e) {
                std::cerr << "Warning: Invalid argument on line " << lineNumber << ": " << e.what() << std::endl;
                fullLine.clear();
            }
        }
    }
    
    if (!fullLine.empty()) {
        try {
            Movie movie = Movie::fromString(fullLine);
            if (movie.getId() != 0) {
                if (movie.getPosterPath().empty() && movie.getId() > 0) {
                    std::string possiblePath = "posters/" + std::to_string(movie.getId()) + ".jpg";
                    movie.setPosterPath(possiblePath);
                }
                movies.push_back(movie);
            }
        } catch (const InvalidMovieDataException& e) {
            std::cerr << "Warning: Failed to parse movie data: " << e.what() << std::endl;
        } catch (const std::invalid_argument& e) {
            std::cerr << "Warning: Parsing error: " << e.what() << std::endl;
        }
    }
    
    return movies;
}

void MovieRepository::saveAll(const std::vector<Movie>& movies) const {
    std::ofstream file(moviesFile, std::ios::out);
    if (!file.is_open()) {
        throw FileNotFoundException(moviesFile);
    }
    
    for (const auto& movie : movies) {
        std::string line = movie.toString();
        file << line << "\n";
    }
    
    file.close();
}

void MovieRepository::addMovie(const Movie& movie, std::vector<Movie>& movies) const {
    for (const auto& m : movies) {
        if (m.getTitle() == movie.getTitle() && m.getYear() == movie.getYear() && movie.getYear() != 0) {
            throw DuplicateMovieException(m.getTitle(), m.getYear());
        }
    }
    
    if (movie.getId() != 0) {
        for (const auto& m : movies) {
            if (m.getId() == movie.getId()) {
                throw DuplicateMovieException(m.getTitle(), m.getYear());
            }
        }
    }
    
    int newId = movie.getId();
    if (newId == 0) {
        newId = 1;
        for (const auto& m : movies) {
            if (m.getId() >= newId) {
                newId = m.getId() + 1;
            }
        }
    }
    
    Movie movieWithId = movie;
    movieWithId.setId(newId);
    
    movies.insert(movies.begin(), movieWithId);
}

void MovieRepository::removeMovie(int movieId, std::vector<Movie>& movies) const {
    auto it = std::ranges::find_if(movies,
                          [movieId](const Movie& m) { return m.getId() == movieId; });
    
    if (it == movies.end()) {
        throw MovieNotFoundException(movieId);
    }
    
    movies.erase(it);
}

const Movie* MovieRepository::findById(int id, const std::vector<Movie>& movies) const {
    auto it = std::ranges::find_if(movies, [id](const Movie& m) { return m.getId() == id; });
    if (it == movies.end()) {
        return nullptr;
    }
    return &(*it);
}

bool MovieRepository::exists(int id, const std::vector<Movie>& movies) const {
    return findById(id, movies) != nullptr;
}

