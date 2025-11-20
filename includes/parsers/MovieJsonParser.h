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
};

#endif // BETA2_MOVIEJSONPARSER_H





