#include "../includes/KinopoiskAPIClient.h"
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
    : QObject(parent), networkManager(new QNetworkAccessManager(this)), apiKey("E42W0MH-8HDMQ8D-JDZ6HG2-X3RCZHZ") {
}

KinopoiskAPIClient::~KinopoiskAPIClient() = default;

void KinopoiskAPIClient::setApiKey(const QString& key) {
    apiKey = key;
}

QString KinopoiskAPIClient::getApiKey() const {
    return apiKey;
}

void KinopoiskAPIClient::searchMovie(const QString& title,
                                     std::function<void(const Movie&, const QString&)> onSuccess,
                                     std::function<void(const QString&)> onError) {
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
    
    QNetworkReply* detailReply = networkManager->get(detailRequest);
    connect(detailReply, &QNetworkReply::finished, this, &KinopoiskAPIClient::onDetailFinished);
}

void KinopoiskAPIClient::onDetailFinished() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    
    reply->deleteLater();
    
    QJsonObject fullMovieJson = searchResultJson;
    
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray detailData = reply->readAll();
        QJsonParseError parseError;
        QJsonDocument detailDoc = QJsonDocument::fromJson(detailData, &parseError);
        
        if (parseError.error == QJsonParseError::NoError && detailDoc.isObject()) {
            QJsonObject detailObj = detailDoc.object();
            for (auto it = detailObj.begin(); it != detailObj.end(); ++it) {
                QString key = it.key();
                QJsonValue detailValue = it.value();
                
                if (detailValue.isArray()) {
                    QJsonArray detailArray = detailValue.toArray();
                    if (!detailArray.isEmpty()) {
                        if (!fullMovieJson.contains(key)) {
                            fullMovieJson[key] = detailValue;
                        } else if (fullMovieJson[key].isArray()) {
                            QJsonArray existingArray = fullMovieJson[key].toArray();
                            if (existingArray.isEmpty() || detailArray.size() > existingArray.size()) {
                                fullMovieJson[key] = detailValue;
                            }
                        } else {
                            fullMovieJson[key] = detailValue;
                        }
                    }
                }
                else if (detailValue.isString()) {
                    QString detailStr = detailValue.toString();
                    if (!detailStr.isEmpty()) {
                        if (!fullMovieJson.contains(key)) {
                            fullMovieJson[key] = detailValue;
                        } else if (fullMovieJson[key].isString()) {
                            QString existingStr = fullMovieJson[key].toString();
                            if (existingStr.isEmpty() || detailStr.length() > existingStr.length()) {
                                fullMovieJson[key] = detailValue;
                            }
                        } else {
                            fullMovieJson[key] = detailValue;
                        }
                    }
                }
                else if (detailValue.isDouble()) {
                    double detailNum = detailValue.toDouble();
                    if (detailNum != 0.0) {
                        if (!fullMovieJson.contains(key)) {
                            fullMovieJson[key] = detailValue;
                        } else if (fullMovieJson[key].isDouble()) {
                            double existingNum = fullMovieJson[key].toDouble();
                            if (existingNum == 0.0 || (detailNum > 0 && detailNum > existingNum)) {
                                fullMovieJson[key] = detailValue;
                            }
                        } else {
                            fullMovieJson[key] = detailValue;
                        }
                    }
                }
                else if (detailValue.isObject()) {
                    if (!fullMovieJson.contains(key)) {
                        fullMovieJson[key] = detailValue;
                    } else if (fullMovieJson[key].isObject()) {
                        QJsonObject existingObj = fullMovieJson[key].toObject();
                        QJsonObject detailObjValue = detailValue.toObject();
                        if (detailObjValue.size() > existingObj.size()) {
                            fullMovieJson[key] = detailValue;
                        } else {
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
                    } else {
                        fullMovieJson[key] = detailValue;
                    }
                }
                else {
                    if (!fullMovieJson.contains(key)) {
                        fullMovieJson[key] = detailValue;
                    }
                }
            }
        }
    }
    
    Movie movie = parseMovieFromJSON(fullMovieJson);
    QString posterUrl = extractPosterUrl(fullMovieJson);
    
    if (movie.getId() == 0) {
        if (errorCallback) errorCallback("Не удалось обработать данные фильма");
        return;
    }
    
    if (successCallback) {
        successCallback(movie, posterUrl);
    }
}

Movie KinopoiskAPIClient::parseMovieFromJSON(const QJsonObject& json) {
    int id = json["id"].toInt();
    QString name = json["name"].toString();
    if (name.isEmpty()) {
        name = json["alternativeName"].toString();
    }
    
    double rating = 0.0;
    if (json.contains("rating")) {
        QJsonObject ratingObj = json["rating"].toObject();
        rating = ratingObj["kp"].toDouble();
        if (rating == 0.0) {
            rating = ratingObj["imdb"].toDouble();
        }
    }
    
    int year = json["year"].toInt();
    
    QStringList genresList;
    if (json.contains("genres")) {
        QJsonArray genresArray = json["genres"].toArray();
        for (const QJsonValue& value : genresArray) {
            QString genreName = value.toObject()["name"].toString();
            if (!genreName.isEmpty()) {
                genresList << genreName;
            }
        }
    }
    
    QString director = "";
    if (json.contains("persons")) {
        QJsonArray personsArray = json["persons"].toArray();
        for (const QJsonValue& value : personsArray) {
            QJsonObject person = value.toObject();
            QString profession = person["enProfession"].toString();
            QString professionRu = person["profession"].toString();
            
            if (profession == "director" || professionRu == "режиссер" || professionRu.contains("режиссер", Qt::CaseInsensitive)) {
                QString name = person["name"].toString();
                if (name.isEmpty()) {
                    name = person["enName"].toString();
                }
                if (name.isEmpty()) {
                    name = person["alternativeName"].toString();
                }
                if (!name.isEmpty()) {
                    director = name;
                    break;
                }
            }
        }
    }
    if (director.isEmpty() && json.contains("director")) {
        QJsonValue directorValue = json["director"];
        if (directorValue.isString()) {
            director = directorValue.toString();
        } else if (directorValue.isObject()) {
            director = directorValue.toObject()["name"].toString();
        }
    }
    
    QString description = json["description"].toString();
    if (description.isEmpty()) {
        description = json["shortDescription"].toString();
    }
    
    QString country = "";
    if (json.contains("countries")) {
        QJsonArray countriesArray = json["countries"].toArray();
        QStringList countries;
        for (const QJsonValue& value : countriesArray) {
            if (value.isObject()) {
                QJsonObject countryObj = value.toObject();
                QString countryName = countryObj["name"].toString();
                if (countryName.isEmpty()) {
                    countryName = countryObj["enName"].toString();
                }
                if (countryName.isEmpty()) {
                    countryName = countryObj["alternativeName"].toString();
                }
                if (!countryName.isEmpty()) {
                    countries << countryName;
                }
            } else if (value.isString()) {
                QString countryName = value.toString();
                if (!countryName.isEmpty()) {
                    countries << countryName;
                }
            }
        }
        country = countries.join(", ");
    }
    if (country.isEmpty() && json.contains("country")) {
        QJsonValue countryValue = json["country"];
        if (countryValue.isString()) {
            country = countryValue.toString();
        } else if (countryValue.isArray()) {
            QStringList countries;
            QJsonArray countryArray = countryValue.toArray();
            for (const QJsonValue& value : countryArray) {
                if (value.isString()) {
                    countries << value.toString();
                } else if (value.isObject()) {
                    QJsonObject countryObj = value.toObject();
                    QString name = countryObj["name"].toString();
                    if (name.isEmpty()) {
                        name = countryObj["enName"].toString();
                    }
                    if (!name.isEmpty()) {
                        countries << name;
                    }
                }
            }
            country = countries.join(", ");
        } else if (countryValue.isObject()) {
            QJsonObject countryObj = countryValue.toObject();
            country = countryObj["name"].toString();
            if (country.isEmpty()) {
                country = countryObj["enName"].toString();
            }
        }
    }
    if (country.isEmpty() && json.contains("productionCountries")) {
        QJsonValue productionCountries = json["productionCountries"];
        if (productionCountries.isArray()) {
            QStringList countries;
            QJsonArray countriesArray = productionCountries.toArray();
            for (const QJsonValue& value : countriesArray) {
                if (value.isObject()) {
                    QString name = value.toObject()["name"].toString();
                    if (!name.isEmpty()) {
                        countries << name;
                    }
                } else if (value.isString()) {
                    countries << value.toString();
                }
            }
            country = countries.join(", ");
        }
    }
    
    QString actors = "";
    if (json.contains("persons")) {
        QJsonArray personsArray = json["persons"].toArray();
        QStringList actorsList;
        int count = 0;
        for (const QJsonValue& value : personsArray) {
            if (!value.isObject()) continue;
            QJsonObject person = value.toObject();
            QString profession = person["enProfession"].toString();
            QString professionRu = person["profession"].toString();
            
            bool isActor = (profession == "actor" || 
                           profession == "актер" ||
                           professionRu.contains("актер", Qt::CaseInsensitive) ||
                           professionRu.contains("актриса", Qt::CaseInsensitive) ||
                           professionRu == "актер" || 
                           professionRu == "актриса" ||
                           professionRu.contains("actor", Qt::CaseInsensitive));
            
            if (isActor && count < 5) {
                QString name = person["name"].toString();
                if (name.isEmpty()) {
                    name = person["enName"].toString();
                }
                if (name.isEmpty()) {
                    name = person["alternativeName"].toString();
                }
                if (name.isEmpty() && person.contains("names")) {
                    QJsonArray namesArray = person["names"].toArray();
                    if (!namesArray.isEmpty()) {
                        name = namesArray[0].toObject()["name"].toString();
                    }
                }
                if (!name.isEmpty()) {
                    actorsList << name;
                    count++;
                }
            }
        }
        actors = actorsList.join(", ");
    }
    if (actors.isEmpty() && json.contains("actors")) {
        QJsonValue actorsValue = json["actors"];
        if (actorsValue.isString()) {
            actors = actorsValue.toString();
        } else if (actorsValue.isArray()) {
            QStringList actorsList;
            QJsonArray actorsArray = actorsValue.toArray();
            for (const QJsonValue& value : actorsArray) {
                if (value.isString()) {
                    actorsList << value.toString();
                } else if (value.isObject()) {
                    QJsonObject actorObj = value.toObject();
                    QString name = actorObj["name"].toString();
                    if (name.isEmpty()) {
                        name = actorObj["enName"].toString();
                    }
                    if (!name.isEmpty()) {
                        actorsList << name;
                    }
                }
                if (actorsList.size() >= 5) break;
            }
            actors = actorsList.join(", ");
        }
    }
    if (actors.isEmpty() && json.contains("cast")) {
        QJsonValue castValue = json["cast"];
        if (castValue.isArray()) {
            QStringList actorsList;
            QJsonArray castArray = castValue.toArray();
            for (const QJsonValue& value : castArray) {
                if (value.isObject()) {
                    QJsonObject castObj = value.toObject();
                    QString name = castObj["name"].toString();
                    if (name.isEmpty()) {
                        name = castObj["character"].toString();
                    }
                    if (!name.isEmpty()) {
                        actorsList << name;
                    }
                }
                if (actorsList.size() >= 5) break;
            }
            actors = actorsList.join(", ");
        }
    }
    int duration = 0;
    if (json.contains("movieLength")) {
        QJsonValue movieLengthValue = json["movieLength"];
        if (movieLengthValue.isDouble()) {
            duration = movieLengthValue.toInt();
        } else if (movieLengthValue.isString()) {
            bool ok;
            duration = movieLengthValue.toString().toInt(&ok);
            if (!ok) duration = 0;
        }
    }
    if (duration == 0 && json.contains("length")) {
        QJsonValue lengthValue = json["length"];
        if (lengthValue.isDouble()) {
            duration = lengthValue.toInt();
        } else if (lengthValue.isString()) {
            bool ok;
            duration = lengthValue.toString().toInt(&ok);
            if (!ok) duration = 0;
        }
    }
    if (duration == 0 && json.contains("duration")) {
        QJsonValue durationValue = json["duration"];
        if (durationValue.isDouble()) {
            duration = durationValue.toInt();
        } else if (durationValue.isString()) {
            bool ok;
            duration = durationValue.toString().toInt(&ok);
            if (!ok) duration = 0;
        }
    }
    if (duration == 0 && json.contains("runtime")) {
        QJsonValue runtimeValue = json["runtime"];
        if (runtimeValue.isDouble()) {
            duration = runtimeValue.toInt();
        } else if (runtimeValue.isString()) {
            bool ok;
            duration = runtimeValue.toString().toInt(&ok);
            if (!ok) duration = 0;
        }
    }
    if (json.contains("seriesLength") && duration == 0) {
        QJsonValue seriesLength = json["seriesLength"];
        if (seriesLength.type() != QJsonValue::Null && seriesLength.type() != QJsonValue::Undefined) {
            int seriesLen = seriesLength.toInt();
            if (seriesLen > 0) {
                duration = seriesLen;
            }
        }
    }
    if (json.contains("totalSeriesLength") && duration == 0) {
        QJsonValue totalSeriesLength = json["totalSeriesLength"];
        if (totalSeriesLength.type() != QJsonValue::Null && totalSeriesLength.type() != QJsonValue::Undefined) {
            int totalLen = totalSeriesLength.toInt();
            if (totalLen > 0) {
                duration = totalLen;
            }
        }
    }
    
    std::vector<std::string> genres;
    for (const QString& genre : genresList) {
        genres.push_back(genre.toStdString());
    }
    
    QString posterPath = "";
    if (id > 0) {
        posterPath = "posters/" + QString::number(id) + ".jpg";
    }
    
    Movie movie(id, name.toStdString(), rating, year, genres, 
                 director.toStdString(), description.toStdString(),
                 posterPath.toStdString(), country.toStdString(), 
                 actors.toStdString(), duration);
    
    return movie;
}

QString KinopoiskAPIClient::extractPosterUrl(const QJsonObject& movieJson) {
    QString posterUrl = "";
    
    if (movieJson.contains("poster")) {
        QJsonValue posterValue = movieJson["poster"];
        if (posterValue.isObject()) {
            QJsonObject poster = posterValue.toObject();
            posterUrl = poster["url"].toString();
            if (posterUrl.isEmpty()) {
                posterUrl = poster["previewUrl"].toString();
            }
            if (posterUrl.isEmpty()) {
                posterUrl = poster["preview"].toString();
            }
            if (posterUrl.isEmpty()) {
                posterUrl = poster["original"].toString();
            }
        } else if (posterValue.isString()) {
            posterUrl = posterValue.toString();
        }
    }
    
    if (posterUrl.isEmpty() && movieJson.contains("posterUrl")) {
        posterUrl = movieJson["posterUrl"].toString();
    }
    
    if (posterUrl.isEmpty() && movieJson.contains("backdrop")) {
        QJsonValue backdropValue = movieJson["backdrop"];
        if (backdropValue.isObject()) {
            QJsonObject backdrop = backdropValue.toObject();
            posterUrl = backdrop["url"].toString();
        } else if (backdropValue.isString()) {
            posterUrl = backdropValue.toString();
        }
    }
    
    return posterUrl;
}

void KinopoiskAPIClient::handleError(QNetworkReply* reply, const QString& defaultMessage) {
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

