#include "../../includes/repositories/FavoriteRepository.h"
#include "../../includes/exceptions/DuplicateFavoriteException.h"
#include "../../includes/exceptions/MovieNotFoundException.h"
#include "../../includes/exceptions/FileNotFoundException.h"
#include <fstream>
#include <algorithm>
#include <ranges>

FavoriteRepository::FavoriteRepository(const std::string& favoritesFile)
    : favoritesFile(favoritesFile) {
}

std::vector<int> FavoriteRepository::loadAll() const {
    std::vector<int> favoriteIds;
    std::ifstream file(favoritesFile);

    if (!file.is_open()) {
        return favoriteIds;
    }

    int id;
    while (file >> id) {
        if (file.fail()) {
            break;
        }
        favoriteIds.push_back(id);
    }
    
    return favoriteIds;
}

void FavoriteRepository::saveAll(const std::vector<int>& favoriteIds) const {
    std::ofstream file(favoritesFile);
    if (!file.is_open()) {
        throw FileNotFoundException(favoritesFile);
    }

    for (int id : favoriteIds) {
        file << id << "\n";
    }
    
    file.close();
}

void FavoriteRepository::addFavorite(int movieId, std::vector<int>& favoriteIds) const {
    if (auto it = std::ranges::find(favoriteIds, movieId); it != favoriteIds.end()) {
        throw DuplicateFavoriteException("Фильм с ID " + std::to_string(movieId));
    }

    favoriteIds.insert(favoriteIds.begin(), movieId);
}

void FavoriteRepository::removeFavorite(int movieId, std::vector<int>& favoriteIds) const {
    auto it = std::ranges::find(favoriteIds, movieId);
    if (it == favoriteIds.end()) {
        throw MovieNotFoundException(movieId);
    }

    favoriteIds.erase(it);
}

bool FavoriteRepository::isFavorite(int movieId, const std::vector<int>& favoriteIds) const {
    return std::ranges::find(favoriteIds, movieId) != favoriteIds.end();
}

