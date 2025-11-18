#ifndef BETA2_COLLECTIONREPOSITORY_H
#define BETA2_COLLECTIONREPOSITORY_H

#include "../MovieCollection.h"
#include <string>
#include <vector>
#include <memory>
#include <map>

class CollectionRepository {
private:
    std::string collectionsDirectory;
    
public:
    explicit CollectionRepository(const std::string& dir = "collections/");
    
    void saveCollection(const MovieCollection& collection) const;
    void loadCollection(const std::string& name, MovieCollection& collection) const;
    std::vector<std::string> getAllCollectionNames() const;
    void deleteCollection(const std::string& name) const;
    bool collectionExists(const std::string& name) const;
};

#endif // BETA2_COLLECTIONREPOSITORY_H

