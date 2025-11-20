#ifndef BETA2_MOVIECARDFACTORY_H
#define BETA2_MOVIECARDFACTORY_H

#include <QWidget>
#include <QStringList>
#include <vector>
#include "models/movie.h"

class MovieManager;
class PosterManager;
class QStatusBar;
class QPushButton;

class MovieCardFactory {
public:
    explicit MovieCardFactory(MovieManager* manager, PosterManager* posterManager, QStatusBar* statusBar);
    
    QWidget* createMovieCard(const Movie& movie, QWidget* parent = nullptr);
    
    void setOnFavoritesChanged(const std::function<void()>& callback);
    void setOnCollectionsChanged(const std::function<void()>& callback);
    void setOnMoviesChanged(const std::function<void()>& callback);
    void setOnGenresChanged(const std::function<void()>& callback);
    void setOnShowInfo(const std::function<void(const Movie&)>& callback);

private:
    void handleCollectionException(const std::exception& e) const;
    std::string qStringToStdString(const QString& str) const;
    void handleMoreButtonClicked(const Movie& movie, const QPushButton* moreBtn) const;
    void handleAddToCollection(const Movie& movie) const;
    void handleRemoveFromCollection(const Movie& movie, const QStringList& collectionsWithMovie) const;
    void handleDeleteMovie(const Movie& movie) const;
    
    QPushButton* createPlayButton() const;
    QPushButton* createFavoriteButton(const Movie& movie) const;
    QPushButton* createMoreButton() const;
    QPushButton* createInfoButton() const;
    void setupButtonConnections(const QPushButton* playBtn, QPushButton* favoriteBtn, 
                                const QPushButton* moreBtn, const QPushButton* infoBtn, const Movie& movie) const;
    
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


