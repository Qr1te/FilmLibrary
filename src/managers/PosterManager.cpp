#include "managers/PosterManager.h"
#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QImage>
#include <QImageReader>
#include <QPixmap>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QFile>
#include <QThread>
#include <QDebug>

PosterManager::PosterManager(QObject* parent)
    : QObject(parent) {
}

PosterManager::~PosterManager() = default;

void PosterManager::setNetworkManager(QNetworkAccessManager* manager) {
    networkManager = manager;
}

QStringList PosterManager::getSearchDirectories() const {
    QString appDir = QApplication::applicationDirPath();
    QString currentDir = QDir::currentPath();
    QString sep = QDir::separator();
    
    QStringList searchDirs;
    auto addDir = [&searchDirs](const QString& path) {
        QString normalized = QDir::cleanPath(path);
        if (!normalized.isEmpty() && !searchDirs.contains(normalized)) {
            searchDirs << normalized;
        }
    };
    
    addDir(appDir + sep + "posters");
    addDir(appDir + sep + ".." + sep + "posters");
    addDir(appDir + sep + ".." + sep + ".." + sep + "posters");
    addDir(currentDir + sep + "posters");
    addDir(currentDir + sep + ".." + sep + "posters");
    addDir("posters");
    addDir(".." + sep + "posters");
    addDir(".." + sep + ".." + sep + "posters");
    addDir(appDir);
    
    return searchDirs;
}

QStringList PosterManager::getValidDirectories(const QStringList& searchDirs) const {
    QStringList validDirs;
    for (const QString& dir : searchDirs) {
        QDir testDir(dir);
        if (testDir.exists()) {
            QString absPath = testDir.absolutePath();
            if (!validDirs.contains(absPath)) {
                validDirs << absPath;
            }
        }
    }
    return validDirs;
}

QString PosterManager::normalizePath(const QString& path) const {
    QString currentDir = QDir::currentPath();
    QString appDir = QApplication::applicationDirPath();
    
    QString fullPath = path;
    if (!QDir::isAbsolutePath(fullPath)) {
        QString testPath1 = QDir(currentDir).absoluteFilePath(fullPath);
        QString testPath2 = QDir(appDir).absoluteFilePath(fullPath);
        if (QFile::exists(testPath1)) {
            fullPath = testPath1;
        } else if (QFile::exists(testPath2)) {
            fullPath = testPath2;
        }
    }
    return QDir::cleanPath(fullPath);
}

QString PosterManager::findPosterFile(int movieId, const QString& posterPath) const {
    QString appDir = QApplication::applicationDirPath();
    QString currentDir = QDir::currentPath();
    QString sep = QDir::separator();
    
    QString fileName = QFileInfo(posterPath).fileName();
    if (fileName.isEmpty() || !fileName.contains(QString::number(movieId))) {
        fileName = QString::number(movieId) + ".jpg";
    }
    
    QStringList searchDirs = getSearchDirectories();
    QStringList validDirs = getValidDirectories(searchDirs);
    
    QStringList possiblePaths;
    
    if (!posterPath.isEmpty()) {
        possiblePaths << posterPath;
        possiblePaths << appDir + sep + posterPath;
        possiblePaths << currentDir + sep + posterPath;
    }
    
    QString idFileNameJpg = QString::number(movieId) + ".jpg";
    QString idFileNamePng = QString::number(movieId) + ".png";
    
    possiblePaths << "posters" + sep + idFileNameJpg;
    possiblePaths << "posters" + sep + idFileNamePng;
    
    for (const QString& dir : validDirs) {
        possiblePaths << dir + sep + idFileNameJpg;
        possiblePaths << dir + sep + idFileNamePng;
    }
    
    if (!fileName.isEmpty() && fileName != idFileNameJpg && fileName != idFileNamePng) {
        possiblePaths << "posters" + sep + fileName;
        for (const QString& dir : validDirs) {
            possiblePaths << dir + sep + fileName;
        }
    }
    
    for (const QString& path : possiblePaths) {
        QString normalizedPath = QDir::cleanPath(path);
        if (!QDir::isAbsolutePath(normalizedPath)) {
            normalizedPath = QDir(currentDir).absoluteFilePath(normalizedPath);
        }
        if (QFile::exists(normalizedPath)) {
            return QDir::cleanPath(normalizedPath);
        }
    }
    
    if (!validDirs.isEmpty() && movieId > 0) {
        for (const QString& dir : validDirs) {
            QDir searchDir(dir);
            QString idJpg = QString::number(movieId) + ".jpg";
            QString idPng = QString::number(movieId) + ".png";
            
            QString testPath = searchDir.absoluteFilePath(idJpg);
            if (QFile::exists(testPath)) {
                return QDir::cleanPath(testPath);
            }
            
            testPath = searchDir.absoluteFilePath(idPng);
            if (QFile::exists(testPath)) {
                return QDir::cleanPath(testPath);
            }
        }
    }
    
    return "";
}

bool PosterManager::matchesMovieTitle(const QString& baseName, const QString& movieTitle) const {
    QString normalizedBaseName = baseName.toLower().remove(' ').remove('-').remove('_').remove('\'');
    QString normalizedTitle = movieTitle.toLower().remove(' ').remove('-').remove('_').remove('\'');
    
    if (normalizedBaseName == normalizedTitle) {
        return true;
    }
    if (normalizedBaseName.contains(normalizedTitle) || normalizedTitle.contains(normalizedBaseName)) {
        return true;
    }
    if (baseName.toLower().contains(movieTitle.toLower()) || movieTitle.toLower().contains(baseName.toLower())) {
        return true;
    }
    return false;
}

QString PosterManager::findPosterFileByTitle(const QString& movieTitle) const {
    QStringList searchDirs = getSearchDirectories();
    QStringList validDirs = getValidDirectories(searchDirs);
    
    if (validDirs.isEmpty()) {
        return "";
    }
    
    for (const QString& dir : validDirs) {
        QDir searchDir(dir);
        QFileInfoList files = searchDir.entryInfoList(QStringList() << "*.png" << "*.jpg" << "*.jpeg", QDir::Files);
        
        for (const QFileInfo& fileInfo : files) {
            QString baseName = fileInfo.baseName();
            if (matchesMovieTitle(baseName, movieTitle)) {
                return fileInfo.absoluteFilePath();
            }
        }
    }
    
    return "";
}

void PosterManager::loadImageToLabel(QLabel* label, const QString& filePath) const {
    if (!label || filePath.isEmpty()) return;
    
    QString absolutePath = QFileInfo(filePath).absoluteFilePath();
    
    QImageReader reader(absolutePath);
    QImage image;
    
    if (reader.canRead()) {
        reader.setAutoTransform(true);
        image = reader.read();
    }
    
    if (image.isNull()) {
        image = QImage(absolutePath);
    }
    
    if (!image.isNull()) {
        QPixmap pixmap = QPixmap::fromImage(image);
        QSize labelSize = label->size();
        if (labelSize.width() <= 0 || labelSize.height() <= 0) {
            labelSize = label->maximumSize();
            if (labelSize.width() <= 0 || labelSize.height() <= 0) {
                labelSize = label->minimumSize();
            }
            if (labelSize.width() <= 0 || labelSize.height() <= 0) {
                labelSize = QSize(450, 675);
            }
        }
        pixmap = pixmap.scaled(labelSize.width(), labelSize.height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        label->setPixmap(pixmap);
        label->setAlignment(Qt::AlignCenter);
        label->setScaledContents(false);
    }
}

void PosterManager::loadPosterToLabel(QLabel* label, const Movie& movie) const {
    if (!label) return;
    
    QString posterPath = QString::fromStdString(movie.getPosterPath());
    int movieId = movie.getId();
    
    if (QString fullPath = findPosterFile(movieId, posterPath); !fullPath.isEmpty()) {
        fullPath = normalizePath(fullPath);
        
        if (QFile::exists(fullPath)) {
            loadImageToLabel(label, fullPath);
            return;
        }
    }
    
    label->setText("No Poster\n" + QString::fromStdString(movie.getTitle()));
    label->setStyleSheet("background-color: #4a4a4a; color: #999; border-radius: 4px;");
    label->setAlignment(Qt::AlignCenter);
}

void PosterManager::loadPosterToLabelByTitle(QLabel* label, const QString& movieTitle) const {
    if (!label) return;
    
    if (QString fullPath = findPosterFileByTitle(movieTitle); !fullPath.isEmpty()) {
        fullPath = normalizePath(fullPath);
        
        if (QFile::exists(fullPath)) {
            loadImageToLabel(label, fullPath);
            return;
        }
    }
    
    label->setText("🎬\n" + movieTitle);
    label->setStyleSheet("background-color: #2b2b2b; color: #999; border-radius: 4px; border: 2px solid #555; min-height: 360px;");
    label->setAlignment(Qt::AlignCenter);
    label->setWordWrap(true);
}

void PosterManager::downloadPoster(const QString& posterUrl, const QString& savePath, 
                                   const std::function<void(bool)>& callback) {
    if (!networkManager) {
        if (callback) callback(false);
        return;
    }
    
    if (posterUrl.isEmpty()) {
        if (callback) callback(false);
        return;
    }
    
    QUrl url(posterUrl);
    if (!url.isValid()) {
        if (callback) callback(false);
        return;
    }
    
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36");
    request.setRawHeader("Accept", "image/*");
    request.setRawHeader("Referer", "https://www.kinopoisk.ru/");
    
    QNetworkReply* reply = networkManager->get(request);
    
    connect(reply, &QNetworkReply::finished, [this, reply, savePath, callback]() {
        onPosterDownloadFinished(reply, savePath, callback);
    });
}

QString PosterManager::detectImageFormat(const QByteArray& imageData) const {
    QByteArray jpegHeader = QByteArray::fromHex("FFD8FF");
    QByteArray pngHeader = QByteArray::fromHex("89") + "PNG";
    
    if (imageData.startsWith(jpegHeader)) {
        return "JPEG";
    }
    if (imageData.startsWith(pngHeader)) {
        return "PNG";
    }
    if (imageData.startsWith("GIF")) {
        return "GIF";
    }
    if (imageData.startsWith("BM")) {
        return "BMP";
    }
    return "JPEG";
}

QString PosterManager::detectImageExtension(const QByteArray& imageData) const {
    QByteArray jpegHeader = QByteArray::fromHex("FFD8FF");
    QByteArray pngHeader = QByteArray::fromHex("89") + "PNG";
    
    if (imageData.startsWith(jpegHeader)) {
        return "jpg";
    }
    if (imageData.startsWith(pngHeader)) {
        return "png";
    }
    if (imageData.startsWith("GIF")) {
        return "gif";
    }
    if (imageData.startsWith("BM")) {
        return "bmp";
    }
    return "jpg";
}

QImage PosterManager::loadImageFromData(const QByteArray& imageData, QString& detectedFormat) const {
    QImage image;
    
    if (QString initialFormat = detectedFormat; 
        image.loadFromData(imageData) || image.loadFromData(imageData, initialFormat.toUtf8().constData())) {
        return image;
    }
    
    QStringList formats = {"JPEG", "JPG", "PNG", "GIF", "BMP"};
    for (const QString& fmt : formats) {
        if (image.loadFromData(imageData, fmt.toUtf8().constData())) {
            detectedFormat = fmt;
            return image;
        }
    }
    
    return QImage();
}

bool PosterManager::validateSavedFile(const QString& filePath) const {
    if (QImage testImage; testImage.load(filePath)) {
        return true;
    }
    if (QFile::exists(filePath) && QFileInfo(filePath).size() > 0) {
        return true;
    }
    return false;
}

bool PosterManager::saveImageData(const QByteArray& imageData, const QString& savePath, const QString& fileExtension) const {
    QFileInfo pathInfo(savePath);
    QString sep = QDir::separator();
    QString correctPath = pathInfo.path() + sep + pathInfo.baseName() + "." + fileExtension;
    
    QFile file(correctPath);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    
    qint64 bytesWritten = file.write(imageData);
    file.close();
    
    if (bytesWritten <= 0 || bytesWritten != imageData.size()) {
        return false;
    }
    
    file.flush();
    QThread::msleep(50);
    
    return validateSavedFile(correctPath);
}

bool PosterManager::saveImageFromQImage(const QImage& image, const QString& savePath, const QString& imageFormat) const {
    if (image.isNull()) {
        return false;
    }
    
    QFileInfo pathInfo(savePath);
    QString sep = QDir::separator();
    QString fileExtension = "jpg";
    
    if (imageFormat == "PNG") {
        fileExtension = "png";
    } else if (imageFormat == "GIF") {
        fileExtension = "gif";
    } else if (imageFormat == "BMP") {
        fileExtension = "bmp";
    }
    
    QString correctPath = pathInfo.path() + sep + pathInfo.baseName() + "." + fileExtension;
    
    QFile file(correctPath);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    
    QString saveFormat = imageFormat;
    if (saveFormat == "JPG") {
        saveFormat = "JPEG";
    }
    
    bool saved = image.save(&file, saveFormat.toUtf8().constData());
    file.close();
    
    return saved;
}

void PosterManager::onPosterDownloadFinished(QNetworkReply* reply, const QString& savePath, const std::function<void(bool)>& callback) const {
    bool success = false;
    
    if (reply->error() != QNetworkReply::NoError) {
        reply->deleteLater();
        if (callback) callback(false);
        return;
    }
    
    QByteArray imageData = reply->readAll();
    if (imageData.isEmpty()) {
        reply->deleteLater();
        if (callback) callback(false);
        return;
    }
    
    QString imageFormat = detectImageFormat(imageData);
    QString fileExtension = detectImageExtension(imageData);
    
    QImage image;
    QString detectedFormat = imageFormat;
    image = loadImageFromData(imageData, detectedFormat);
    if (!detectedFormat.isEmpty()) {
        imageFormat = detectedFormat;
    }
    
    success = saveImageData(imageData, savePath, fileExtension);
    
    if (!success && !image.isNull()) {
        success = saveImageFromQImage(image, savePath, imageFormat);
    }
    
    reply->deleteLater();
    
    if (callback) {
        callback(success);
    }
}

