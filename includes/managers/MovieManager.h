#ifndef BETA2_MOVIEMANAGER_H
#define BETA2_MOVIEMANAGER_H

#include "models/movie.h"
#include "exceptions/exceptions.h"
#include "models/MovieCollection.h"
#include "services/MovieService.h"
#include "services/FavoriteService.h"
#include "services/CollectionService.h"
#include <vector>
#include <string>
#include <memory>
#include <set>

class MovieManager {
private:
    std::unique_ptr<MovieService> movieService;
    std::unique_ptr<FavoriteService> favoriteService;
    std::unique_ptr<CollectionService> collectionService;
    std::string moviesFile;
    std::string favoritesFile;
    
    mutable std::unique_ptr<CollectionManager> collectionManagerAdapter;
    
    void syncCollectionAdapter() const;
    void copyCollectionToAdapter(const std::string& name, CollectionManager* adapter) const;

public:
    explicit MovieManager(std::string  moviesFile = "movies.txt",
                 std::string  favoritesFile = "favorites.txt");
    ~MovieManager();

    void searchByTitle(const std::string& title) const;
    void searchByGenre(const std::string& genre) const;
    void addToFavorites(int movieId);
    void removeFromFavorites(int movieId);
    void showFavorites() const;
    void showAllMovies() const;
    void sortByRating() const;
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
    
    std::set<std::string, std::less<>> getAllGenres() const;
    
    CollectionManager* getCollectionManager();
    const CollectionManager* getCollectionManager() const;
    MovieCollection* createCollection(const std::string& name);
    std::vector<std::string> getAllCollectionNames() const;
    
    void addMovieToFile(const Movie& movie);
    void saveMovies() const;
    void removeMovie(int movieId);
    
    MovieService* getMovieService();
    FavoriteService* getFavoriteService();
    CollectionService* getCollectionService();
};

#endif
