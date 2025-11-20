#ifndef BETA2_MOVIEJSONPARSER_H
#define BETA2_MOVIEJSONPARSER_H

#include "../movie.h"
#include <QJsonObject>
#include <QString>

class MovieJsonParser {
public:
    static Movie parseMovieFromJSON(const QJsonObject& json);
    static QString extractPosterUrl(const QJsonObject& movieJson);

private:
    // Helper functions to reduce complexity
    static double extractRating(const QJsonObject& json);
    static QStringList extractGenres(const QJsonObject& json);
    static QString extractDirector(const QJsonObject& json);
    static QString extractCountry(const QJsonObject& json);
    static QString extractActors(const QJsonObject& json);
    static int extractDuration(const QJsonObject& json);
    
    // Helper functions for nested parsing
    static QString extractPersonName(const QJsonObject& person);
    static QString extractCountryName(const QJsonValue& countryValue);
    static QString extractActorName(const QJsonObject& actorObj);
    static int parseDurationValue(const QJsonValue& value);
    
    // Helper functions to reduce complexity further
    static QStringList extractCountriesFromArray(const QJsonArray& countriesArray);
    static QStringList extractCountriesFromValue(const QJsonValue& countryValue);
    static QStringList extractActorsFromPersons(const QJsonObject& json);
    static QStringList extractActorsFromArray(const QJsonArray& actorsArray, int maxCount);
    static QStringList extractActorsFromCast(const QJsonArray& castArray, int maxCount);
    static bool isActorProfession(const QString& profession, const QString& professionRu);
    static int extractDurationFromValue(const QJsonValue& value, const QString& /*key*/);
};

#endif // BETA2_MOVIEJSONPARSER_H





