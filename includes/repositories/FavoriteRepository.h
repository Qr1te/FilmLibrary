#ifndef BETA2_FAVORITEREPOSITORY_H
#define BETA2_FAVORITEREPOSITORY_H

#include <vector>
#include <string>

class FavoriteRepository {
private:
    std::string favoritesFile;
    
public:
    explicit FavoriteRepository(const std::string& favoritesFile = "favorites.txt");
    
    std::vector<int> loadAll() const;
    void saveAll(const std::vector<int>& favoriteIds) const;
    void addFavorite(int movieId, std::vector<int>& favoriteIds) const;
    void removeFavorite(int movieId, std::vector<int>& favoriteIds) const;
    bool isFavorite(int movieId, const std::vector<int>& favoriteIds) const;
};

#endif // BETA2_FAVORITEREPOSITORY_H





