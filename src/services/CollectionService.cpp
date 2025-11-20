#include "services/CollectionService.h"
#include "exceptions/DuplicateCollectionException.h"
#include "exceptions/CollectionNotFoundException.h"
#include <algorithm>
#include <QString>

CollectionService::CollectionService(MovieService* movieService, const std::string& dir)
    : repository(dir), movieService(movieService) {
    reload();
}

void CollectionService::reload() {
    collections.clear();
    
    std::vector<std::string> names = repository.getAllCollectionNames();
    for (const auto& name : names) {
        // Additional check: filter out obviously garbled names
        QString qName = QString::fromUtf8(name.c_str(), name.length());
        QString upperName = qName.toUpper();
        // More aggressive check for garbled patterns
        bool isGarbled = (upperName.contains("P ") && upperName.contains("B")) ||
                        upperName.contains("P B--") || upperName.contains("P B P") ||
                        upperName.startsWith("P B") || upperName == "P B--" ||
                        (qName.contains("--") && qName.length() <= 6) ||
                        qName.contains("Р ") || qName.contains("вЂ") ||
                        (qName.length() <= 5 && qName.contains("P") && qName.contains("B"));
        
        if (!isGarbled) {
            auto collection = std::make_unique<MovieCollection>(name, &movieService->getAllMovies());
            collection->load();
            collections[name] = std::move(collection);
        }
    }
}

void CollectionService::saveAll() const {
    for (const auto& [name, collection] : collections) {
        // Save collection - this will update the name in file if it was corrected
        repository.saveCollection(*collection);
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
    for (const auto& [name, collection] : collections) {
        names.push_back(name);
    }
    return names;
}

void CollectionService::updateMoviesReference() const {
    for (const auto& [name, collection] : collections) {
        collection->setAllMoviesRef(&movieService->getAllMovies());
    }
}

