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
    
    qDebug() << "Search URL:" << url.toString();
    
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
    
    qDebug() << "API Response Status Code:" << statusCode;
    qDebug() << "API Response Data:" << data.left(500);
    
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
        qDebug() << "Unexpected API response structure:" << root.keys();
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
    
    // Делаем дополнительный запрос для получения полной информации
    fetchMovieDetails(movieId);
}

void KinopoiskAPIClient::fetchMovieDetails(int movieId) {
    QUrl detailUrl(QString("https://api.kinopoisk.dev/v1.4/movie/%1").arg(movieId));
    QNetworkRequest detailRequest(detailUrl);
    detailRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    detailRequest.setRawHeader("Accept", "application/json");
    detailRequest.setRawHeader("X-API-KEY", apiKey.toUtf8());
    
    qDebug() << "Fetching full movie details from:" << detailUrl.toString();
    
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
            // Объединяем данные: приоритет у детального запроса, но не перезаписываем непустые значения
            for (auto it = detailObj.begin(); it != detailObj.end(); ++it) {
                QString key = it.key();
                QJsonValue detailValue = it.value();
                
                // Для массивов (countries, persons) - объединяем, если исходный пустой
                if (detailValue.isArray()) {
                    QJsonArray detailArray = detailValue.toArray();
                    if (!detailArray.isEmpty()) {
                        if (!fullMovieJson.contains(key) || 
                            !fullMovieJson[key].isArray() || 
                            fullMovieJson[key].toArray().isEmpty()) {
                            fullMovieJson[key] = detailValue;
                        }
                    }
                }
                // Для строк - перезаписываем только если исходная пустая
                else if (detailValue.isString()) {
                    QString detailStr = detailValue.toString();
                    if (!detailStr.isEmpty()) {
                        if (!fullMovieJson.contains(key) || 
                            !fullMovieJson[key].isString() || 
                            fullMovieJson[key].toString().isEmpty()) {
                            fullMovieJson[key] = detailValue;
                        }
                    }
                }
                // Для чисел - перезаписываем только если исходное равно 0
                else if (detailValue.isDouble()) {
                    double detailNum = detailValue.toDouble();
                    if (detailNum != 0.0) {
                        if (!fullMovieJson.contains(key) || 
                            !fullMovieJson[key].isDouble() || 
                            fullMovieJson[key].toDouble() == 0.0) {
                            fullMovieJson[key] = detailValue;
                        }
                    }
                }
                // Для объектов - рекурсивно объединяем
                else if (detailValue.isObject()) {
                    if (!fullMovieJson.contains(key) || !fullMovieJson[key].isObject()) {
                        fullMovieJson[key] = detailValue;
                    } else {
                        QJsonObject existingObj = fullMovieJson[key].toObject();
                        QJsonObject detailObj = detailValue.toObject();
                        for (auto objIt = detailObj.begin(); objIt != detailObj.end(); ++objIt) {
                            if (!existingObj.contains(objIt.key()) || 
                                (existingObj[objIt.key()].isString() && existingObj[objIt.key()].toString().isEmpty())) {
                                existingObj[objIt.key()] = objIt.value();
                            }
                        }
                        fullMovieJson[key] = existingObj;
                    }
                }
                // Для остальных типов - просто добавляем, если отсутствует
                else {
                    if (!fullMovieJson.contains(key)) {
                        fullMovieJson[key] = detailValue;
                    }
                }
            }
            qDebug() << "Full movie details loaded and merged successfully";
            qDebug() << "Countries in merged JSON:" << (fullMovieJson.contains("countries") ? "yes" : "no");
            qDebug() << "Persons in merged JSON:" << (fullMovieJson.contains("persons") ? "yes" : "no");
            qDebug() << "movieLength in merged JSON:" << (fullMovieJson.contains("movieLength") ? "yes" : "no");
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
        qDebug() << "Parsing persons array, size:" << personsArray.size();
        for (const QJsonValue& value : personsArray) {
            QJsonObject person = value.toObject();
            QString profession = person["enProfession"].toString();
            QString professionRu = person["profession"].toString();
            qDebug() << "Person profession (en):" << profession << "profession (ru):" << professionRu;
            
            if (profession == "director" || professionRu == "режиссер" || professionRu.contains("режиссер", Qt::CaseInsensitive)) {
                QString name = person["name"].toString();
                if (name.isEmpty()) {
                    name = person["enName"].toString();
                }
                if (name.isEmpty()) {
                    name = person["alternativeName"].toString();
                }
                qDebug() << "Found director:" << name;
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
    qDebug() << "Final director:" << director;
    
    QString description = json["description"].toString();
    if (description.isEmpty()) {
        description = json["shortDescription"].toString();
    }
    
    QString country = "";
    if (json.contains("countries")) {
        QJsonArray countriesArray = json["countries"].toArray();
        QStringList countries;
        for (const QJsonValue& value : countriesArray) {
            QString countryName = value.toObject()["name"].toString();
            if (!countryName.isEmpty()) {
                countries << countryName;
            }
        }
        country = countries.join(", ");
        qDebug() << "Parsed countries:" << country;
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
                    QString name = value.toObject()["name"].toString();
                    if (!name.isEmpty()) {
                        countries << name;
                    }
                }
            }
            country = countries.join(", ");
        }
        qDebug() << "Parsed country (alternative):" << country;
    }
    
    QString actors = "";
    if (json.contains("persons")) {
        QJsonArray personsArray = json["persons"].toArray();
        QStringList actorsList;
        int count = 0;
        for (const QJsonValue& value : personsArray) {
            QJsonObject person = value.toObject();
            QString profession = person["enProfession"].toString();
            QString professionRu = person["profession"].toString();
            
            bool isActor = (profession == "actor" || 
                           professionRu.contains("актер", Qt::CaseInsensitive) ||
                           professionRu.contains("актриса", Qt::CaseInsensitive) ||
                           professionRu == "актер" || professionRu == "актриса");
            
            if (isActor && count < 5) {
                QString name = person["name"].toString();
                if (name.isEmpty()) {
                    name = person["enName"].toString();
                }
                if (name.isEmpty()) {
                    name = person["alternativeName"].toString();
                }
                if (!name.isEmpty()) {
                    actorsList << name;
                    count++;
                    qDebug() << "Found actor:" << name;
                }
            }
        }
        actors = actorsList.join(", ");
        qDebug() << "Total actors found:" << actorsList.size();
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
                    QString name = value.toObject()["name"].toString();
                    if (!name.isEmpty()) {
                        actorsList << name;
                    }
                }
                if (actorsList.size() >= 5) break;
            }
            actors = actorsList.join(", ");
        }
    }
    qDebug() << "Final actors:" << actors;
    
    int duration = 0;
    if (json.contains("movieLength")) {
        duration = json["movieLength"].toInt();
    }
    if (duration == 0 && json.contains("length")) {
        duration = json["length"].toInt();
    }
    if (duration == 0 && json.contains("duration")) {
        duration = json["duration"].toInt();
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
    qDebug() << "Parsed duration:" << duration;
    
    std::vector<std::string> genres;
    for (const QString& genre : genresList) {
        genres.push_back(genre.toStdString());
    }
    
    QString posterPath = "";
    if (id > 0) {
        posterPath = "posters/" + QString::number(id) + ".jpg";
    }
    
    qDebug() << "=== Final parsed movie data ===";
    qDebug() << "ID:" << id;
    qDebug() << "Title:" << name;
    qDebug() << "Country:" << country;
    qDebug() << "Actors:" << actors;
    qDebug() << "Duration:" << duration;
    qDebug() << "Director:" << director;
    
    Movie movie(id, name.toStdString(), rating, year, genres, 
                 director.toStdString(), description.toStdString(),
                 posterPath.toStdString(), country.toStdString(), 
                 actors.toStdString(), duration);
    
    qDebug() << "Movie created. Country from getter:" << QString::fromStdString(movie.getCountry());
    qDebug() << "Movie created. Actors from getter:" << QString::fromStdString(movie.getActors());
    qDebug() << "Movie created. Duration from getter:" << movie.getDuration();
    
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

