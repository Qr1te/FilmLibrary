#include "../includes/MovieCollection.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

// MovieCollection implementation

MovieCollection::MovieCollection(const std::string& name, const std::vector<Movie>* allMovies)
    : collectionName(name), allMoviesRef(allMovies) {
    validateCollectionName(name);
    

    std::string safeName = name;
    std::replace(safeName.begin(), safeName.end(), ' ', '_');
    std::transform(safeName.begin(), safeName.end(), safeName.begin(), ::tolower);
    filename = "collections/" + safeName + ".txt";
    
    // Создаем директорию, если её нет
    try {
        std::filesystem::create_directories("collections");
    } catch (const std::exception&) {
        // Игнорируем ошибки создания директории
    }
}

void MovieCollection::validateCollectionName(const std::string& name) const {
    if (name.empty()) {
        throw InvalidInputException("Collection name cannot be empty");
    }
    
    // Проверка на недопустимые символы в имени файла
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
    
    // Проверяем, нет ли уже этого фильма в коллекции
    if (containsMovie(id)) {
        throw DuplicateFavoriteException(id);  // Используем существующее исключение
    }
    
    movieIds.push_back(id);
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

void MovieCollection::save() const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw FileNotFoundException(filename);
    }
    
    // Сохраняем в формате: ID коллекции на строку
    for (int id : movieIds) {
        file << id << "\n";
    }
}

void MovieCollection::load() {
    movieIds.clear();
    
    std::ifstream file(filename);
    if (!file.is_open()) {
        // Файл не существует - это нормально для новой коллекции
        return;
    }
    
    int id;
    while (file >> id) {
        if (file.fail()) {
            break;
        }
        // Проверяем, существует ли фильм с таким ID
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

// CollectionManager implementation

CollectionManager::CollectionManager(const std::vector<Movie>* allMovies, const std::string& dir)
    : allMoviesRef(allMovies), collectionsDirectory(dir) {
    try {
        std::filesystem::create_directories(collectionsDirectory);
    } catch (const std::exception&) {
        // Игнорируем ошибки
    }
    loadAll();
}

MovieCollection* CollectionManager::createCollection(const std::string& name) {
    if (collections.find(name) != collections.end()) {
        throw InvalidInputException("Collection with this name already exists");
    }
    
    auto collection = std::make_unique<MovieCollection>(name, allMoviesRef);
    MovieCollection* ptr = collection.get();
    collections[name] = std::move(collection);
    
    // Сохраняем коллекцию при создании
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
        throw MovieNotFoundException("Collection not found: " + name);
    }
    
    // Удаляем файл коллекции
    try {
        std::string filename = it->second->getFilename();
        if (std::filesystem::exists(filename)) {
            std::filesystem::remove(filename);
        }
    } catch (const std::exception&) {
        // Игнорируем ошибки удаления файла
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
            // Продолжаем сохранять остальные коллекции
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
                std::string filename = entry.path().filename().string();
                // Убираем расширение .txt
                std::string name = filename.substr(0, filename.length() - 4);
                
                // Заменяем подчеркивания на пробелы и делаем первую букву заглавной
                std::string displayName = name;
                std::replace(displayName.begin(), displayName.end(), '_', ' ');
                if (!displayName.empty()) {
                    displayName[0] = static_cast<char>(::toupper(displayName[0]));
                }
                
                // Создаем коллекцию и загружаем данные
                auto collection = std::make_unique<MovieCollection>(displayName, allMoviesRef);
                collection->load();
                collections[displayName] = std::move(collection);
            }
        }
    } catch (const std::exception&) {
        // Игнорируем ошибки загрузки
    }
}

void CollectionManager::updateAllMoviesRef(const std::vector<Movie>* movies) {
    allMoviesRef = movies;
    for (auto& pair : collections) {
        pair.second->setAllMoviesRef(movies);
    }
}

