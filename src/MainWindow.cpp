#include "../includes/MainWindow.h"
#include "ui_MainWindow.h"

#include <QInputDialog>
#include <QMessageBox>
#include <QHeaderView>
#include <QSet>
#include <QFile>
#include <QScrollArea>
#include <QGridLayout>
#include <QApplication>
#include <QDir>
#include <QMenu>
#include <QFileInfo>
#include <QDebug>
#include <QImage>
#include <QIODevice>
#include <QImageReader>
#include <QDesktopServices>
#include <QUrl>
#include <QProcess>
#include <QDialog>
#include <QTextEdit>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrlQuery>
#include <QUrl>
#include <QThread>
#include <functional>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), isSortedByRating(false) {
    ui->setupUi(this);

    networkManager = new QNetworkAccessManager(this);
    apiClient = new KinopoiskAPIClient(this);
    posterManager = new PosterManager(this);
    posterManager->setNetworkManager(networkManager);
    cardFactory = new MovieCardFactory(&manager, posterManager, ui->statusbar);
    
    // Настройка callbacks для cardFactory
    cardFactory->setOnFavoritesChanged([this]() { populateFavorites(); });
    cardFactory->setOnCollectionsChanged([this]() { populateCollections(); handleCollectionChanged(); });
    cardFactory->setOnMoviesChanged([this]() { populateAllMovies(manager.getAllMovies()); });
    cardFactory->setOnGenresChanged([this]() { populateGenres(); });
    cardFactory->setOnShowInfo([this](const Movie& movie) { showMovieInfo(movie); });

    setupModelsAndViews();
    populateGenres();
    
    populateAllMovies(manager.getAllMovies());
    populateFavorites();
    populateCollections();

    connect(ui->searchButton, &QPushButton::clicked, this, &MainWindow::handleSearch);
    connect(ui->addMovieButton, &QPushButton::clicked, this, &MainWindow::handleAddMovie);

    connect(ui->actionSortByRating, &QAction::triggered, this, &MainWindow::handleSortByRating);
    connect(ui->actionTopN, &QAction::triggered, this, &MainWindow::handleShowTopN);
    connect(ui->actionHome, &QAction::triggered, this, &MainWindow::handleHome);
    connect(ui->createCollectionButton, &QPushButton::clicked, this, &MainWindow::handleCreateCollection);
    connect(ui->manageCollectionsButton, &QPushButton::clicked, this, &MainWindow::handleManageCollections);
    connect(ui->collectionComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::handleCollectionChanged);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::setupModelsAndViews() {
    setStyleSheet(
        "QMainWindow { background-color: #2b2b2b; }"
        "QWidget { background-color: #2b2b2b; color: #e0e0e0; }"
        "QLabel { color: #e0e0e0; background-color: transparent; }"
        "QLineEdit { background-color: #3a3a3a; color: #e0e0e0; border: 1px solid #555; padding: 5px; border-radius: 4px; }"
        "QComboBox { background-color: #3a3a3a; color: #e0e0e0; border: 1px solid #555; padding: 5px; border-radius: 4px; }"
        "QPushButton { background-color: #ff6b35; color: white; border: none; padding: 8px; border-radius: 4px; font-weight: bold; }"
        "QPushButton:hover { background-color: #ff8555; }"
        "QPushButton:pressed { background-color: #e55a2b; }"
        "QTabWidget::pane { border: 1px solid #555; background-color: #2b2b2b; }"
        "QTabBar::tab { background-color: #3a3a3a; color: #e0e0e0; padding: 8px 16px; border-top-left-radius: 4px; border-top-right-radius: 4px; }"
        "QTabBar::tab:selected { background-color: #ff6b35; color: white; }"
        "QTabBar::tab:hover { background-color: #4a4a4a; }"
        "QScrollArea { background-color: #2b2b2b; border: none; }"
        "QTextEdit { background-color: #3a3a3a; color: #e0e0e0; border: 1px solid #555; border-radius: 4px; font-size: 11pt; line-height: 1.4; }"
        "QStatusBar { background-color: #1e1e1e; color: #e0e0e0; }"
        "QToolBar { background-color: #1e1e1e; border: none; }"
        "QLabel[name=\"titleValue\"] { font-size: 16pt; font-weight: bold; color: #ff6b35; padding: 5px 0px; }"
        "QLabel[name=\"titleLabel\"], QLabel[name=\"ratingLabel\"], QLabel[name=\"yearLabel\"], QLabel[name=\"genreLabel\"], QLabel[name=\"directorLabel\"] { font-size: 11pt; font-weight: bold; color: #bbb; padding: 3px 0px; }"
        "QLabel[name=\"ratingValue\"] { font-size: 14pt; font-weight: bold; color: #46d369; padding: 3px 0px; }"
        "QLabel[name=\"yearValue\"], QLabel[name=\"genreValue\"], QLabel[name=\"directorValue\"] { font-size: 11pt; color: #e0e0e0; padding: 3px 0px; }"
        "QLabel[name=\"posterLabel\"] { padding: 10px; }"
    );
}

void MainWindow::populateGenres() {
    ui->genreComboBox->clear();
    ui->genreComboBox->addItem("");
    QSet<QString> genres;
    for (const auto& m : manager.getAllMovies()) {
        const auto& movieGenres = m.getGenres();
        for (const auto& genre : movieGenres) {
            genres.insert(QString::fromStdString(genre));
        }
    }
    QStringList sorted = QStringList(genres.begin(), genres.end());
    sorted.sort(Qt::CaseInsensitive);
    ui->genreComboBox->addItems(sorted);
}

void MainWindow::showMovieInfo(const Movie& movie) {
        QDialog* dialog = new QDialog(this);
    dialog->setWindowTitle("Информация о фильме");
        dialog->setMinimumSize(1100, 750);
        dialog->setStyleSheet(
            "QDialog { background-color: #2b2b2b; }"
            "QLabel { color: #e0e0e0; background-color: transparent; }"
            "QTextEdit { background-color: #3a3a3a; color: #e0e0e0; border: 1px solid #555; border-radius: 4px; }"
            "QPushButton { background-color: #ff6b35; color: white; border: none; padding: 8px 20px; border-radius: 4px; font-weight: bold; }"
            "QPushButton:hover { background-color: #ff8555; }"
        );
        
        QHBoxLayout* mainLayout = new QHBoxLayout(dialog);
        mainLayout->setSpacing(20);
        mainLayout->setContentsMargins(20, 20, 20, 20);
        
        QLabel* posterLabel = new QLabel();
        posterLabel->setMinimumSize(450, 675);
        posterLabel->setMaximumSize(450, 675);
        posterLabel->setAlignment(Qt::AlignCenter);
        posterLabel->setStyleSheet("border: 1px solid #555; border-radius: 4px; background-color: #3a3a3a;");
    posterManager->loadPosterToLabel(posterLabel, movie);
        mainLayout->addWidget(posterLabel);
        
        QVBoxLayout* infoLayout = new QVBoxLayout();
        infoLayout->setSpacing(10);
        
        QLabel* titleLabel = new QLabel(QString::fromStdString(movie.getTitle()));
        titleLabel->setStyleSheet("font-size: 20pt; font-weight: bold; color: #ff6b35;");
        titleLabel->setWordWrap(true);
        infoLayout->addWidget(titleLabel);
        
    QLabel* ratingLabel = new QLabel(QString("Рейтинг: %1").arg(movie.getRating(), 0, 'f', 1));
        ratingLabel->setStyleSheet("font-size: 14pt; font-weight: bold; color: #46d369;");
        infoLayout->addWidget(ratingLabel);
        
    QLabel* yearLabel = new QLabel(QString("Год: %1").arg(movie.getYear()));
        yearLabel->setStyleSheet("font-size: 12pt; color: #e0e0e0;");
        infoLayout->addWidget(yearLabel);
        
    QLabel* genreLabel = new QLabel(QString("Жанр: %1").arg(QString::fromStdString(movie.getGenreString())));
        genreLabel->setStyleSheet("font-size: 12pt; color: #e0e0e0;");
        genreLabel->setWordWrap(true);
        infoLayout->addWidget(genreLabel);
        
    QString directorText = QString::fromStdString(movie.getDirector());
    if (directorText.isEmpty()) {
        directorText = "Не указано";
    }
    QLabel* directorLabel = new QLabel(QString("Режиссер: %1").arg(directorText));
        directorLabel->setStyleSheet("font-size: 12pt; color: #e0e0e0;");
        directorLabel->setWordWrap(true);
        infoLayout->addWidget(directorLabel);
        
    QString countryText = QString::fromStdString(movie.getCountry());
    if (countryText.isEmpty()) {
        countryText = "Не указано";
    }
    QLabel* countryLabel = new QLabel(QString("Страна: %1").arg(countryText));
        countryLabel->setStyleSheet("font-size: 12pt; color: #e0e0e0;");
        countryLabel->setWordWrap(true);
        infoLayout->addWidget(countryLabel);
        
    QString actorsText = QString::fromStdString(movie.getActors());
    if (actorsText.isEmpty()) {
        actorsText = "Не указано";
    }
    QLabel* actorsLabel = new QLabel(QString("Актеры: %1").arg(actorsText));
        actorsLabel->setStyleSheet("font-size: 12pt; color: #e0e0e0;");
        actorsLabel->setWordWrap(true);
        infoLayout->addWidget(actorsLabel);
        
    QString durationText = QString("%1 мин").arg(movie.getDuration());
    QLabel* durationLabel = new QLabel(QString("Длительность: %1").arg(durationText));
        durationLabel->setStyleSheet("font-size: 12pt; color: #e0e0e0;");
        infoLayout->addWidget(durationLabel);
        
    QLabel* descTitleLabel = new QLabel("Описание:");
        descTitleLabel->setStyleSheet("font-size: 12pt; font-weight: bold; color: #bbb; margin-top: 10px;");
        infoLayout->addWidget(descTitleLabel);
        
        QTextEdit* descriptionText = new QTextEdit();
        descriptionText->setPlainText(QString::fromStdString(movie.getDescription()));
        descriptionText->setReadOnly(true);
        descriptionText->setMaximumHeight(150);
        infoLayout->addWidget(descriptionText);
        
        infoLayout->addStretch();
        
    QPushButton* closeBtn = new QPushButton("ОК");
        closeBtn->setCursor(Qt::PointingHandCursor);
        connect(closeBtn, &QPushButton::clicked, dialog, &QDialog::accept);
        infoLayout->addWidget(closeBtn, 0, Qt::AlignRight);
        
        mainLayout->addLayout(infoLayout);
        
        dialog->exec();
        delete dialog;
}

// УДАЛЕНО: createMovieCard - перенесено в MovieCardFactory
// УДАЛЕНО: loadPosterToLabel - перенесено в PosterManager
// УДАЛЕНО: loadPosterToLabelByTitle - перенесено в PosterManager
// УДАЛЕНО: searchMovieAPI - перенесено в KinopoiskAPIClient
// УДАЛЕНО: processMovieData - перенесено в KinopoiskAPIClient
// УДАЛЕНО: parseMovieFromJSON - перенесено в KinopoiskAPIClient
// УДАЛЕНО: downloadPoster - перенесено в PosterManager

void MainWindow::populateAllMovies(const std::vector<Movie>& movies) {
    if (!ui->gridLayoutMovies) {
        return;
    }
    
    QLayoutItem* item;
    while ((item = ui->gridLayoutMovies->takeAt(0)) != nullptr) {
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }

    const int columns = 4;
    int row = 0, col = 0;
    
    for (const auto& movie : movies) {
        QWidget* card = cardFactory->createMovieCard(movie, ui->scrollAreaWidgetContentsAll);
        if (card) {
            ui->gridLayoutMovies->addWidget(card, row, col);
            
            col++;
            if (col >= columns) {
                col = 0;
        row++;
            }
        }
    }
}

void MainWindow::populateFavorites() {
    if (!ui->gridLayoutFavorites) {
        return;
    }
    
    auto favs = manager.getFavoriteMovies();
    QLayoutItem* item;
    while ((item = ui->gridLayoutFavorites->takeAt(0)) != nullptr) {
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }

    const int columns = 4;
    int row = 0, col = 0;
    
    for (const auto& movie : favs) {
        QWidget* card = cardFactory->createMovieCard(movie, ui->scrollAreaWidgetContentsFavorites);
        if (card) {
            ui->gridLayoutFavorites->addWidget(card, row, col);
            
            col++;
            if (col >= columns) {
                col = 0;
        row++;
            }
        }
    }
}

void MainWindow::populateCollections() {
    ui->collectionComboBox->clear();
    auto collectionNames = manager.getAllCollectionNames();
    
    for (const auto& name : collectionNames) {
        ui->collectionComboBox->addItem(QString::fromUtf8(name.c_str(), name.length()));
    }
    
    if (ui->collectionComboBox->count() > 0) {
        handleCollectionChanged();
    } else {
        if (ui->gridLayoutCollections) {
            QLayoutItem* item;
            while ((item = ui->gridLayoutCollections->takeAt(0)) != nullptr) {
                if (item->widget()) {
                    item->widget()->deleteLater();
                }
                delete item;
            }
        }
    }
}

void MainWindow::handleCollectionChanged() {
    if (!ui->gridLayoutCollections) {
        return;
    }
    QString collectionName = ui->collectionComboBox->currentText();
    if (collectionName.isEmpty()) {
        QLayoutItem* item;
        while ((item = ui->gridLayoutCollections->takeAt(0)) != nullptr) {
            if (item->widget()) {
                item->widget()->deleteLater();
            }
            delete item;
        }
        return;
    }
    
    auto* collManager = manager.getCollectionManager();
    if (!collManager) {
        return;
    }
    
    QByteArray utf8Name = collectionName.toUtf8();
    MovieCollection* collection = collManager->getCollection(std::string(utf8Name.constData(), utf8Name.length()));
    if (!collection) {
        return;
    }
    
    auto movies = collection->getMovies();
    
    QLayoutItem* item;
    while ((item = ui->gridLayoutCollections->takeAt(0)) != nullptr) {
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }

    const int columns = 4;
    int row = 0, col = 0;
    
    for (const auto& movie : movies) {
        QWidget* card = cardFactory->createMovieCard(movie, ui->scrollAreaWidgetContentsCollections);
        if (card) {
            ui->gridLayoutCollections->addWidget(card, row, col);
            
            col++;
            if (col >= columns) {
                col = 0;
                row++;
            }
        }
    }
}

// УДАЛЕНО: loadPosterToLabel - перенесено в PosterManager


void MainWindow::handleSearch() {
    const QString title = ui->searchLineEdit->text().trimmed();
    const QString genre = ui->genreComboBox->currentText().trimmed();

    // Search in local movies by title and/or genre
    try {
        std::vector<Movie> filtered = manager.getAllMovies();
        
        // Filter by title if provided
        if (!title.isEmpty()) {
            filtered = manager.searchByTitleResults(title.toStdString());
        }
        
        // Filter by genre if provided
        if (!genre.isEmpty()) {
            std::vector<Movie> genreFiltered = manager.searchByGenreResults(genre.toStdString());
            
            // If title was also provided, intersect the results
            if (!title.isEmpty()) {
                std::vector<Movie> intersection;
                for (const auto& movie : filtered) {
                    for (const auto& genreMovie : genreFiltered) {
                        if (movie.getId() == genreMovie.getId()) {
                            intersection.push_back(movie);
                            break;
                        }
                    }
                }
                filtered = intersection;
            } else {
                filtered = genreFiltered;
            }
        }
        
        populateAllMovies(filtered);
        ui->statusbar->showMessage(QString("Found %1 movie(s)").arg(filtered.size()), 2000);
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Search error", e.what());
    }
}

void MainWindow::handleAddMovie() {
    const QString title = ui->searchLineEdit->text().trimmed();
    
    if (title.isEmpty()) {
        QMessageBox::information(this, "Добавить фильм", "Пожалуйста, введите название фильма для поиска");
        return;
    }
    
    // Search via API to add new movie
    apiClient->searchMovie(title, 
        [this](const Movie& movie, const QString& posterUrl) {
        if (movie.getId() == 0) {
            ui->statusbar->showMessage("Фильм не найден", 3000);
            QMessageBox::information(this, "Результат поиска", "Фильм не найден.\n\nПопробуйте:\n- Проверить название фильма\n- Использовать оригинальное название\n- Добавить API ключ для расширенного поиска");
            return;
        }

        QString infoText = QString("Фильм найден:\n\n") +
                           QString("Название: %1\n").arg(QString::fromStdString(movie.getTitle())) +
                           QString("Год: %1\n").arg(movie.getYear()) +
                           QString("Рейтинг: %1\n").arg(movie.getRating());

        QString director = QString::fromStdString(movie.getDirector());
        if (!director.isEmpty()) {
            infoText += QString("Режиссер: %1\n").arg(director);
        }

        QString actors = QString::fromStdString(movie.getActors());
        if (!actors.isEmpty()) {
            infoText += QString("Актеры: %1\n").arg(actors);
        }

        infoText += "\nДобавить фильм в фильмотеку?";

        int ret = QMessageBox::question(this, "Фильм найден", infoText,
                                        QMessageBox::Yes | QMessageBox::No);

        if (ret == QMessageBox::Yes) {
            QString appDir = QApplication::applicationDirPath();
            QString currentDir = QDir::currentPath();
            QString sep = QDir::separator();

            QString posterDir;
            bool foundDir = false;
            QStringList possiblePosterDirs;
            possiblePosterDirs << appDir + sep + "posters";
            possiblePosterDirs << appDir + sep + ".." + sep + "posters";
            possiblePosterDirs << currentDir + sep + "posters";
            possiblePosterDirs << currentDir + sep + ".." + sep + "posters";
            possiblePosterDirs << "posters";

            for (const QString& dir : possiblePosterDirs) {
                QDir testDir(dir);
                if (testDir.exists()) {
                    posterDir = testDir.absolutePath();
                    foundDir = true;
                    break;
                }
            }

            if (!foundDir) {
                posterDir = currentDir + sep + "posters";
                QDir().mkpath(posterDir);
            }

            QString posterFileName = QString::number(movie.getId()) + ".jpg";
            QString savePath = posterDir + sep + posterFileName;

            if (!posterUrl.isEmpty()) {
                ui->statusbar->showMessage("Загрузка постера...", 0);
                posterManager->downloadPoster(posterUrl, savePath, [this, movie, posterDir, sep, movieId = movie.getId()](bool success) {
                    Movie movieToAdd = movie;
                    QString relativePath = "";

                    if (success) {
                        QString posterFileNameJpg = QString::number(movieId) + ".jpg";
                        QString posterFileNamePng = QString::number(movieId) + ".png";
                        QString fullPosterPathJpg = posterDir + sep + posterFileNameJpg;
                        QString fullPosterPathPng = posterDir + sep + posterFileNamePng;

                        QThread::msleep(100);

                        if (QFile::exists(fullPosterPathJpg)) {
                            relativePath = "posters/" + posterFileNameJpg;
                        } else if (QFile::exists(fullPosterPathPng)) {
                            relativePath = "posters/" + posterFileNamePng;
                        } else {
                            QDir dir(posterDir);
                            QStringList filters;
                            filters << QString::number(movieId) + ".*";
                            QFileInfoList files = dir.entryInfoList(filters, QDir::Files);
                            if (!files.isEmpty()) {
                                relativePath = "posters/" + files.first().fileName();
                            } else {
                                relativePath = "posters/" + posterFileNameJpg;
                            }
                        }
                        movieToAdd.setPosterPath(relativePath.toStdString());
                        ui->statusbar->showMessage("Постер загружен!", 2000);
            } else {
                        relativePath = "posters/" + QString::number(movieId) + ".jpg";
                        movieToAdd.setPosterPath(relativePath.toStdString());
                    }

                    try {
                        qDebug() << "=== Adding movie to file ===";
                        qDebug() << "Movie country before add:" << QString::fromStdString(movieToAdd.getCountry());
                        qDebug() << "Movie actors before add:" << QString::fromStdString(movieToAdd.getActors());
                        qDebug() << "Movie duration before add:" << movieToAdd.getDuration();
                        manager.addMovieToFile(movieToAdd);
                        manager.reloadMovies();
                        // Проверяем, что данные сохранились
                        const Movie* savedMovie = manager.findMovieById(movieToAdd.getId());
                        if (savedMovie) {
                            qDebug() << "Movie country after reload:" << QString::fromStdString(savedMovie->getCountry());
                            qDebug() << "Movie actors after reload:" << QString::fromStdString(savedMovie->getActors());
                            qDebug() << "Movie duration after reload:" << savedMovie->getDuration();
                        }
                        populateAllMovies(manager.getAllMovies());
                        populateGenres();
                        ui->statusbar->showMessage("Фильм добавлен в фильмотеку!", 3000);
                        QMessageBox::information(this, "Успех", "Фильм успешно добавлен в фильмотеку!");
                    } catch (const std::exception& e) {
                        QMessageBox::warning(this, "Ошибка", "Не удалось добавить фильм: " + QString(e.what()));
                    }
                });
            } else {
                try {
                    qDebug() << "=== Adding movie to file (no poster) ===";
                    qDebug() << "Movie country before add:" << QString::fromStdString(movie.getCountry());
                    qDebug() << "Movie actors before add:" << QString::fromStdString(movie.getActors());
                    qDebug() << "Movie duration before add:" << movie.getDuration();
                    manager.addMovieToFile(movie);
                    manager.reloadMovies();
                    // Проверяем, что данные сохранились
                    const Movie* savedMovie = manager.findMovieById(movie.getId());
                    if (savedMovie) {
                        qDebug() << "Movie country after reload:" << QString::fromStdString(savedMovie->getCountry());
                        qDebug() << "Movie actors after reload:" << QString::fromStdString(savedMovie->getActors());
                        qDebug() << "Movie duration after reload:" << savedMovie->getDuration();
                    }
                    populateAllMovies(manager.getAllMovies());
                    populateGenres();
                    ui->statusbar->showMessage("Фильм добавлен в фильмотеку!", 3000);
                    QMessageBox::information(this, "Успех", "Фильм успешно добавлен в фильмотеку!");
                } catch (const std::exception& e) {
                    QMessageBox::warning(this, "Ошибка", "Не удалось добавить фильм: " + QString(e.what()));
                }
            }
        }
    },
    [this](const QString& errorMessage) {
        ui->statusbar->showMessage("Ошибка поиска: " + errorMessage, 3000);
        QMessageBox::warning(this, "Ошибка поиска", "Не удалось выполнить поиск фильма:\n" + errorMessage);
    });
}


void MainWindow::handleSortByRating() {
    try {
        manager.sortByRating();
        isSortedByRating = true;
        populateAllMovies(manager.getAllMovies());
        ui->statusbar->showMessage("Sorted by rating", 2000);
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Sort error", e.what());
    }
}

void MainWindow::handleShowTopN() {
    bool ok = false;
    int n = QInputDialog::getInt(this, "Top N", "How many movies?", 10, 1, 1000, 1, &ok);
    if (!ok) return;
    try {
        auto top = manager.topRatedResults(n);
        populateAllMovies(top);
        ui->statusbar->showMessage("Showing top N", 2000);
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Top N error", e.what());
    }
}

int MainWindow::selectedMovieIdFromAll() const {
    return -1;
}

int MainWindow::selectedMovieIdFromFavorites() const {
    return -1;
}


void MainWindow::handleHome() {
    try {
        manager.reloadMovies();
        isSortedByRating = false;
        populateAllMovies(manager.getAllMovies());
        populateFavorites();
        populateCollections();
        populateGenres();
        ui->statusbar->showMessage("Home", 1500);
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Home", e.what());
    }
}

void MainWindow::handleAllSelectionChanged() {
}

void MainWindow::handleFavSelectionChanged() {
}

void MainWindow::handleCreateCollection() {
    bool ok;
    QString name = QInputDialog::getText(this, "Create Collection",
                                         "Collection name:", QLineEdit::Normal,
                                         "", &ok);
    if (!ok || name.isEmpty()) {
        return;
    }

    try {
        QByteArray utf8Name = name.toUtf8();
        MovieCollection* collection = manager.createCollection(std::string(utf8Name.constData(), utf8Name.length()));
        QMessageBox::information(this, "Success",
                                 QString("Collection '%1' created successfully!").arg(name));
        ui->statusbar->showMessage(QString("Collection '%1' created").arg(name), 3000);
        populateCollections();
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Error", e.what());
    }
}

void MainWindow::handleManageCollections() {
    auto collectionNames = manager.getAllCollectionNames();
    
    if (collectionNames.empty()) {
        QMessageBox::information(this, "Collections",
                                 "No collections available.\nUse 'Create Collection' to create one.");
        return;
    }

    QStringList names;
    for (const auto& name : collectionNames) {
        names << QString::fromUtf8(name.c_str(), name.length());
    }

    bool ok;
    QString selected = QInputDialog::getItem(this, "Manage Collections",
                                            "Select collection:", names, 0, false, &ok);
    if (!ok || selected.isEmpty()) {
        return;
    }

    auto* collManager = manager.getCollectionManager();
    if (!collManager) {
        QMessageBox::warning(this, "Error", "Collection manager not available");
        return;
    }

    QByteArray utf8Selected = selected.toUtf8();
    MovieCollection* collection = collManager->getCollection(std::string(utf8Selected.constData(), utf8Selected.length()));
    if (!collection) {
        QMessageBox::warning(this, "Error", "Collection not found");
        return;
    }

    QStringList items;
    items << "Add movie" << "Remove movie" << "View collection" << "Delete collection";
    QString action = QInputDialog::getItem(this, "Collection Action",
                                           QString("What to do with '%1'?").arg(selected),
                                           items, 0, false, &ok);
    if (!ok) {
        return;
    }

    if (action == "Add movie") {
        int id = selectedMovieIdFromAll();
        if (id < 0) {
            QMessageBox::information(this, "Add to Collection",
                                     "Select a movie in All Movies.");
            return;
        }
        const Movie* movie = manager.findMovieById(id);
        if (movie) {
            try {
                collection->addMovie(*movie);
                QMessageBox::information(this, "Success",
                                         QString("Movie added to collection '%1'").arg(selected));
                ui->statusbar->showMessage("Movie added to collection", 2000);
                populateCollections();
                handleCollectionChanged();
            } catch (const std::exception& e) {
                QMessageBox::warning(this, "Error", e.what());
            }
        }
    } else if (action == "Remove movie") {
        int id = selectedMovieIdFromAll();
        if (id < 0) {
            QMessageBox::information(this, "Remove from Collection",
                                     "Select a movie first.");
            return;
        }
        try {
            collection->removeMovie(id);
            QMessageBox::information(this, "Success",
                                     QString("Movie removed from collection '%1'").arg(selected));
            ui->statusbar->showMessage("Movie removed from collection", 2000);
            populateCollections();
            handleCollectionChanged();
        } catch (const std::exception& e) {
            QMessageBox::warning(this, "Error", e.what());
        }
    } else if (action == "View collection") {
        auto movies = collection->getMovies();
        QStringList movieTitles;
        for (const auto& m : movies) {
            movieTitles << QString::fromStdString(m.getTitle());
        }
        QMessageBox::information(this, QString("Collection: %1").arg(selected),
                                 movieTitles.isEmpty() ? "Collection is empty" :
                                 movieTitles.join("\n"));
    } else if (action == "Delete collection") {
        int ret = QMessageBox::question(this, "Delete Collection",
                                        QString("Are you sure you want to delete '%1'?")
                                        .arg(selected),
                                        QMessageBox::Yes | QMessageBox::No);
        if (ret == QMessageBox::Yes) {
            try {
                QByteArray utf8Selected = selected.toUtf8();
                collManager->deleteCollection(std::string(utf8Selected.constData(), utf8Selected.length()));
                QMessageBox::information(this, "Success",
                                         QString("Collection '%1' deleted").arg(selected));
                ui->statusbar->showMessage("Collection deleted", 2000);
                populateCollections();
            } catch (const std::exception& e) {
                QMessageBox::warning(this, "Error", e.what());
            }
        }
    }
}

// УДАЛЕНО: populatePlayer - вкладка Плеер удалена
// УДАЛЕНО: playVideoFile - вкладка Плеер удалена

// УДАЛЕНО: searchMovieAPI - перенесено в KinopoiskAPIClient
// УДАЛЕНО: processMovieData - перенесено в KinopoiskAPIClient
// УДАЛЕНО: parseMovieFromJSON - перенесено в KinopoiskAPIClient
// УДАЛЕНО: downloadPoster - перенесено в PosterManager


