#include "api/KinopoiskAPIClient.h"
#include "parsers/MovieJsonParser.h"
#include <QUrl>
#include <QUrlQuery>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QMessageBox>

KinopoiskAPIClient::KinopoiskAPIClient(QObject* parent)
    : QObject(parent), networkManager(new QNetworkAccessManager(this)) {
}

KinopoiskAPIClient::~KinopoiskAPIClient() = default;

void KinopoiskAPIClient::setApiKey(const QString& key) {
    apiKey = key;
}

QString KinopoiskAPIClient::getApiKey() const {
    return apiKey;
}

void KinopoiskAPIClient::searchMovie(const QString& title,
                                     const std::function<void(const Movie&, const QString&)>& onSuccess,
                                     const std::function<void(const QString&)>& onError) {
    if (title.isEmpty()) {
        if (onError) onError("Название фильма не может быть пустым");
        return;
    }
    
    successCallback = onSuccess;
    errorCallback = onError;
    currentTitle = title;
    
    QUrl url("https://api.kinopoisk.dev/v1.4/movie/search");
    QUrlQuery query;
    query.addQueryItem("query", title);
    query.addQueryItem("limit", "1");
    url.setQuery(query);
    
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Accept", "application/json");
    request.setRawHeader("X-API-KEY", apiKey.toUtf8());
    
    // NOSONAR: S5976 - Qt connect() requires non-const QObject pointer (Qt framework limitation)
    QNetworkReply* reply = networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, &KinopoiskAPIClient::onSearchFinished);
}

void KinopoiskAPIClient::onSearchFinished() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    
    reply->deleteLater();
    
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QByteArray data = reply->readAll();
    
    if (reply->error() != QNetworkReply::NoError) {
        handleError(reply, "Ошибка при поиске фильма через API.");
        return;
    }
    
    if (statusCode >= 400) {
        QString errorMsg = "Ошибка API: HTTP " + QString::number(statusCode);
        if (!data.isEmpty()) {
            errorMsg += "\n\nОтвет сервера: " + QString::fromUtf8(data.left(200));
        }
        if (errorCallback) errorCallback(errorMsg);
        return;
    }
    
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    
    if (parseError.error != QJsonParseError::NoError) {
        QString errorMsg = "Не удалось обработать ответ API.\n\nОшибка парсинга: " + parseError.errorString() +
                          "\n\nОтвет: " + QString::fromUtf8(data.left(200));
        if (errorCallback) errorCallback(errorMsg);
        return;
    }
    
    if (doc.isNull() || !doc.isObject()) {
        if (errorCallback) errorCallback("Фильм не найден");
        return;
    }
    
    QJsonObject root = doc.object();
    
    if (!root.contains("docs")) {
        if (errorCallback) errorCallback("Неожиданный формат ответа от API");
        return;
    }
    
    QJsonArray docs = root["docs"].toArray();
    
    if (docs.isEmpty()) {
        if (errorCallback) errorCallback("Фильм не найден");
        return;
    }
    
    searchResultJson = docs[0].toObject();
    int movieId = searchResultJson["id"].toInt();
    
    fetchMovieDetails(movieId);
}

void KinopoiskAPIClient::fetchMovieDetails(int movieId) {
    QUrl detailUrl(QString("https://api.kinopoisk.dev/v1.4/movie/%1").arg(movieId));
    QNetworkRequest detailRequest(detailUrl);
    detailRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    detailRequest.setRawHeader("Accept", "application/json");
    detailRequest.setRawHeader("X-API-KEY", apiKey.toUtf8());
    
    // NOSONAR: S5976 - Qt connect() requires non-const QObject pointer (Qt framework limitation)
    QNetworkReply* detailReply = networkManager->get(detailRequest);
    connect(detailReply, &QNetworkReply::finished, this, &KinopoiskAPIClient::onDetailFinished);
}

void KinopoiskAPIClient::onDetailFinished() const {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    
    reply->deleteLater();
    
    QJsonObject fullMovieJson = searchResultJson;
    
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray detailData = reply->readAll();
        QJsonParseError parseError;
        QJsonDocument detailDoc = QJsonDocument::fromJson(detailData, &parseError);
        
        if (parseError.error == QJsonParseError::NoError && detailDoc.isObject()) {
            mergeDetailData(fullMovieJson, detailDoc.object());
        }
    }
    
    Movie movie = MovieJsonParser::parseMovieFromJSON(fullMovieJson);
    QString posterUrl = MovieJsonParser::extractPosterUrl(fullMovieJson);
    
    if (movie.getId() == 0) {
        if (errorCallback) errorCallback("Не удалось обработать данные фильма");
        return;
    }
    
    if (successCallback) {
        successCallback(movie, posterUrl);
    }
}

void KinopoiskAPIClient::mergeDetailData(QJsonObject& fullMovieJson, const QJsonObject& detailObj) const {
    for (auto it = detailObj.begin(); it != detailObj.end(); ++it) {
        QString key = it.key();
        QJsonValue detailValue = it.value();
        
        if (detailValue.isArray()) {
            mergeJsonArray(fullMovieJson, key, detailValue.toArray());
        } else if (detailValue.isString()) {
            mergeJsonString(fullMovieJson, key, detailValue.toString());
        } else if (detailValue.isDouble()) {
            mergeJsonDouble(fullMovieJson, key, detailValue.toDouble());
        } else if (detailValue.isObject()) {
            mergeJsonObject(fullMovieJson, key, detailValue.toObject());
        } else {
            if (!fullMovieJson.contains(key)) {
                fullMovieJson[key] = detailValue;
            }
        }
    }
}

void KinopoiskAPIClient::mergeJsonArray(QJsonObject& fullMovieJson, const QString& key, const QJsonArray& detailArray) const {
    if (detailArray.isEmpty()) {
        return;
    }
    
    if (!fullMovieJson.contains(key)) {
        fullMovieJson[key] = detailArray;
        return;
    }
    
    if (!fullMovieJson[key].isArray()) {
        fullMovieJson[key] = detailArray;
        return;
    }
    
    QJsonArray existingArray = fullMovieJson[key].toArray();
    if (existingArray.isEmpty() || detailArray.size() > existingArray.size()) {
        fullMovieJson[key] = detailArray;
    }
}

void KinopoiskAPIClient::mergeJsonString(QJsonObject& fullMovieJson, const QString& key, const QString& detailStr) const {
    if (detailStr.isEmpty()) {
        return;
    }
    
    if (!fullMovieJson.contains(key)) {
        fullMovieJson[key] = detailStr;
        return;
    }
    
    if (!fullMovieJson[key].isString()) {
        fullMovieJson[key] = detailStr;
        return;
    }
    
    QString existingStr = fullMovieJson[key].toString();
    if (existingStr.isEmpty() || detailStr.length() > existingStr.length()) {
        fullMovieJson[key] = detailStr;
    }
}

void KinopoiskAPIClient::mergeJsonDouble(QJsonObject& fullMovieJson, const QString& key, double detailNum) const {
    if (detailNum == 0.0) {
        return;
    }
    
    if (!fullMovieJson.contains(key)) {
        fullMovieJson[key] = detailNum;
        return;
    }
    
    if (!fullMovieJson[key].isDouble()) {
        fullMovieJson[key] = detailNum;
        return;
    }
    
    double existingNum = fullMovieJson[key].toDouble();
    if (existingNum == 0.0 || (detailNum > 0 && detailNum > existingNum)) {
        fullMovieJson[key] = detailNum;
    }
}

void KinopoiskAPIClient::mergeJsonObject(QJsonObject& fullMovieJson, const QString& key, const QJsonObject& detailObjValue) const {
    if (!fullMovieJson.contains(key)) {
        fullMovieJson[key] = detailObjValue;
        return;
    }
    
    if (!fullMovieJson[key].isObject()) {
        fullMovieJson[key] = detailObjValue;
        return;
    }
    
    QJsonObject existingObj = fullMovieJson[key].toObject();
    if (detailObjValue.size() > existingObj.size()) {
        fullMovieJson[key] = detailObjValue;
        return;
    }
    
    for (auto objIt = detailObjValue.begin(); objIt != detailObjValue.end(); ++objIt) {
        if (!existingObj.contains(objIt.key())) {
            existingObj[objIt.key()] = objIt.value();
        } else if (objIt.value().isString() && existingObj[objIt.key()].isString()) {
            QString existingStr = existingObj[objIt.key()].toString();
            QString newStr = objIt.value().toString();
            if (existingStr.isEmpty() && !newStr.isEmpty()) {
                existingObj[objIt.key()] = objIt.value();
            }
        }
    }
    fullMovieJson[key] = existingObj;
}

void KinopoiskAPIClient::handleError(const QNetworkReply* reply, const QString& defaultMessage) const {
    QString errorMsg = defaultMessage;
    QString errorString = reply->errorString();
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    
    if (errorString.contains("TLS", Qt::CaseInsensitive) || 
        errorString.contains("SSL", Qt::CaseInsensitive) ||
        errorString.contains("initialization", Qt::CaseInsensitive)) {
        errorMsg += "\n\nОшибка SSL/TLS: не удалось установить безопасное соединение.";
        errorMsg += "\n\nВозможные решения:";
        errorMsg += "\n1. Убедитесь, что OpenSSL DLL файлы находятся в папке с exe";
        errorMsg += "\n2. Проверьте, что Qt SSL плагины скопированы в plugins/tls/";
        errorMsg += "\n3. Пересоберите проект для автоматического копирования SSL библиотек";
        errorMsg += "\n\nОшибка: " + errorString;
    } else if (statusCode == 401 || statusCode == 403) {
        errorMsg += "\n\nДля работы API требуется ключ доступа.\n"
                   "Получите бесплатный ключ на kinopoiskapi.dev\n"
                   "и добавьте его в код.";
    } else if (statusCode == 404) {
        errorMsg += "\n\nAPI endpoint не найден. Возможно, изменился адрес API.";
    } else if (statusCode == 429) {
        errorMsg += "\n\nПревышен лимит запросов. Попробуйте позже.";
    } else {
        errorMsg += "\n\nОшибка: " + errorString;
        if (statusCode > 0) {
            errorMsg += "\nHTTP код: " + QString::number(statusCode);
        }
    }
    
    if (errorCallback) errorCallback(errorMsg);
}

