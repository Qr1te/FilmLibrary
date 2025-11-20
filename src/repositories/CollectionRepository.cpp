#include "repositories/CollectionRepository.h"
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <ranges>
#include <iostream>
#include <string>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QFileInfo>
#include <QStringConverter>

CollectionRepository::CollectionRepository(const std::string& dir)
    : collectionsDirectory(dir) {
    try {
        std::filesystem::create_directories(collectionsDirectory);
    } catch (const std::filesystem::filesystem_error& e) {

        if (!std::filesystem::exists(collectionsDirectory)) {
            std::cerr << "Warning: Failed to create collections directory: " << e.what() << std::endl;
        }
    }
}

void CollectionRepository::saveCollection(const MovieCollection& collection) const {
    collection.save();
}

void CollectionRepository::loadCollection(std::string_view /*name*/, MovieCollection& collection) const {
    collection.load();
}

bool CollectionRepository::isGarbled(const QString& str) {
    if (str.isEmpty() || str.length() < 1) return true;
    if (str.contains("Р ") || str.contains("вЂ") || 
        str.contains("РІ") || str.contains("Р†") ||
        str.contains("Р вЂ") || str.contains("Р'В") ||
        str.contains("РЎ") || str.contains("РІР") ||
        str.startsWith("Collection_")) {
        return true;
    }
    // Check for patterns like "P B", "P'B" which are common in garbled text
    if (QString upperStr = str.toUpper(); (upperStr.contains("P ") && upperStr.contains("B")) ||
        upperStr.contains("P'B") || upperStr.contains("P B") ||
        upperStr.contains("P B--") || upperStr.contains("P B P") ||
        upperStr.contains("P B P'") || upperStr.contains("P'В") ||
        upperStr.contains("P B--") || upperStr.startsWith("P B")) {
        return true;
    }
    // Check for suspicious short names with special chars
    if (str.length() <= 5 && (str.contains("--") || str.contains("№") || 
        (str.contains("P") && str.contains("B")))) {
        return true;
    }
    // Check for patterns like "P B--" specifically
    if (str.contains("--") && (str.contains("P") || str.contains("B"))) {
        return true;
    }
    return false;
}

bool CollectionRepository::isValidCollectionName(const QString& collectionName) {
    if (collectionName.isEmpty()) {
        return false;
    }
    
    bool isNumber = false;
    collectionName.toInt(&isNumber);
    if (isNumber) {
        return false;
    }
    
    // Count replacement characters () which indicate encoding issues
    // If more than 10% are replacement chars, it's likely garbled
    if (int replacementCount = collectionName.count(QChar(0xFFFD)); 
        replacementCount > collectionName.length() / 10) {
        return false;
    }
    
    // Check for common garbled patterns using helper function
    if (isGarbled(collectionName)) {
        return false;
    }
    
    // Check if name is too short or suspicious
    if (collectionName.length() < 1) {
        return false;
    }
    
    // Check for suspicious patterns: many Cyrillic Р followed by spaces or other chars
    if (collectionName.count("Р ") > 2 || collectionName.count("Р'") > 2 ||
        collectionName.count("Р вЂ") > 1) {
        return false;
    }
    
    // Additional check: if name is very short (1-2 chars) and contains only Cyrillic,
    // it might be a single corrupted character
    if (collectionName.length() <= 2 && collectionName.toStdString().length() > collectionName.length() * 2) {
        // UTF-8 encoding issue - single char takes more than 2 bytes
        return false;
    }
    
    return true;
}

QString CollectionRepository::tryReadCollectionNameUTF8(QFile& file) const {
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QString();
    }
    
    QTextStream in(&file);
    in.setEncoding(QStringConverter::Encoding::Utf8);
    QString collectionName = in.readLine().trimmed();
    file.close();
    
    if (isValidCollectionName(collectionName)) {
        return collectionName;
    }
    
    return QString();
}

QString CollectionRepository::tryReadCollectionNameLocale(QFile& file) const {
    if (!file.open(QIODevice::ReadOnly)) {
        return QString();
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    // Try to find first line (until \n or \r\n)
    int lineEnd = data.indexOf('\n');
    if (lineEnd < 0) lineEnd = data.indexOf('\r');
    if (lineEnd <= 0) {
        return QString();
    }
    
    QByteArray firstLine = data.left(lineEnd).trimmed();
    
    // Try system locale encoding (often Windows-1251 on Russian Windows)
    QString testName = QString::fromLocal8Bit(firstLine);
    if (testName.isEmpty() || testName.contains(QChar(0xFFFD))) {
        return QString();
    }
    
    bool isNumber = false;
    testName.toInt(&isNumber);
    if (isNumber || !isValidCollectionName(testName)) {
        return QString();
    }
    
    return testName;
}

QString CollectionRepository::tryReadCollectionName(QFile& file) const {
    // Try UTF-8 first
    if (QString collectionName = tryReadCollectionNameUTF8(file); !collectionName.isEmpty()) {
        return collectionName;
    }
    
    // If UTF-8 didn't work, try reading as binary and interpreting in different encodings
    return tryReadCollectionNameLocale(file);
}

void CollectionRepository::deleteCorruptedFile(const QString& filePath) const {
    QFile corruptedFile(filePath);
    if (!corruptedFile.exists()) {
        return;
    }
    
    if (corruptedFile.remove()) {
        std::cerr << "Deleted corrupted collection file: " << filePath.toStdString() << std::endl;
        return;
    }
    
    std::cerr << "Failed to delete corrupted collection file: " << filePath.toStdString() << std::endl;
    // Try using filesystem remove as fallback
    try {
        std::filesystem::remove(filePath.toStdString());
        std::cerr << "Deleted corrupted collection file using filesystem: " << filePath.toStdString() << std::endl;
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error deleting file: " << e.what() << std::endl;
    }
}

std::vector<std::string> CollectionRepository::getAllCollectionNames() const {
    std::vector<std::string> names;
    
    try {
        QDir dir(QString::fromStdString(collectionsDirectory));
        if (!dir.exists()) {
            return names;
        }
        
        QStringList filters;
        filters << "*.txt";
        QFileInfoList fileList = dir.entryInfoList(filters, QDir::Files);
        
        for (const QFileInfo& fileInfo : fileList) {
            QString filePath = fileInfo.absoluteFilePath();
            QFile file(filePath);
            
            // Use name from file if valid
            if (QString collectionName = tryReadCollectionName(file); !collectionName.isEmpty()) {
                QByteArray utf8 = collectionName.toUtf8();
                names.emplace_back(utf8.constData(), utf8.length());
            } else {
                // Check if filename itself is valid (not garbled)
                QString filename = fileInfo.baseName();
                filename.replace('_', ' ');
                
                // Check if filename looks garbled using helper function
                bool filenameGarbled = isGarbled(filename);
                
                // Only use filename if it doesn't look garbled
                if (!filenameGarbled) {
                    QByteArray utf8 = filename.toUtf8();
                    names.emplace_back(utf8.constData(), utf8.length());
                } else {
                    // Filename is also garbled - delete the corrupted file
                    deleteCorruptedFile(filePath);
                }
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Warning: Filesystem error while getting collection names: " << e.what() << std::endl;
    } catch (const std::bad_alloc& e) {
        std::cerr << "Warning: Memory allocation error while getting collection names: " << e.what() << std::endl;
    } catch (const std::ios_base::failure& e) {
        std::cerr << "Warning: I/O error while getting collection names: " << e.what() << std::endl;
    }
    
    return names;
}

void CollectionRepository::deleteCollection(const std::string& name) const {
    std::string safeName = name;
    std::ranges::replace(safeName, ' ', '_');
    std::ranges::transform(safeName, safeName.begin(), ::tolower);
    std::filesystem::path filename = std::filesystem::path(collectionsDirectory) / (safeName + ".txt");
    
    if (std::filesystem::exists(filename)) {
        std::filesystem::remove(filename);
    }
}

bool CollectionRepository::collectionExists(const std::string& name) const {
    std::string safeName = name;
    std::ranges::replace(safeName, ' ', '_');
    std::ranges::transform(safeName, safeName.begin(), ::tolower);
    std::filesystem::path filename = std::filesystem::path(collectionsDirectory) / (safeName + ".txt");
    
    return std::filesystem::exists(filename);
}

