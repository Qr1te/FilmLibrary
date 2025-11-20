#ifndef BETA2_POSTERMANAGER_H
#define BETA2_POSTERMANAGER_H

#include <QObject>
#include <QLabel>
#include <QString>
#include <functional>
#include "movie.h"

class QNetworkAccessManager;
class QNetworkReply;

class PosterManager : public QObject {
    Q_OBJECT

public:
    explicit PosterManager(QObject* parent = nullptr);
    ~PosterManager() override;

    void loadPosterToLabel(QLabel* label, const Movie& movie) const;
    void loadPosterToLabelByTitle(QLabel* label, const QString& movieTitle) const;
    void downloadPoster(const QString& posterUrl, const QString& savePath, 
                       const std::function<void(bool)>& callback);
    
    void setNetworkManager(QNetworkAccessManager* manager);

private:
    QNetworkAccessManager* networkManager = nullptr;
    
    QStringList getSearchDirectories() const;
    QStringList getValidDirectories(const QStringList& searchDirs) const;
    QString normalizePath(const QString& path) const;
    
    QString findPosterFile(int movieId, const QString& posterPath = "") const;
    QString findPosterFileByTitle(const QString& movieTitle) const;
    void loadImageToLabel(QLabel* label, const QString& filePath) const;
    void onPosterDownloadFinished(QNetworkReply* reply, const QString& savePath, const std::function<void(bool)>& callback) const;
    
    // Helper functions for reducing complexity
    bool matchesMovieTitle(const QString& baseName, const QString& movieTitle) const;
    QString detectImageFormat(const QByteArray& imageData) const;
    QString detectImageExtension(const QByteArray& imageData) const;
    QImage loadImageFromData(const QByteArray& imageData, QString& detectedFormat) const;
    bool validateSavedFile(const QString& filePath) const;
    bool saveImageData(const QByteArray& imageData, const QString& savePath, const QString& fileExtension) const;
    bool saveImageFromQImage(const QImage& image, const QString& savePath, const QString& imageFormat) const;
};

#endif // BETA2_POSTERMANAGER_H


