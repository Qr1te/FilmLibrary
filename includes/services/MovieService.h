#ifndef BETA2_MOVIESERVICE_H
#define BETA2_MOVIESERVICE_H

#include "../models/movie.h"
#include "../repositories/MovieRepository.h"
#include "../exceptions/exceptions.h"
#include <vector>
#include <string>
#include <set>

class MovieService {
private:
    MovieRepository repository;
    std::vector<Movie> movies;
    
public:
    explicit MovieService(const std::string& moviesFile = "movies.txt");
    
    void reload();
    void save() const;
    
    const std::vector<Movie>& getAllMovies() const;
    const Movie* findById(int id) const;
    std::vector<Movie> searchByTitle(const std::string& title) const;
    std::vector<Movie> searchByGenre(const std::string& genre) const;
    std::vector<Movie> topRated(int count = 10) const;
    std::set<std::string, std::less<>> getAllGenres() const;
    
    void addMovie(const Movie& movie);
    void removeMovie(int movieId);
    
    size_t getCount() const;
    bool exists(int id) const;
    
    void validateMovieId(int id) const;
};

#endif // BETA2_MOVIESERVICE_H

