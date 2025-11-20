#include "../includes/MovieCollection.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <ranges>
#include <cctype>
#include <stdexcept>
#include <iostream>

// Helper function for C++20 compatibility (contains() is C++23)
// NOLINTNEXTLINE(clumsy, since-c++23) - using find() instead of contains() for C++20 compatibility
static bool string_view_contains(std::string_view str, char c) {
    return str.find(c) != std::string_view::npos;
}


MovieCollection::MovieCollection(const std::string& name, const std::vector<Movie>* allMovies)
    : collectionName(name), allMoviesRef(allMovies) {
    validateCollectionName(name);
    

    std::string safeName = name;
    std::ranges::replace(safeName, ' ', '_');
    std::ranges::transform(safeName, safeName.begin(), ::tolower);
    filename = "collections/" + safeName + ".txt";
    
    try {
        std::filesystem::create_directories("collections");
    } catch (const std::filesystem::filesystem_error& e) {
        // Директория может уже существовать, это не критично
        if (!std::filesystem::exists("collections")) {
            std::cerr << "Warning: Failed to create collections directory: " << e.what() << std::endl;
        }
    }
}

void MovieCollection::validateCollectionName(std::string_view name) const {
    if (name.empty()) {
        throw InvalidInputException("Collection name cannot be empty");
    }
    
    std::string invalidChars = R"(<>:"|?*\/)";
    for (char c : invalidChars) {
        if (string_view_contains(name, c)) {
            throw InvalidInputException("Collection name contains invalid characters");
        }
    }
}

void MovieCollection::validateMovieId(int id) const {
    if (!allMoviesRef) {
        throw MovieException("Movie reference not set");
    }
    
    auto it = std::ranges::find_if(*allMoviesRef,
                          [id](const Movie& m) { return m.getId() == id; });
    if (it == allMoviesRef->end()) {
        throw MovieNotFoundException(id);
    }
}

void MovieCollection::addMovie(const Movie& movie) {
    int id = movie.getId();
    validateMovieId(id);
    
    if (containsMovie(id)) {

        throw DuplicateFavoriteException(movie.getTitle());
    }
    
    movieIds.insert(movieIds.begin(), id);
    save();
}

void MovieCollection::removeMovie(int movieId) {
    auto it = std::ranges::find(movieIds, movieId);
    if (it == movieIds.end()) {
        throw MovieNotFoundException(movieId);
    }
    
    movieIds.erase(it);
    save();
}

bool MovieCollection::containsMovie(int movieId) const {
    return std::ranges::find(movieIds, movieId) != movieIds.end();
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
        auto it = std::ranges::find_if(*allMoviesRef,
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
    
    // Skip first line if it exists (collection name)
    if (std::string line; std::getline(file, line)) {
        // Line read and discarded
    }
    
    int id;
    while (file >> id) {
        if (file.fail()) {
            break;
        }
        if (allMoviesRef) {
            bool exists = std::ranges::any_of(*allMoviesRef,
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
    } catch (const std::filesystem::filesystem_error& e) {
        // Директория может уже существовать, это не критично
        if (!std::filesystem::exists(collectionsDirectory)) {
            std::cerr << "Warning: Failed to create collections directory: " << e.what() << std::endl;
        }
    }
    loadAll();
}

MovieCollection* CollectionManager::createCollection(const std::string& name) {
    if (collections.contains(name)) {
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
        std::filesystem::path filename = it->second->getFilename();
        if (std::filesystem::exists(filename)) {
            std::filesystem::remove(filename);
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Warning: Failed to delete collection file: " << e.what() << std::endl;
    }
    
    collections.erase(it);
}

std::vector<std::string> CollectionManager::getAllCollectionNames() const {
    std::vector<std::string> names;
    for (const auto& [name, collection] : collections) {
        names.push_back(name);
    }
    std::ranges::sort(names);
    return names;
}

void CollectionManager::saveAll() const {
    for (const auto& [name, collection] : collections) {
        try {
            collection->save();
        } catch (const FileNotFoundException& e) {
            std::cerr << "Warning: Failed to save collection '" << name << "': " << e.what() << std::endl;
        } catch (const MovieException& e) {
            std::cerr << "Warning: Error saving collection '" << name << "': " << e.what() << std::endl;
        }
    }
}

std::string CollectionManager::extractCollectionName(const std::filesystem::path& filepath) const {
    std::ifstream file(filepath.string(), std::ios::binary);
    std::string collectionName;
    if (file.is_open() && std::getline(file, collectionName) && !collectionName.empty()) {
        return collectionName;
    }
    
    std::string filename = filepath.filename().string();
    collectionName = filename.substr(0, filename.length() - 4);
    std::ranges::replace(collectionName, '_', ' ');
    if (!collectionName.empty()) {
        collectionName[0] = static_cast<char>(::toupper(collectionName[0]));
    }
    return collectionName;
}

void CollectionManager::loadAll() {
    try {
        if (!std::filesystem::exists(collectionsDirectory)) {
            return;
        }
        
        for (const auto& entry : std::filesystem::directory_iterator(collectionsDirectory)) {
            if (!entry.is_regular_file() || entry.path().extension() != ".txt") {
                continue;
            }
            
            std::string collectionName = extractCollectionName(entry.path());
            
            if (collectionName.empty()) {
                continue;
            }
            
            auto collection = std::make_unique<MovieCollection>(collectionName, allMoviesRef);
            collection->load();
            collections[collectionName] = std::move(collection);
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Warning: Filesystem error while loading collections: " << e.what() << std::endl;
    } catch (const FileNotFoundException& e) {
        std::cerr << "Warning: Collection file not found: " << e.what() << std::endl;
    }
}

void CollectionManager::updateAllMoviesRef(const std::vector<Movie>* movies) {
    allMoviesRef = movies;
    for (const auto& [name, collection] : collections) {
        collection->setAllMoviesRef(movies);
    }
}

