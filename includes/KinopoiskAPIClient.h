#ifndef BETA2_KINOPOISKAPICLIENT_H
#define BETA2_KINOPOISKAPICLIENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QString>
#include <functional>
#include "movie.h"
#include "parsers/MovieJsonParser.h"

class KinopoiskAPIClient : public QObject {
    Q_OBJECT

public:
    explicit KinopoiskAPIClient(QObject* parent = nullptr);
    ~KinopoiskAPIClient() override;

    void searchMovie(const QString& title, 
                     const std::function<void(const Movie&, const QString&)>& onSuccess,
                     const std::function<void(const QString&)>& onError);
    
    void setApiKey(const QString& apiKey);
    QString getApiKey() const;

private slots:
    void onSearchFinished();
    void onDetailFinished();

private:
    QNetworkAccessManager* networkManager;
    QString apiKey;
    
    std::function<void(const Movie&, const QString&)> successCallback;
    std::function<void(const QString&)> errorCallback;
    QString currentTitle;
    QJsonObject searchResultJson;
    
    void fetchMovieDetails(int movieId);
    void handleError(QNetworkReply* reply, const QString& defaultMessage);
};

#endif // BETA2_KINOPOISKAPICLIENT_H


