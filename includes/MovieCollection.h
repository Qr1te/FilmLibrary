#ifndef BETA2_MOVIECOLLECTION_H
#define BETA2_MOVIECOLLECTION_H

#include "movie.h"
#include "exceptions/exceptions.h"
#include <string>
#include <string_view>
#include <vector>
#include <memory>
#include <fstream>
#include <algorithm>
#include <ranges>
#include <set>
#include <map>

class IMovieCollection {
public:
    virtual ~IMovieCollection() = default;
    virtual void addMovie(const Movie& movie) = 0;
    virtual void removeMovie(int movieId) = 0;
    virtual bool containsMovie(int movieId) const = 0;
    virtual size_t size() const = 0;
    virtual std::vector<Movie> getMovies() const = 0;
    virtual void clear() = 0;
    virtual std::string getName() const = 0;
    
    virtual void save() const = 0;
    virtual void load() = 0;
};

class MovieCollection : public IMovieCollection {
private:
    std::string collectionName;
    std::string filename;
    std::vector<int> movieIds;
    const std::vector<Movie>* allMoviesRef;
    
    void validateMovieId(int id) const;
    void validateCollectionName(std::string_view name) const;
    
public:
    MovieCollection(const std::string& name, const std::vector<Movie>* allMovies);
    ~MovieCollection() override = default;
    
    void addMovie(const Movie& movie) override;
    void removeMovie(int movieId) override;
    bool containsMovie(int movieId) const override;
    size_t size() const override;
    std::vector<Movie> getMovies() const override;
    void clear() override;
    std::string getName() const override;
    
    void save() const override;
    void load() override;
    
    std::string getFilename() const;
    void setAllMoviesRef(const std::vector<Movie>* movies);
    
    std::vector<int>::const_iterator begin() const;
    std::vector<int>::const_iterator end() const;
};

template<typename Container>
class GenericMovieContainer {
private:
    Container movies;
    std::string containerName;
    
public:
    explicit GenericMovieContainer(const std::string& name = "") : containerName(name) {}
    
    void add(const Movie& movie) {
        movies.insert(movies.end(), movie);
    }
    
    void remove(int id) {
        auto it = std::ranges::find_if(movies,
                              [id](const Movie& m) { return m.getId() == id; });
        if (it != movies.end()) {
            movies.erase(it);
        }
    }
    
    typename Container::iterator findMovie(int id) {
        return std::ranges::find_if(movies,
                           [id](const Movie& m) { return m.getId() == id; });
    }
    
    typename Container::const_iterator findMovie(int id) const {
        return std::ranges::find_if(movies,
                           [id](const Movie& m) { return m.getId() == id; });
    }
    
    size_t count() const { return movies.size(); }
    bool empty() const { return movies.empty(); }
    
    typename Container::iterator begin() { return movies.begin(); }
    typename Container::iterator end() { return movies.end(); }
    typename Container::const_iterator begin() const { return movies.begin(); }
    typename Container::const_iterator end() const { return movies.end(); }
    
    std::string getName() const { return containerName; }
    void setName(std::string_view name) { containerName = std::string(name); }
};

template<typename Iterator, typename Predicate>
std::vector<Movie> findMovies(Iterator begin, Iterator end, Predicate pred) {
    std::vector<Movie> results;
    std::copy_if(begin, end, std::back_inserter(results), pred);
    return results;
}

template<std::ranges::random_access_range Container>
void sortMoviesByRating(Container& movies) {
    std::ranges::sort(movies,
              [](const Movie& a, const Movie& b) {
                  return a.getRating() > b.getRating();
              });
}

class CollectionManager {
private:
    std::map<std::string, std::unique_ptr<MovieCollection>, std::less<>> collections;
    const std::vector<Movie>* allMoviesRef;
    std::string collectionsDirectory;
    
public:
    explicit CollectionManager(const std::vector<Movie>* allMovies, const std::string& dir = "collections/");
    ~CollectionManager() = default;
    
    MovieCollection* createCollection(const std::string& name);
    
    MovieCollection* getCollection(const std::string& name);
    const MovieCollection* getCollection(const std::string& name) const;
    
    void deleteCollection(const std::string& name);
    
    std::vector<std::string> getAllCollectionNames() const;
    
    void saveAll() const;
    void loadAll();
    
    void updateAllMoviesRef(const std::vector<Movie>* movies);
};

#endif


