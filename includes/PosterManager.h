#ifndef BETA2_POSTERMANAGER_H
#define BETA2_POSTERMANAGER_H

#include <QObject>
#include <QLabel>
#include <QString>
#include <functional>
#include "movie.h"

class QNetworkAccessManager;

class PosterManager : public QObject {
    Q_OBJECT

public:
    explicit PosterManager(QObject* parent = nullptr);
    ~PosterManager() override;

    void loadPosterToLabel(QLabel* label, const Movie& movie);
    void loadPosterToLabelByTitle(QLabel* label, const QString& movieTitle);
    void downloadPoster(const QString& posterUrl, const QString& savePath, 
                       std::function<void(bool)> callback);
    
    void setNetworkManager(QNetworkAccessManager* manager);

private:
    QNetworkAccessManager* networkManager;
    
    QString findPosterFile(int movieId, const QString& posterPath = "");
    QString findPosterFileByTitle(const QString& movieTitle);
    void loadImageToLabel(QLabel* label, const QString& filePath);
};

#endif // BETA2_POSTERMANAGER_H

