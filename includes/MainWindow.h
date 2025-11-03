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
#include "MovieManager.h"

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
    void handleReset();
    void handleCreateSample();
    void handleSortByRating();
    void handleShowTopN();
    void handleRefresh();
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
    void updateDetailsFromSelection(int movieId);
    void updateStats();
    QWidget* createMovieCard(const Movie& movie, QWidget* parent = nullptr);
    int selectedMovieIdFromAll() const;
    int selectedMovieIdFromFavorites() const;
    void loadPosterToLabel(QLabel* label, const Movie& movie);

private:
    Ui::MainWindow* ui;
    MovieManager manager;
    // Модели больше не используются, так как перешли на карточки
    // QScopedPointer<QStandardItemModel> allModel;
    // QScopedPointer<QStandardItemModel> favModel;
};

#endif // BETA2_MAINWINDOW_H



