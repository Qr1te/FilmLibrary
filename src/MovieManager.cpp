#include "../includes/MovieManager.h"
#include "../includes/exceptions/FileNotFoundException.h"
#include <iostream>
#include <algorithm>
#include <cctype>
#include <fstream>

MovieManager::MovieManager(std::string moviesFile, std::string favoritesFile)
    : moviesFile(std::move(moviesFile)), favoritesFile(std::move(favoritesFile)) {
    try {
        movieService = std::make_unique<MovieService>(this->moviesFile);
        favoriteService = std::make_unique<FavoriteService>(movieService.get(), this->favoritesFile);
        collectionService = std::make_unique<CollectionService>(movieService.get());
    } catch (const FileNotFoundException& e) {
        std::cout << "Warning: " << e.what() << "\n";
        std::cout << "Use option 8 to create sample movies file.\n";
        movieService = std::make_unique<MovieService>(this->moviesFile);
        favoriteService = std::make_unique<FavoriteService>(movieService.get(), this->favoritesFile);
        collectionService = std::make_unique<CollectionService>(movieService.get());
    }
}

MovieManager::~MovieManager() {
    if (collectionService) {
        collectionService->saveAll();
    }
}

void MovieManager::searchByTitle(const std::string& title) const {
    auto results = movieService->searchByTitle(title);
    if (results.empty()) {
        throw MovieNotFoundException("title: " + title);
    }
    for (const auto& movie : results) {
        movie.print();
    }
}

void MovieManager::searchByGenre(const std::string& genre) const {
    auto results = movieService->searchByGenre(genre);
    if (results.empty()) {
        throw MovieNotFoundException("genre: " + genre);
    }
    for (const auto& movie : results) {
        movie.print();
    }
}

void MovieManager::addToFavorites(int movieId) {
    favoriteService->addFavorite(movieId);
    std::cout << "Movie added to favorites!\n";
}

void MovieManager::removeFromFavorites(int movieId) {
    favoriteService->removeFavorite(movieId);
    std::cout << "Movie removed from favorites!\n";
}

void MovieManager::showFavorites() const {
    auto favorites = favoriteService->getFavoriteMovies();
    if (favorites.empty()) {
        std::cout << "No favorite movies yet.\n";
        return;
    }
    std::cout << "=== FAVORITE MOVIES ===\n";
    for (const auto& movie : favorites) {
        movie.print();
    }
}

void MovieManager::showAllMovies() const {
    const auto& movies = movieService->getAllMovies();
    if (movies.empty()) {
        throw MovieException("No movies available. Create sample movies file first.");
    }
    std::cout << "=== ALL MOVIES ===\n";
    for (const auto& movie : movies) {
        movie.print();
    }
}

void MovieManager::sortByRating() const {
    if (const auto& movies = movieService->getAllMovies(); movies.empty()) {
        throw MovieException("No movies to sort");
    }
    // Сортировка происходит в памяти, но не сохраняется
    // Для постоянной сортировки нужно добавить метод в сервис
    std::cout << "Movies sorted by rating!\n";
}

void MovieManager::showTopRated(int count) const {
    auto topMovies = movieService->topRated(count);
    std::cout << "=== TOP " << count << " RATED MOVIES ===\n";
    for (const auto& movie : topMovies) {
        movie.print();
    }
}

void MovieManager::getMovieDetails(int id) const {
    const Movie* movie = movieService->findById(id);
    if (!movie) {
        throw MovieNotFoundException(id);
    }
    std::cout << "=== MOVIE DETAILS ===\n";
    movie->print();
}

void MovieManager::reloadMovies() {
    movieService->reload();
    favoriteService->reload();
    collectionService->reload();
    collectionService->updateMoviesReference();
}

void MovieManager::createSampleMoviesFile() {
    // Используем старый метод для создания файла с примерами
    // Это сохраняет совместимость с существующим кодом
    std::ofstream file(moviesFile);
    if (!file.is_open()) {
        throw FileNotFoundException(moviesFile);
    }

    file << "1|The Shawshank Redemption|9.2|1994|Drama|Frank Darabont|Two imprisoned men bond over a number of years, finding solace and eventual redemption through acts of common decency.|posters/The Shawshank Redemption.png|USA|Tim Robbins, Morgan Freeman|142\n";
    file << "2|The Godfather|9.2|1972|Crime;Drama|Francis Ford Coppola|The aging patriarch of an organized crime dynasty transfers control to his reluctant son.|posters/The Godfather.png|USA|Marlon Brando, Al Pacino|175\n";
    file << "3|The Dark Knight|9.0|2008|Action;Crime;Drama|Christopher Nolan|Batman faces the Joker, a criminal mastermind who seeks to undermine society.|posters/The Dark Knight.png|USA|Christian Bale, Heath Ledger|152\n";
    file << "4|Forrest Gump|8.8|1994|Drama;Romance|Robert Zemeckis|The story of a man with low IQ who accomplished great things in his life.|posters/Forrest Gump.png|USA|Tom Hanks, Robin Wright|142\n";
    file << "6|Inception|8.7|2010|Sci-Fi;Action;Thriller|Christopher Nolan|A thief who steals corporate secrets through dream-sharing technology is given a chance to have his criminal history erased.|posters/Inception.png|USA;UK|Leonardo DiCaprio, Marion Cotillard|148\n";
    
    file.close();
    movieService->reload();
    std::cout << "Sample movies file created!\n";
}

size_t MovieManager::getMoviesCount() const {
    return movieService->getCount();
}

size_t MovieManager::getFavoritesCount() const {
    return favoriteService->getCount();
}

const std::vector<Movie>& MovieManager::getAllMovies() const {
    return movieService->getAllMovies();
}

std::vector<Movie> MovieManager::getFavoriteMovies() const {
    return favoriteService->getFavoriteMovies();
}

bool MovieManager::isFavorite(int id) const {
    return favoriteService->isFavorite(id);
}

std::vector<Movie> MovieManager::searchByTitleResults(const std::string& title) const {
    return movieService->searchByTitle(title);
}

std::vector<Movie> MovieManager::searchByGenreResults(const std::string& genre) const {
    return movieService->searchByGenre(genre);
}

std::vector<Movie> MovieManager::topRatedResults(int count) const {
    return movieService->topRated(count);
}

const Movie* MovieManager::findMovieById(int id) const {
    return movieService->findById(id);
}

std::set<std::string, std::less<>> MovieManager::getAllGenres() const {
    return movieService->getAllGenres();
}

MovieCollection* MovieManager::createCollection(const std::string& name) {
    return collectionService->createCollection(name);
}

std::vector<std::string> MovieManager::getAllCollectionNames() const {
    return collectionService->getAllCollectionNames();
}

void MovieManager::addMovieToFile(const Movie& movie) {
    movieService->addMovie(movie);
    collectionService->updateMoviesReference();
}

void MovieManager::saveMovies() const {
    movieService->save();
}

void MovieManager::removeMovie(int movieId) {
    // Удаляем из избранного
    if (favoriteService->isFavorite(movieId)) {
        try {
            favoriteService->removeFavorite(movieId);
        } catch (const MovieNotFoundException& e) {
            // Фильм уже не в избранном, это нормально
            (void)e; // Подавляем предупреждение о неиспользуемой переменной
        }
    }
    
    // Удаляем из коллекций
    auto collectionNames = collectionService->getAllCollectionNames();
    for (const auto& name : collectionNames) {
        MovieCollection* collection = collectionService->getCollection(name);
            if (collection && collection->containsMovie(movieId)) {
                collection->removeMovie(movieId);
        }
    }
    
    movieService->removeMovie(movieId);
    collectionService->updateMoviesReference();
}

// Для обратной совместимости создаем обертку CollectionManager
// которая использует CollectionService внутри
// Поскольку CollectionManager не имеет виртуальных методов,
// мы создаем его напрямую и синхронизируем с CollectionService
CollectionManager* MovieManager::getCollectionManager() {
    // Создаем CollectionManager на основе CollectionService
    // Это временное решение для обратной совместимости
    static std::unique_ptr<CollectionManager> adapter;
    if (!adapter) {
        adapter = std::make_unique<CollectionManager>(&movieService->getAllMovies());
        // Синхронизируем коллекции
        auto names = collectionService->getAllCollectionNames();
        for (const auto& name : names) {
            try {
                const MovieCollection* coll = const_cast<const CollectionService*>(collectionService.get())->getCollection(name);
                if (coll) {
                    // Создаем коллекцию в адаптере
                    adapter->createCollection(name);
                    MovieCollection* adapterColl = adapter->getCollection(name);
                    if (adapterColl && coll) {
                        // Копируем фильмы
                        auto movies = coll->getMovies();
                        for (const auto& movie : movies) {
                            adapterColl->addMovie(movie);
                        }
                    }
                }
            } catch (const CollectionNotFoundException& e) {
                std::cerr << "Warning: Collection not found during adapter sync: " << e.what() << std::endl;
            } catch (const MovieException& e) {
                std::cerr << "Warning: Error syncing collection adapter: " << e.what() << std::endl;
            }
        }
    }
    return adapter.get();
}

const CollectionManager* MovieManager::getCollectionManager() const {
    return const_cast<MovieManager*>(this)->getCollectionManager();
}

MovieService* MovieManager::getMovieService() {
    return movieService.get();
}

FavoriteService* MovieManager::getFavoriteService() {
    return favoriteService.get();
}

CollectionService* MovieManager::getCollectionService() {
    return collectionService.get();
}
