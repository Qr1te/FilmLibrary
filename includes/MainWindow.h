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
#include "MovieManager.h"
#include "KinopoiskAPIClient.h"
#include "PosterManager.h"
#include "MovieCardFactory.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    void handleSearch();
    void handleAddMovie();
    void handleSortByRating();
    void handleShowTopN();
    void handleHome();
    void handleAllSelectionChanged();
    void handleFavSelectionChanged();
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

private:

    QWidget* centralWidget;
    QVBoxLayout* mainLayout;
    

    QHBoxLayout* searchLayout;
    QPushButton* addMovieButton;
    QLineEdit* searchLineEdit;
    QComboBox* genreComboBox;
    QPushButton* searchButton;
    

    QTabWidget* tabWidget;
    

    QWidget* tabAll;
    QVBoxLayout* verticalLayoutAll;
    QScrollArea* scrollAreaAll;
    QWidget* scrollAreaWidgetContentsAll;
    QGridLayout* gridLayoutMovies;
    

    QWidget* tabFavorites;
    QVBoxLayout* verticalLayoutFav;
    QScrollArea* scrollAreaFavorites;
    QWidget* scrollAreaWidgetContentsFavorites;
    QGridLayout* gridLayoutFavorites;
    

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
    

    QToolBar* mainToolBar;
    QAction* actionHome;
    QAction* actionSortByRating;
    QAction* actionTopN;
    

    QStatusBar* statusbar;
    

    MovieManager manager;
    bool isSortedByRating;
    QNetworkAccessManager* networkManager;
    KinopoiskAPIClient* apiClient;
    PosterManager* posterManager;
    MovieCardFactory* cardFactory;
};

#endif



