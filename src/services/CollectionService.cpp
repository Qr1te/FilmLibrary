#include "../../includes/services/CollectionService.h"
#include "../../includes/exceptions/DuplicateCollectionException.h"
#include "../../includes/exceptions/CollectionNotFoundException.h"
#include <algorithm>

CollectionService::CollectionService(MovieService* movieService, const std::string& dir)
    : repository(dir), movieService(movieService) {
    reload();
}

void CollectionService::reload() {
    collections.clear();
    
    std::vector<std::string> names = repository.getAllCollectionNames();
    for (const auto& name : names) {
        auto collection = std::make_unique<MovieCollection>(name, &movieService->getAllMovies());
        collection->load();
        collections[name] = std::move(collection);
    }
}

void CollectionService::saveAll() {
    for (const auto& pair : collections) {
        repository.saveCollection(*pair.second);
    }
}

MovieCollection* CollectionService::createCollection(const std::string& name) {
    if (collections.contains(name)) {
        throw DuplicateCollectionException(name);
    }
    
    auto collection = std::make_unique<MovieCollection>(name, &movieService->getAllMovies());
    MovieCollection* ptr = collection.get();
    collections[name] = std::move(collection);
    
    saveAll();
    return ptr;
}

MovieCollection* CollectionService::getCollection(const std::string& name) {
    auto it = collections.find(name);
    if (it == collections.end()) {
        throw CollectionNotFoundException(name);
    }
    return it->second.get();
}

const MovieCollection* CollectionService::getCollection(const std::string& name) const {
    auto it = collections.find(name);
    if (it == collections.end()) {
        throw CollectionNotFoundException(name);
    }
    return it->second.get();
}

void CollectionService::deleteCollection(const std::string& name) {
    auto it = collections.find(name);
    if (it == collections.end()) {
        throw CollectionNotFoundException(name);
    }
    
    collections.erase(it);
    repository.deleteCollection(name);
}

std::vector<std::string> CollectionService::getAllCollectionNames() const {
    std::vector<std::string> names;
    for (const auto& pair : collections) {
        names.push_back(pair.first);
    }
    return names;
}

void CollectionService::updateMoviesReference() {
    for (auto& pair : collections) {
        pair.second->setAllMoviesRef(&movieService->getAllMovies());
    }
}

