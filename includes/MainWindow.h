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
#include "MovieManager.h"
#include "KinopoiskAPIClient.h"
#include "PosterManager.h"
#include "MovieCardFactory.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

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
    void handleManageCollections();
    void handleCollectionChanged();

private:
    void setupModelsAndViews();
    void populateAllMovies(const std::vector<Movie>& movies);
    void populateFavorites();
    void populateCollections();
    void populateGenres();
    int selectedMovieIdFromAll() const;
    int selectedMovieIdFromFavorites() const;
    void showMovieInfo(const Movie& movie);

private:
    Ui::MainWindow* ui;
    MovieManager manager;
    bool isSortedByRating;
    QNetworkAccessManager* networkManager;
    KinopoiskAPIClient* apiClient;
    PosterManager* posterManager;
    MovieCardFactory* cardFactory;
};

#endif



