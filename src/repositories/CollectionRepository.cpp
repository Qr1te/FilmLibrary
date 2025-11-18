#include "../../includes/repositories/CollectionRepository.h"
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <ranges>
#include <iostream>

CollectionRepository::CollectionRepository(const std::string& dir)
    : collectionsDirectory(dir) {
    try {
        std::filesystem::create_directories(collectionsDirectory);
    } catch (const std::filesystem::filesystem_error& e) {
        // Директория может уже существовать, это не критично
        if (!std::filesystem::exists(collectionsDirectory)) {
            std::cerr << "Warning: Failed to create collections directory: " << e.what() << std::endl;
        }
    }
}

void CollectionRepository::saveCollection(const MovieCollection& collection) const {
    collection.save();
}

void CollectionRepository::loadCollection(std::string_view name, MovieCollection& collection) const {
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
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Warning: Filesystem error while getting collection names: " << e.what() << std::endl;
    }
    
    return names;
}

void CollectionRepository::deleteCollection(const std::string& name) const {
    std::string safeName = name;
    std::ranges::replace(safeName, ' ', '_');
    std::ranges::transform(safeName, safeName.begin(), ::tolower);
    std::string filename = collectionsDirectory + safeName + ".txt";
    
    if (std::filesystem::exists(filename)) {
        std::filesystem::remove(filename);
    }
}

bool CollectionRepository::collectionExists(const std::string& name) const {
    std::string safeName = name;
    std::ranges::replace(safeName, ' ', '_');
    std::ranges::transform(safeName, safeName.begin(), ::tolower);
    std::string filename = collectionsDirectory + safeName + ".txt";
    
    return std::filesystem::exists(filename);
}

