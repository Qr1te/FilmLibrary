#ifndef BETA2_MOVIEJSONPARSER_H
#define BETA2_MOVIEJSONPARSER_H

#include "../movie.h"
#include <QJsonObject>
#include <QString>

class MovieJsonParser {
public:
    static Movie parseMovieFromJSON(const QJsonObject& json);
    static QString extractPosterUrl(const QJsonObject& movieJson);
};

#endif // BETA2_MOVIEJSONPARSER_H


