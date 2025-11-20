#ifndef BETA2_MAINWINDOW_H
#define BETA2_MAINWINDOW_H

#include <QMainWindow>
#include <QScopedPointer>
#include <QStandardItemModel>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPixmap>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QLineEdit>
#include <QComboBox>
#include <QTabWidget>
#include <QScrollArea>
#include <QGridLayout>
#include <QToolBar>
#include <QStatusBar>
#include <QAction>
#include <QSpacerItem>
#include "managers/MovieManager.h"
#include "api/KinopoiskAPIClient.h"
#include "managers/PosterManager.h"
#include "ui/MovieCardFactory.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;

private slots:
    void handleSearch();
    void handleAddMovie();
    void handleSortByRating();
    void handleShowTopN();
    void handleHome();
    void handleAllSelectionChanged() const;
    void handleFavSelectionChanged() const;
    
    void onMovieSearchSuccess(const Movie& movie, const QString& posterUrl);
    void onPosterDownloadFinished(const Movie& movie, const QString& posterDir, const QString& sep, int movieId, bool success);
    void handleCreateCollection();
    void handleDeleteCollection();
    void handleCollectionChanged();

private:
    void setupUI();
    void setupModelsAndViews();
    void populateAllMovies(const std::vector<Movie>& movies);
    void populateFavorites();
    void populateCollections();
    void populateGenres();
    int selectedMovieIdFromAll() const;
    int selectedMovieIdFromFavorites() const;
    void showMovieInfo(const Movie& movie);
    void clearGridLayout(QGridLayout* layout);
    void populateGridLayoutWithMovies(QGridLayout* layout, QWidget* parent, const std::vector<Movie>& movies);
    std::string qStringToStdString(const QString& str) const;
    QString stdStringToQString(const std::string& str) const;
    QString findPosterDirectory() const;
    void handleAddMovieToFile(const Movie& movie);
    std::vector<Movie> findMovieIntersection(const std::vector<Movie>& movies1, const std::vector<Movie>& movies2) const;

private:
    struct SearchUI {
        QHBoxLayout* searchLayout;
        QPushButton* addMovieButton;
        QLineEdit* searchLineEdit;
        QComboBox* genreComboBox;
        QPushButton* searchButton;
    };

    struct TabAllUI {
        QWidget* tabAll;
        QVBoxLayout* verticalLayoutAll;
        QScrollArea* scrollAreaAll;
        QWidget* scrollAreaWidgetContentsAll;
        QGridLayout* gridLayoutMovies;
    };

    struct TabFavoritesUI {
        QWidget* tabFavorites;
        QVBoxLayout* verticalLayoutFav;
        QScrollArea* scrollAreaFavorites;
        QWidget* scrollAreaWidgetContentsFavorites;
        QGridLayout* gridLayoutFavorites;
    };

    struct TabCollectionsUI {
        QWidget* tabCollections;
        QVBoxLayout* verticalLayoutCollections;
        QHBoxLayout* horizontalLayoutCollectionSelector;
        QLabel* collectionLabel;
        QComboBox* collectionComboBox;
        QPushButton* createCollectionButton;
        QPushButton* manageCollectionsButton;
        QScrollArea* scrollAreaCollections;
        QWidget* scrollAreaWidgetContentsCollections;
        QGridLayout* gridLayoutCollections;
    };

    struct ToolBarUI {
        QToolBar* mainToolBar;
        QAction* actionHome;
        QAction* actionSortByRating;
        QAction* actionTopN;
    };

    struct Managers {
        QNetworkAccessManager* networkManager;
        KinopoiskAPIClient* apiClient;
        PosterManager* posterManager;
        MovieCardFactory* cardFactory;
    };

    QWidget* centralWidget;
    QVBoxLayout* mainLayout;
    QTabWidget* tabWidget;
    QStatusBar* statusbar;
    MovieManager manager;
    bool isSortedByRating = false;
    
    SearchUI searchUI;
    TabAllUI tabAllUI;
    TabFavoritesUI tabFavoritesUI;
    TabCollectionsUI tabCollectionsUI;
    ToolBarUI toolBarUI;
    Managers managers;
};

#endif



