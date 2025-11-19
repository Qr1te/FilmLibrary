#include "../../includes/services/MovieService.h"
#include <algorithm>
#include <ranges>
#include <cctype>

MovieService::MovieService(const std::string& moviesFile)
    : repository(moviesFile) {
    reload();
}

void MovieService::reload() {
    movies = repository.loadAll();
}

void MovieService::save() const {
    repository.saveAll(movies);
}

const std::vector<Movie>& MovieService::getAllMovies() const {
    return movies;
}

const Movie* MovieService::findById(int id) const {
    return repository.findById(id, movies);
}

std::vector<Movie> MovieService::searchByTitle(const std::string& title) const {
    if (title.empty()) {
        throw InvalidInputException("Title cannot be empty");
    }
    
    std::string searchTitle = title;
    std::ranges::transform(searchTitle, searchTitle.begin(), ::tolower);
    
    std::vector<Movie> results;
    for (const auto& movie : movies) {
        std::string movieTitle = movie.getTitle();
        std::ranges::transform(movieTitle, movieTitle.begin(), ::tolower);
        if (movieTitle.contains(searchTitle)) {
            results.push_back(movie);
        }
    }
    
    return results;
}

std::vector<Movie> MovieService::searchByGenre(const std::string& genre) const {
    if (genre.empty()) {
        throw InvalidInputException("Genre cannot be empty");
    }
    
    std::string searchGenre = genre;
    std::ranges::transform(searchGenre, searchGenre.begin(), ::tolower);
    
    std::vector<Movie> results;
    for (const auto& movie : movies) {
        if (movie.hasGenre(searchGenre)) {
            results.push_back(movie);
        }
    }
    
    return results;
}

std::vector<Movie> MovieService::topRated(int count) const {
    if (movies.empty()) {
        throw MovieException("No movies available");
    }
    
    if (count <= 0) {
        throw InvalidInputException("Count must be positive");
    }
    
    std::vector<Movie> sortedMovies = movies;
    std::ranges::sort(sortedMovies,
              [](const Movie& a, const Movie& b) { return a.getRating() > b.getRating(); });
    
    if (count < static_cast<int>(sortedMovies.size())) {
        sortedMovies = std::vector<Movie>(sortedMovies.begin(), sortedMovies.begin() + count);
    }
    
    return sortedMovies;
}

std::set<std::string, std::less<>> MovieService::getAllGenres() const {
    std::set<std::string, std::less<>> genres;
    for (const auto& movie : movies) {
        const auto& movieGenres = movie.getGenres();
        for (const auto& genre : movieGenres) {
            genres.insert(genre);
        }
    }
    return genres;
}

void MovieService::addMovie(const Movie& movie) {
    repository.addMovie(movie, movies);
    save();
}

void MovieService::removeMovie(int movieId) {
    validateMovieId(movieId);
    repository.removeMovie(movieId, movies);
    save();
}

size_t MovieService::getCount() const {
    return movies.size();
}

bool MovieService::exists(int id) const {
    return repository.exists(id, movies);
}

void MovieService::validateMovieId(int id) const {
    if (!exists(id)) {
        throw MovieNotFoundException(id);
    }
}

