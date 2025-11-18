#include "../../includes/repositories/CollectionRepository.h"
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <ranges>

CollectionRepository::CollectionRepository(const std::string& dir)
    : collectionsDirectory(dir) {
    try {
        std::filesystem::create_directories(collectionsDirectory);
    } catch (const std::filesystem::filesystem_error&) {
        // Игнорируем ошибки создания директории
    }
}

void CollectionRepository::saveCollection(const MovieCollection& collection) const {
    collection.save();
}

void CollectionRepository::loadCollection(const std::string& name, MovieCollection& collection) const {
    collection.load();
}

std::vector<std::string> CollectionRepository::getAllCollectionNames() const {
    std::vector<std::string> names;
    
    try {
        if (!std::filesystem::exists(collectionsDirectory)) {
            return names;
        }
        
        for (const auto& entry : std::filesystem::directory_iterator(collectionsDirectory)) {
            if (entry.is_regular_file() && entry.path().extension() == ".txt") {
                std::string filename = entry.path().stem().string();
                std::ranges::replace(filename, '_', ' ');
                names.push_back(filename);
            }
        }
    } catch (const std::filesystem::filesystem_error&) {
        // Игнорируем ошибки файловой системы
    }
    
    return names;
}

void CollectionRepository::deleteCollection(const std::string& name) const {
    std::string safeName = name;
    std::ranges::replace(safeName, ' ', '_');
    std::transform(safeName.begin(), safeName.end(), safeName.begin(), ::tolower);
    std::string filename = collectionsDirectory + safeName + ".txt";
    
    if (std::filesystem::exists(filename)) {
        std::filesystem::remove(filename);
    }
}

bool CollectionRepository::collectionExists(const std::string& name) const {
    std::string safeName = name;
    std::ranges::replace(safeName, ' ', '_');
    std::transform(safeName.begin(), safeName.end(), safeName.begin(), ::tolower);
    std::string filename = collectionsDirectory + safeName + ".txt";
    
    return std::filesystem::exists(filename);
}

