#include "parsers/MovieJsonParser.h"
#include <QJsonArray>
#include <QJsonValue>
#include <QStringList>

double MovieJsonParser::extractRating(const QJsonObject& json) {
    if (!json.contains("rating")) {
        return 0.0;
    }
    
    QJsonObject ratingObj = json["rating"].toObject();
    double rating = ratingObj["kp"].toDouble();
    if (rating == 0.0) {
        rating = ratingObj["imdb"].toDouble();
    }
    return rating;
}

QStringList MovieJsonParser::extractGenres(const QJsonObject& json) {
    QStringList genresList;
    if (!json.contains("genres")) {
        return genresList;
    }
    
    QJsonArray genresArray = json["genres"].toArray();
    for (const QJsonValue& value : genresArray) {
        QString genreName = value.toObject()["name"].toString();
        if (!genreName.isEmpty()) {
            genresList << genreName;
        }
    }
    return genresList;
}

QString MovieJsonParser::extractPersonName(const QJsonObject& person) {
    QString personName = person["name"].toString();
    if (!personName.isEmpty()) {
        return personName;
    }
    
    personName = person["enName"].toString();
    if (!personName.isEmpty()) {
        return personName;
    }
    
    return person["alternativeName"].toString();
}

QString MovieJsonParser::extractDirector(const QJsonObject& json) {
    if (json.contains("persons")) {
        QJsonArray personsArray = json["persons"].toArray();
        for (const QJsonValue& value : personsArray) {
            QJsonObject person = value.toObject();
            QString profession = person["enProfession"].toString();
            
            if (QString professionRu = person["profession"].toString();
                !(profession == "director" || 
                  professionRu == "режиссер" || 
                  professionRu.contains("режиссер", Qt::CaseInsensitive))) {
                continue;
            }
            
            QString personName = extractPersonName(person);
            if (!personName.isEmpty()) {
                return personName;
            }
        }
    }
    
    if (!json.contains("director")) {
        return "";
    }
    
    QJsonValue directorValue = json["director"];
    if (directorValue.isString()) {
        return directorValue.toString();
    }
    
    if (directorValue.isObject()) {
        return directorValue.toObject()["name"].toString();
    }
    
    return "";
}

QString MovieJsonParser::extractCountryName(const QJsonValue& countryValue) {
    if (countryValue.isString()) {
        return countryValue.toString();
    }
    
    if (!countryValue.isObject()) {
        return "";
    }
    
    QJsonObject countryObj = countryValue.toObject();
    QString countryName = countryObj["name"].toString();
    if (!countryName.isEmpty()) {
        return countryName;
    }
    
    countryName = countryObj["enName"].toString();
    if (!countryName.isEmpty()) {
        return countryName;
    }
    
    return countryObj["alternativeName"].toString();
}

QStringList MovieJsonParser::extractCountriesFromArray(const QJsonArray& countriesArray) {
    QStringList countries;
    for (const QJsonValue& value : countriesArray) {
        QString countryName = extractCountryName(value);
        if (!countryName.isEmpty()) {
            countries << countryName;
        }
    }
    return countries;
}

QStringList MovieJsonParser::extractCountriesFromValue(const QJsonValue& countryValue) {
    QStringList countries;
    
    if (countryValue.isString()) {
        countries << countryValue.toString();
        return countries;
    }
    
    if (countryValue.isArray()) {
        return extractCountriesFromArray(countryValue.toArray());
    }
    
    if (countryValue.isObject()) {
        QString countryName = extractCountryName(countryValue);
        if (!countryName.isEmpty()) {
            countries << countryName;
        }
    }
    
    return countries;
}

QString MovieJsonParser::extractCountry(const QJsonObject& json) {
    QStringList countries;
    
    if (json.contains("countries")) {
        QJsonArray countriesArray = json["countries"].toArray();
        countries = extractCountriesFromArray(countriesArray);
    }
    
    if (countries.isEmpty() && json.contains("country")) {
        QJsonValue countryValue = json["country"];
        countries = extractCountriesFromValue(countryValue);
    }
    
    if (countries.isEmpty() && json.contains("productionCountries")) {
        QJsonValue productionCountries = json["productionCountries"];
        if (productionCountries.isArray()) {
            QJsonArray countriesArray = productionCountries.toArray();
            countries = extractCountriesFromArray(countriesArray);
        }
    }
    
    return countries.join(", ");
}

QString MovieJsonParser::extractActorName(const QJsonObject& actorObj) {
    QString actorName = actorObj["name"].toString();
    if (!actorName.isEmpty()) {
        return actorName;
    }
    
    actorName = actorObj["enName"].toString();
    if (!actorName.isEmpty()) {
        return actorName;
    }
    
    actorName = actorObj["alternativeName"].toString();
    if (!actorName.isEmpty()) {
        return actorName;
    }
    
    if (actorObj.contains("names")) {
        QJsonArray namesArray = actorObj["names"].toArray();
        if (!namesArray.isEmpty()) {
            return namesArray[0].toObject()["name"].toString();
        }
    }
    
    return "";
}

bool MovieJsonParser::isActorProfession(const QString& profession, const QString& professionRu) {
    if (profession == "actor" || profession == "актер") {
        return true;
    }
    
    if (professionRu.contains("актер", Qt::CaseInsensitive) ||
        professionRu.contains("актриса", Qt::CaseInsensitive) ||
        professionRu == "актер" || 
        professionRu == "актриса" ||
        professionRu.contains("actor", Qt::CaseInsensitive)) {
        return true;
    }
    
    return false;
}

QStringList MovieJsonParser::extractActorsFromPersons(const QJsonObject& json) {
    QStringList actorsList;
    int count = 0;
    
    if (!json.contains("persons")) {
        return actorsList;
    }
    
    QJsonArray personsArray = json["persons"].toArray();
    for (const QJsonValue& value : personsArray) {
        if (!value.isObject() || count >= 5) {
            continue;
        }
        
        QJsonObject person = value.toObject();
        QString profession = person["enProfession"].toString();
        
        if (QString professionRu = person["profession"].toString();
            !isActorProfession(profession, professionRu)) {
            continue;
        }
        
        QString actorName = extractActorName(person);
        if (!actorName.isEmpty()) {
            actorsList << actorName;
            count++;
        }
    }
    
    return actorsList;
}

QStringList MovieJsonParser::extractActorsFromArray(const QJsonArray& actorsArray, int maxCount) {
    QStringList actorsList;
    
    for (const QJsonValue& value : actorsArray) {
        if (actorsList.size() >= maxCount) {
            break;
        }
        
        if (value.isString()) {
            actorsList << value.toString();
            continue;
        }
        
        if (!value.isObject()) {
            continue;
        }
        
        QString actorName = extractActorName(value.toObject());
        if (!actorName.isEmpty()) {
            actorsList << actorName;
        }
    }
    
    return actorsList;
}

QStringList MovieJsonParser::extractActorsFromCast(const QJsonArray& castArray, int maxCount) {
    QStringList actorsList;
    
    for (const QJsonValue& value : castArray) {
        if (actorsList.size() >= maxCount) {
            break;
        }
        
        if (!value.isObject()) {
            continue;
        }
        
        QJsonObject castObj = value.toObject();
        QString actorName = castObj["name"].toString();
        if (actorName.isEmpty()) {
            actorName = castObj["character"].toString();
        }
        if (!actorName.isEmpty()) {
            actorsList << actorName;
        }
    }
    
    return actorsList;
}

QString MovieJsonParser::extractActors(const QJsonObject& json) {
    QStringList actorsList = extractActorsFromPersons(json);
    
    if (actorsList.isEmpty() && json.contains("actors")) {
        QJsonValue actorsValue = json["actors"];
        if (actorsValue.isString()) {
            return actorsValue.toString();
        }
        
        if (actorsValue.isArray()) {
            actorsList = extractActorsFromArray(actorsValue.toArray(), 5);
        }
    }
    
    if (actorsList.isEmpty() && json.contains("cast")) {
        QJsonValue castValue = json["cast"];
        if (castValue.isArray()) {
            actorsList = extractActorsFromCast(castValue.toArray(), 5);
        }
    }
    
    return actorsList.join(", ");
}

int MovieJsonParser::parseDurationValue(const QJsonValue& value) {
    if (value.isDouble()) {
        return value.toInt();
    }
    
    if (value.isString()) {
        bool ok;
        int duration = value.toString().toInt(&ok);
        return ok ? duration : 0;
    }
    
    return 0;
}

int MovieJsonParser::extractDurationFromValue(const QJsonValue& value, const QString& /*key*/) {
    if (value.type() == QJsonValue::Null || value.type() == QJsonValue::Undefined) {
        return 0;
    }
    
    int duration = value.toInt();
    return duration > 0 ? duration : 0;
}

int MovieJsonParser::extractDuration(const QJsonObject& json) {
    const QStringList durationKeys = {"movieLength", "length", "duration", "runtime"};
    
    for (const QString& key : durationKeys) {
        if (!json.contains(key)) {
            continue;
        }
        
        int duration = parseDurationValue(json[key]);
        if (duration > 0) {
            return duration;
        }
    }
    
    if (json.contains("seriesLength")) {
        int duration = extractDurationFromValue(json["seriesLength"], "seriesLength");
        if (duration > 0) {
            return duration;
        }
    }
    
    if (json.contains("totalSeriesLength")) {
        int duration = extractDurationFromValue(json["totalSeriesLength"], "totalSeriesLength");
        if (duration > 0) {
            return duration;
        }
    }
    
    return 0;
}

Movie MovieJsonParser::parseMovieFromJSON(const QJsonObject& json) {
    int id = json["id"].toInt();
    
    QString name = json["name"].toString();
    if (name.isEmpty()) {
        name = json["alternativeName"].toString();
    }
    
    double rating = extractRating(json);
    int year = json["year"].toInt();
    QStringList genresList = extractGenres(json);
    QString director = extractDirector(json);
    
    QString description = json["description"].toString();
    if (description.isEmpty()) {
        description = json["shortDescription"].toString();
    }
    
    QString country = extractCountry(json);
    QString actors = extractActors(json);
    int duration = extractDuration(json);
    
    std::vector<std::string> genres;
    for (const QString& genre : genresList) {
        genres.push_back(genre.toStdString());
    }
    
    QString posterPath = "";
    if (id > 0) {
        posterPath = "posters/" + QString::number(id) + ".jpg";
    }
    
    return Movie(id, name.toStdString(), rating, year, genres, 
                 director.toStdString(), description.toStdString(),
                 posterPath.toStdString(), country.toStdString(), 
                 actors.toStdString(), duration);
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


