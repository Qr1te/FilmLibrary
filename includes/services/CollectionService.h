#ifndef BETA2_COLLECTIONSERVICE_H
#define BETA2_COLLECTIONSERVICE_H

#include "../MovieCollection.h"
#include "../repositories/CollectionRepository.h"
#include "../services/MovieService.h"
#include <vector>
#include <string>
#include <memory>
#include <map>

class CollectionService {
private:
    CollectionRepository repository;
    MovieService* movieService;
    std::map<std::string, std::unique_ptr<MovieCollection>, std::less<>> collections;
    
public:
    explicit CollectionService(MovieService* movieService, const std::string& dir = "collections/");
    
    void reload();
    void saveAll() const;
    
    MovieCollection* createCollection(const std::string& name);
    MovieCollection* getCollection(const std::string& name);
    const MovieCollection* getCollection(const std::string& name) const;
    void deleteCollection(const std::string& name);
    
    std::vector<std::string> getAllCollectionNames() const;
    
    void updateMoviesReference();
};

#endif // BETA2_COLLECTIONSERVICE_H

