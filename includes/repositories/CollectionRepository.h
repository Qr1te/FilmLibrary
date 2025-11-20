#ifndef BETA2_COLLECTIONREPOSITORY_H
#define BETA2_COLLECTIONREPOSITORY_H

#include "../models/MovieCollection.h"
#include <string>
#include <string_view>
#include <vector>
#include <memory>
#include <map>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QDir>

class CollectionRepository {
private:
    std::string collectionsDirectory;
    
public:
    explicit CollectionRepository(const std::string& dir = "collections/");
    
    void saveCollection(const MovieCollection& collection) const;
    void loadCollection(std::string_view name, MovieCollection& collection) const;
    std::vector<std::string> getAllCollectionNames() const;
    void deleteCollection(const std::string& name) const;
    bool collectionExists(const std::string& name) const;
};

#endif // BETA2_COLLECTIONREPOSITORY_H

