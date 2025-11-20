#ifndef BETA2_MOVIEREPOSITORY_H
#define BETA2_MOVIEREPOSITORY_H

#include "../models/movie.h"
#include "../exceptions/exceptions.h"
#include <vector>
#include <string>

class MovieRepository {
private:
    std::string moviesFile;
    
    bool processFullLine(const std::string& fullLine, int lineNumber, std::vector<Movie>& movies) const;
    void processRemainingLine(const std::string& fullLine, std::vector<Movie>& movies) const;
    void setDefaultPosterPath(Movie& movie) const;
    
public:
    explicit MovieRepository(const std::string& moviesFile = "movies.txt");
    
    std::vector<Movie> loadAll() const;
    void saveAll(const std::vector<Movie>& movies) const;
    void addMovie(const Movie& movie, std::vector<Movie>& movies) const;
    void removeMovie(int movieId, std::vector<Movie>& movies) const;
    const Movie* findById(int id, const std::vector<Movie>& movies) const;
    bool exists(int id, const std::vector<Movie>& movies) const;
};

#endif // BETA2_MOVIEREPOSITORY_H





