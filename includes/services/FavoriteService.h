#ifndef BETA2_FAVORITESERVICE_H
#define BETA2_FAVORITESERVICE_H

#include "../movie.h"
#include "../repositories/FavoriteRepository.h"
#include "../services/MovieService.h"
#include <vector>

class FavoriteService {
private:
    FavoriteRepository repository;
    MovieService* movieService;
    std::vector<int> favoriteIds;
    
public:
    explicit FavoriteService(MovieService* movieService, const std::string& favoritesFile = "favorites.txt");
    
    void reload();
    void save();
    
    std::vector<Movie> getFavoriteMovies() const;
    bool isFavorite(int movieId) const;
    void addFavorite(int movieId);
    void removeFavorite(int movieId);
    
    size_t getCount() const;
};

#endif // BETA2_FAVORITESERVICE_H


