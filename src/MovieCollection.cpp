#include "../includes/MovieCollection.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>


MovieCollection::MovieCollection(const std::string& name, const std::vector<Movie>* allMovies)
    : collectionName(name), allMoviesRef(allMovies) {
    validateCollectionName(name);
    

    std::string safeName = name;
    std::replace(safeName.begin(), safeName.end(), ' ', '_');
    std::transform(safeName.begin(), safeName.end(), safeName.begin(), ::tolower);
    filename = "collections/" + safeName + ".txt";
    
    try {
        std::filesystem::create_directories("collections");
    } catch (const std::exception&) {
    }
}

void MovieCollection::validateCollectionName(const std::string& name) const {
    if (name.empty()) {
        throw InvalidInputException("Collection name cannot be empty");
    }
    
    std::string invalidChars = "<>:\"|?*\\/";
    for (char c : invalidChars) {
        if (name.find(c) != std::string::npos) {
            throw InvalidInputException("Collection name contains invalid characters");
        }
    }
}

void MovieCollection::validateMovieId(int id) const {
    if (!allMoviesRef) {
        throw MovieException("Movie reference not set");
    }
    
    auto it = std::find_if(allMoviesRef->begin(), allMoviesRef->end(),
                          [id](const Movie& m) { return m.getId() == id; });
    if (it == allMoviesRef->end()) {
        throw MovieNotFoundException(id);
    }
}

void MovieCollection::addMovie(const Movie& movie) {
    int id = movie.getId();
    validateMovieId(id);
    
    if (containsMovie(id)) {
        // Используем название фильма для исключения
        throw DuplicateFavoriteException(movie.getTitle());
    }
    
    movieIds.insert(movieIds.begin(), id);
    save();
}

void MovieCollection::removeMovie(int movieId) {
    auto it = std::find(movieIds.begin(), movieIds.end(), movieId);
    if (it == movieIds.end()) {
        throw MovieNotFoundException(movieId);
    }
    
    movieIds.erase(it);
    save();
}

bool MovieCollection::containsMovie(int movieId) const {
    return std::find(movieIds.begin(), movieIds.end(), movieId) != movieIds.end();
}

size_t MovieCollection::size() const {
    return movieIds.size();
}

std::vector<Movie> MovieCollection::getMovies() const {
    std::vector<Movie> result;
    
    if (!allMoviesRef) {
        return result;
    }
    
    for (int id : movieIds) {
        auto it = std::find_if(allMoviesRef->begin(), allMoviesRef->end(),
                              [id](const Movie& m) { return m.getId() == id; });
        if (it != allMoviesRef->end()) {
            result.push_back(*it);
        }
    }
    
    return result;
}

void MovieCollection::clear() {
    movieIds.clear();
    save();
}

std::string MovieCollection::getName() const {
    return collectionName;
}

std::string MovieCollection::getFilename() const {
    return filename;
}

void MovieCollection::setAllMoviesRef(const std::vector<Movie>* movies) {
    allMoviesRef = movies;
}

std::vector<int>::const_iterator MovieCollection::begin() const {
    return movieIds.begin();
}

std::vector<int>::const_iterator MovieCollection::end() const {
    return movieIds.end();
}

void MovieCollection::save() const {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        throw FileNotFoundException(filename);
    }
    
    std::string nameLine = collectionName + "\n";
    file.write(nameLine.c_str(), nameLine.size());
    
    for (int id : movieIds) {
        file << id << "\n";
    }
}

void MovieCollection::load() {
    movieIds.clear();
    
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        return;
    }
    
    std::string line;
    if (std::getline(file, line)) {
    }
    
    int id;
    while (file >> id) {
        if (file.fail()) {
            break;
        }
        if (allMoviesRef) {
            bool exists = std::any_of(allMoviesRef->begin(), allMoviesRef->end(),
                                     [id](const Movie& m) { return m.getId() == id; });
            if (exists) {
                movieIds.push_back(id);
            }
        } else {
            movieIds.push_back(id);
        }
    }
}


CollectionManager::CollectionManager(const std::vector<Movie>* allMovies, const std::string& dir)
    : allMoviesRef(allMovies), collectionsDirectory(dir) {
    try {
        std::filesystem::create_directories(collectionsDirectory);
    } catch (const std::exception&) {
    }
    loadAll();
}

MovieCollection* CollectionManager::createCollection(const std::string& name) {
    if (collections.find(name) != collections.end()) {
        throw DuplicateCollectionException(name);
    }
    
    auto collection = std::make_unique<MovieCollection>(name, allMoviesRef);
    MovieCollection* ptr = collection.get();
    collections[name] = std::move(collection);
    
    ptr->save();
    
    return ptr;
}

MovieCollection* CollectionManager::getCollection(const std::string& name) {
    auto it = collections.find(name);
    if (it == collections.end()) {
        return nullptr;
    }
    return it->second.get();
}

const MovieCollection* CollectionManager::getCollection(const std::string& name) const {
    auto it = collections.find(name);
    if (it == collections.end()) {
        return nullptr;
    }
    return it->second.get();
}

void CollectionManager::deleteCollection(const std::string& name) {
    auto it = collections.find(name);
    if (it == collections.end()) {
        throw CollectionNotFoundException(name);
    }
    
    try {
        std::string filename = it->second->getFilename();
        if (std::filesystem::exists(filename)) {
            std::filesystem::remove(filename);
        }
    } catch (const std::exception&) {
    }
    
    collections.erase(it);
}

std::vector<std::string> CollectionManager::getAllCollectionNames() const {
    std::vector<std::string> names;
    for (const auto& pair : collections) {
        names.push_back(pair.first);
    }
    std::sort(names.begin(), names.end());
    return names;
}

void CollectionManager::saveAll() const {
    for (const auto& pair : collections) {
        try {
            pair.second->save();
        } catch (const std::exception&) {
        }
    }
}

void CollectionManager::loadAll() {
    try {
        if (!std::filesystem::exists(collectionsDirectory)) {
            return;
        }
        
        for (const auto& entry : std::filesystem::directory_iterator(collectionsDirectory)) {
            if (entry.is_regular_file() && entry.path().extension() == ".txt") {
                std::string filepath = entry.path().string();
                
                std::ifstream file(filepath, std::ios::binary);
                std::string collectionName;
                if (file.is_open() && std::getline(file, collectionName)) {
                    if (collectionName.empty()) {
                        std::string filename = entry.path().filename().string();
                        collectionName = filename.substr(0, filename.length() - 4);
                        std::replace(collectionName.begin(), collectionName.end(), '_', ' ');
                        if (!collectionName.empty()) {
                            collectionName[0] = static_cast<char>(::toupper(collectionName[0]));
                        }
                    }
                } else {
                    std::string filename = entry.path().filename().string();
                    collectionName = filename.substr(0, filename.length() - 4);
                    std::replace(collectionName.begin(), collectionName.end(), '_', ' ');
                    if (!collectionName.empty()) {
                        collectionName[0] = static_cast<char>(::toupper(collectionName[0]));
                    }
                }
                
                if (!collectionName.empty()) {
                    auto collection = std::make_unique<MovieCollection>(collectionName, allMoviesRef);
                    collection->load();
                    collections[collectionName] = std::move(collection);
                }
            }
        }
    } catch (const std::exception&) {
    }
}

void CollectionManager::updateAllMoviesRef(const std::vector<Movie>* movies) {
    allMoviesRef = movies;
    for (auto& pair : collections) {
        pair.second->setAllMoviesRef(movies);
    }
}

