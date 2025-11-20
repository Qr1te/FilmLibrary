#include "managers/MovieManager.h"
#include "exceptions/FileNotFoundException.h"
#include "exceptions/DuplicateCollectionException.h"
#include <iostream>
#include <algorithm>
#include <cctype>
#include <fstream>

MovieManager::MovieManager(std::string moviesFile, std::string favoritesFile)
    : moviesFile(std::move(moviesFile)), favoritesFile(std::move(favoritesFile)) {
    try {
        movieService = std::make_unique<MovieService>(this->moviesFile);
        favoriteService = std::make_unique<FavoriteService>(movieService.get(), this->favoritesFile);
        collectionService = std::make_unique<CollectionService>(movieService.get());
    } catch (const FileNotFoundException& e) {
        std::cout << "Warning: " << e.what() << "\n";
        std::cout << "Use option 8 to create sample movies file.\n";
        movieService = std::make_unique<MovieService>(this->moviesFile);
        favoriteService = std::make_unique<FavoriteService>(movieService.get(), this->favoritesFile);
        collectionService = std::make_unique<CollectionService>(movieService.get());
    }
}

MovieManager::~MovieManager() {
    if (collectionService) {
        collectionService->saveAll();
    }
}

void MovieManager::addToFavorites(int movieId) {
    favoriteService->addFavorite(movieId);
    std::cout << "Movie added to favorites!\n";
}

void MovieManager::removeFromFavorites(int movieId) {
    favoriteService->removeFavorite(movieId);
    std::cout << "Movie removed from favorites!\n";
}

std::vector<Movie> MovieManager::sortByRatingResults() const {
    std::vector<Movie> movies = movieService->getAllMovies();
    if (movies.empty()) {
        throw MovieException("No movies to sort");
    }
    
    std::ranges::sort(movies, [](const Movie& a, const Movie& b) {
        return a.getRating() > b.getRating();
    });
    
    return movies;
}

void MovieManager::reloadMovies() {
    movieService->reload();
    favoriteService->reload();
    collectionService->reload();
    collectionService->updateMoviesReference();
}

size_t MovieManager::getMoviesCount() const {
    return movieService->getCount();
}

size_t MovieManager::getFavoritesCount() const {
    return favoriteService->getCount();
}

const std::vector<Movie>& MovieManager::getAllMovies() const {
    return movieService->getAllMovies();
}

std::vector<Movie> MovieManager::getFavoriteMovies() const {
    return favoriteService->getFavoriteMovies();
}

bool MovieManager::isFavorite(int id) const {
    return favoriteService->isFavorite(id);
}

std::vector<Movie> MovieManager::searchByTitleResults(const std::string& title) const {
    return movieService->searchByTitle(title);
}

std::vector<Movie> MovieManager::searchByGenreResults(const std::string& genre) const {
    return movieService->searchByGenre(genre);
}

std::vector<Movie> MovieManager::topRatedResults(int count) const {
    return movieService->topRated(count);
}

const Movie* MovieManager::findMovieById(int id) const {
    return movieService->findById(id);
}

std::set<std::string, std::less<>> MovieManager::getAllGenres() const {
    return movieService->getAllGenres();
}

MovieCollection* MovieManager::createCollection(const std::string& name) {
    return collectionService->createCollection(name);
}

std::vector<std::string> MovieManager::getAllCollectionNames() const {
    return collectionService->getAllCollectionNames();
}

void MovieManager::addMovieToFile(const Movie& movie) {
    movieService->addMovie(movie);
    collectionService->updateMoviesReference();
}

void MovieManager::saveMovies() const {
    movieService->save();
}

void MovieManager::removeMovie(int movieId) {
    if (favoriteService->isFavorite(movieId)) {
        try {
            favoriteService->removeFavorite(movieId);
        } catch (const MovieNotFoundException& e) {
            (void)e;
        }
    }
    
    auto collectionNames = collectionService->getAllCollectionNames();
    for (const auto& name : collectionNames) {
        MovieCollection* collection = collectionService->getCollection(name);
            if (collection && collection->containsMovie(movieId)) {
                collection->removeMovie(movieId);
        }
    }
    
    movieService->removeMovie(movieId);
    collectionService->updateMoviesReference();
}

void MovieManager::copyCollectionToAdapter(const std::string& name, CollectionManager* adapter) const {
    const MovieCollection* coll = collectionService->getCollection(name);
    if (!coll) {
        return;
    }
    
    // Check if collection already exists in adapter
    MovieCollection* adapterColl = adapter->getCollection(name);
    if (!adapterColl) {
        // Collection doesn't exist, create it
        try {
            adapter->createCollection(name);
            adapterColl = adapter->getCollection(name);
        } catch (const DuplicateCollectionException&) {
            // Collection already exists, just get it
            adapterColl = adapter->getCollection(name);
        }
    }
    
    if (!adapterColl) {
        return;
    }
    
    // Clear existing movies and add all from source collection
    adapterColl->clear();
    auto movies = coll->getMovies();
    for (const auto& movie : movies) {
        adapterColl->addMovie(movie);
    }
}

void MovieManager::syncCollectionAdapter() const {
    if (!collectionManagerAdapter) {
        collectionManagerAdapter = std::make_unique<CollectionManager>(&movieService->getAllMovies());
    }
    
    auto names = collectionService->getAllCollectionNames();
    for (const auto& name : names) {
        try {
            copyCollectionToAdapter(name, collectionManagerAdapter.get());
        } catch (const CollectionNotFoundException& e) {
            std::cerr << "Warning: Collection not found during adapter sync: " << e.what() << std::endl;
        } catch (const DuplicateCollectionException& e) {
            // Collection already exists in adapter, this is normal - copyCollectionToAdapter handles it
            // Log for debugging but don't treat as error
            (void)e; // Suppress unused variable warning
        } catch (const MovieException& e) {
            std::cerr << "Warning: Error syncing collection adapter: " << e.what() << std::endl;
        }
    }
}

CollectionManager* MovieManager::getCollectionManager() {
    syncCollectionAdapter();
    return collectionManagerAdapter.get();
}

const CollectionManager* MovieManager::getCollectionManager() const {
    syncCollectionAdapter();
    return collectionManagerAdapter.get();
}

MovieService* MovieManager::getMovieService() {
    return movieService.get();
}

FavoriteService* MovieManager::getFavoriteService() {
    return favoriteService.get();
}

CollectionService* MovieManager::getCollectionService() {
    return collectionService.get();
}
