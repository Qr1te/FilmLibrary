#include "../../includes/parsers/MovieJsonParser.h"
#include <QJsonArray>
#include <QJsonValue>
#include <QStringList>

Movie MovieJsonParser::parseMovieFromJSON(const QJsonObject& json) {
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
                QString personName = person["name"].toString();
                if (personName.isEmpty()) {
                    personName = person["enName"].toString();
                }
                if (personName.isEmpty()) {
                    personName = person["alternativeName"].toString();
                }
                if (!personName.isEmpty()) {
                    director = personName;
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
                    QString countryName = countryObj["name"].toString();
                    if (countryName.isEmpty()) {
                        countryName = countryObj["enName"].toString();
                    }
                    if (!countryName.isEmpty()) {
                        countries << countryName;
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
                    QString countryName = value.toObject()["name"].toString();
                    if (!countryName.isEmpty()) {
                        countries << countryName;
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
                QString actorName = person["name"].toString();
                if (actorName.isEmpty()) {
                    actorName = person["enName"].toString();
                }
                if (actorName.isEmpty()) {
                    actorName = person["alternativeName"].toString();
                }
                if (actorName.isEmpty() && person.contains("names")) {
                    QJsonArray namesArray = person["names"].toArray();
                    if (!namesArray.isEmpty()) {
                        actorName = namesArray[0].toObject()["name"].toString();
                    }
                }
                if (!actorName.isEmpty()) {
                    actorsList << actorName;
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
                    QString actorName = actorObj["name"].toString();
                    if (actorName.isEmpty()) {
                        actorName = actorObj["enName"].toString();
                    }
                    if (!actorName.isEmpty()) {
                        actorsList << actorName;
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
                    QString actorName = castObj["name"].toString();
                    if (actorName.isEmpty()) {
                        actorName = castObj["character"].toString();
                    }
                    if (!actorName.isEmpty()) {
                        actorsList << actorName;
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

QString MovieJsonParser::extractPosterUrl(const QJsonObject& movieJson) {
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


