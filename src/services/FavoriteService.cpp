#include "services/FavoriteService.h"
#include "exceptions/MovieNotFoundException.h"
#include <algorithm>

FavoriteService::FavoriteService(MovieService* movieService, const std::string& favoritesFile)
    : repository(favoritesFile), movieService(movieService) {
    reload();
}

void FavoriteService::reload() {
    favoriteIds = repository.loadAll();
}

void FavoriteService::save() const {
    repository.saveAll(favoriteIds);
}

std::vector<Movie> FavoriteService::getFavoriteMovies() const {
    std::vector<Movie> favorites;
    for (int id : favoriteIds) {
        const Movie* movie = movieService->findById(id);
        if (movie) {
            favorites.push_back(*movie);
        }
    }
    return favorites;
}

bool FavoriteService::isFavorite(int movieId) const {
    return repository.isFavorite(movieId, favoriteIds);
}

void FavoriteService::addFavorite(int movieId) {
    if (!movieService->exists(movieId)) {
        throw MovieNotFoundException(movieId);
    }
    
    repository.addFavorite(movieId, favoriteIds);
    save();
}

void FavoriteService::removeFavorite(int movieId) {
    repository.removeFavorite(movieId, favoriteIds);
    save();
}

size_t FavoriteService::getCount() const {
    return favoriteIds.size();
}

