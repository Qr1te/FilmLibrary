#ifndef BETA2_MOVIEMANAGER_H
#define BETA2_MOVIEMANAGER_H


#include "movie.h"
#include "exceptions/exceptions.h"
#include "MovieCollection.h"
#include <vector>
#include <string>
#include <memory>
#include <set>

class MovieManager {
private:
    std::vector<Movie> movies;
    std::vector<int> favoriteIds;
    std::string moviesFile;
    std::string favoritesFile;
    std::unique_ptr<CollectionManager> collectionManager;

    void loadMovies();
    void loadFavorites();
    void saveFavorites();
    void validateMovieId(int id) const;

public:
    MovieManager(const std::string& moviesFile = "movies.txt",
                 const std::string& favoritesFile = "favorites.txt");
    ~MovieManager();

    void searchByTitle(const std::string& title) const;
    void searchByGenre(const std::string& genre) const;
    void addToFavorites(int movieId);
    void removeFromFavorites(int movieId);
    void showFavorites() const;
    void showAllMovies() const;
    void sortByRating();
    void showTopRated(int count = 10) const;
    void getMovieDetails(int id) const;

    void createSampleMoviesFile();
    void reloadMovies();
    size_t getMoviesCount() const;
    size_t getFavoritesCount() const;

    const std::vector<Movie>& getAllMovies() const;
    std::vector<Movie> getFavoriteMovies() const;
    bool isFavorite(int id) const;
    std::vector<Movie> searchByTitleResults(const std::string& title) const;
    std::vector<Movie> searchByGenreResults(const std::string& genre) const;
    std::vector<Movie> topRatedResults(int count = 10) const;
    const Movie* findMovieById(int id) const;
    
    std::set<std::string> getAllGenres() const;
    
    CollectionManager* getCollectionManager();
    const CollectionManager* getCollectionManager() const;
    MovieCollection* createCollection(const std::string& name);
    std::vector<std::string> getAllCollectionNames() const;
    
    // API methods
    void addMovieToFile(const Movie& movie);
    void saveMovies();
    void removeMovie(int movieId);
};


#endif
