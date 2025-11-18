#ifndef BETA2_MOVIEREPOSITORY_H
#define BETA2_MOVIEREPOSITORY_H

#include "../movie.h"
#include "../exceptions/exceptions.h"
#include <vector>
#include <string>

class MovieRepository {
private:
    std::string moviesFile;
    
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

