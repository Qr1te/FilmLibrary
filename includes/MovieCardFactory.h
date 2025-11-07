#ifndef BETA2_MOVIECARDFACTORY_H
#define BETA2_MOVIECARDFACTORY_H

#include <QWidget>
#include <vector>
#include "movie.h"

class MovieManager;
class PosterManager;
class QStatusBar;

class MovieCardFactory {
public:
    explicit MovieCardFactory(MovieManager* manager, PosterManager* posterManager, QStatusBar* statusBar);
    
    QWidget* createMovieCard(const Movie& movie, QWidget* parent = nullptr);
    
    void setOnFavoritesChanged(std::function<void()> callback);
    void setOnCollectionsChanged(std::function<void()> callback);
    void setOnMoviesChanged(std::function<void()> callback);
    void setOnGenresChanged(std::function<void()> callback);
    void setOnShowInfo(std::function<void(const Movie&)> callback);

private:
    MovieManager* movieManager;
    PosterManager* posterManager;
    QStatusBar* statusBar;
    
    std::function<void()> onFavoritesChanged;
    std::function<void()> onCollectionsChanged;
    std::function<void()> onMoviesChanged;
    std::function<void()> onGenresChanged;
    std::function<void(const Movie&)> onShowInfo;
};

#endif // BETA2_MOVIECARDFACTORY_H


