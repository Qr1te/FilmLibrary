#include "../includes/MainWindow.h"
#include "../includes/exceptions/DuplicateMovieException.h"
#include "../includes/exceptions/InvalidInputException.h"
#include "../includes/exceptions/MovieException.h"
#include "../includes/exceptions/FileNotFoundException.h"
#include "../includes/exceptions/DuplicateCollectionException.h"
#include "../includes/exceptions/CollectionNotFoundException.h"

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
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QTabWidget>
#include <QComboBox>
#include <QToolBar>
#include <QStatusBar>
#include <QAction>
#include <QSpacerItem>
#include <QFrame>
#include <QSizePolicy>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {
    setupUI();

    managers.networkManager = new QNetworkAccessManager(this);
    managers.apiClient = new KinopoiskAPIClient(this);
    managers.posterManager = new PosterManager(this);
    managers.posterManager->setNetworkManager(managers.networkManager);
    managers.cardFactory = new MovieCardFactory(&manager, managers.posterManager, statusbar);
    
    managers.cardFactory->setOnFavoritesChanged([this]() { populateFavorites(); });
    managers.cardFactory->setOnCollectionsChanged([this]() { populateCollections(); handleCollectionChanged(); });
    managers.cardFactory->setOnMoviesChanged([this]() { populateAllMovies(manager.getAllMovies()); });
    managers.cardFactory->setOnGenresChanged([this]() { populateGenres(); });
    managers.cardFactory->setOnShowInfo([this](const Movie& movie) { showMovieInfo(movie); });

    setupModelsAndViews();
    populateGenres();
    
    populateAllMovies(manager.getAllMovies());
    populateFavorites();
    populateCollections();

    connect(searchUI.searchButton, &QPushButton::clicked, this, &MainWindow::handleSearch);
    connect(searchUI.addMovieButton, &QPushButton::clicked, this, &MainWindow::handleAddMovie);
    

    searchUI.addMovieButton->setStyleSheet(
        "QPushButton { background-color: #9FFF9F; color: #1a1a1a; border: none; padding: 8px; border-radius: 4px; font-weight: bold; }"
        "QPushButton:hover { background-color: #BFFFBF; }"
        "QPushButton:pressed { background-color: #7FFF7F; }"
    );

    connect(toolBarUI.actionSortByRating, &QAction::triggered, this, &MainWindow::handleSortByRating);
    connect(toolBarUI.actionTopN, &QAction::triggered, this, &MainWindow::handleShowTopN);
    connect(toolBarUI.actionHome, &QAction::triggered, this, &MainWindow::handleHome);
    connect(tabCollectionsUI.createCollectionButton, &QPushButton::clicked, this, &MainWindow::handleCreateCollection);
    connect(tabCollectionsUI.manageCollectionsButton, &QPushButton::clicked, this, &MainWindow::handleDeleteCollection);
    connect(tabCollectionsUI.collectionComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::handleCollectionChanged);
    
    tabCollectionsUI.createCollectionButton->setStyleSheet(
        "QPushButton { background-color: #9FFF9F; color: #1a1a1a; border: none; padding: 8px; border-radius: 4px; font-weight: bold; }"
        "QPushButton:hover { background-color: #BFFFBF; }"
        "QPushButton:pressed { background-color: #7FFF7F; }"
    );
    
    tabCollectionsUI.manageCollectionsButton->setStyleSheet(
        "QPushButton { background-color: #FF6B6B; color: white; border: none; padding: 8px; border-radius: 4px; font-weight: bold; }"
        "QPushButton:hover { background-color: #FF8E8E; }"
        "QPushButton:pressed { background-color: #E55555;}"
    );
}

void MainWindow::setupUI() {

    setWindowTitle("Домашняя фильмотека");
    resize(1100, 720);
    
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    searchUI.searchLayout = new QHBoxLayout();
    searchUI.searchLayout->setContentsMargins(10, 10, 10, 10);
    
    searchUI.addMovieButton = new QPushButton("Добавить фильм", centralWidget);
    searchUI.searchLayout->addWidget(searchUI.addMovieButton);
    
    searchUI.searchLineEdit = new QLineEdit(centralWidget);
    searchUI.searchLineEdit->setPlaceholderText("Поиск по названию...");
    searchUI.searchLayout->addWidget(searchUI.searchLineEdit);
    
    searchUI.genreComboBox = new QComboBox(centralWidget);
    searchUI.genreComboBox->setEditable(true);
    searchUI.genreComboBox->setPlaceholderText("Жанр");
    searchUI.searchLayout->addWidget(searchUI.genreComboBox);
    
    searchUI.searchButton = new QPushButton("Поиск", centralWidget);
    searchUI.searchLayout->addWidget(searchUI.searchButton);
    
    mainLayout->addLayout(searchUI.searchLayout);
    

    tabWidget = new QTabWidget(centralWidget);
    tabWidget->setCurrentIndex(0);
    

    tabAllUI.tabAll = new QWidget();
    tabAllUI.verticalLayoutAll = new QVBoxLayout(tabAllUI.tabAll);
    tabAllUI.verticalLayoutAll->setContentsMargins(0, 0, 0, 0);
    
    tabAllUI.scrollAreaAll = new QScrollArea(tabAllUI.tabAll);
    tabAllUI.scrollAreaAll->setWidgetResizable(true);
    tabAllUI.scrollAreaAll->setFrameShape(QFrame::NoFrame);
    
    tabAllUI.scrollAreaWidgetContentsAll = new QWidget();
    tabAllUI.scrollAreaWidgetContentsAll->setGeometry(0, 0, 1062, 617);
    tabAllUI.gridLayoutMovies = new QGridLayout(tabAllUI.scrollAreaWidgetContentsAll);
    tabAllUI.gridLayoutMovies->setContentsMargins(10, 10, 10, 10);
    tabAllUI.gridLayoutMovies->setSpacing(10);
    
    tabAllUI.scrollAreaAll->setWidget(tabAllUI.scrollAreaWidgetContentsAll);
    tabAllUI.verticalLayoutAll->addWidget(tabAllUI.scrollAreaAll);
    
    tabWidget->addTab(tabAllUI.tabAll, "Все фильмы");
    

    tabFavoritesUI.tabFavorites = new QWidget();
    tabFavoritesUI.verticalLayoutFav = new QVBoxLayout(tabFavoritesUI.tabFavorites);
    tabFavoritesUI.verticalLayoutFav->setContentsMargins(0, 0, 0, 0);
    
    tabFavoritesUI.scrollAreaFavorites = new QScrollArea(tabFavoritesUI.tabFavorites);
    tabFavoritesUI.scrollAreaFavorites->setWidgetResizable(true);
    tabFavoritesUI.scrollAreaFavorites->setFrameShape(QFrame::NoFrame);
    
    tabFavoritesUI.scrollAreaWidgetContentsFavorites = new QWidget();
    tabFavoritesUI.scrollAreaWidgetContentsFavorites->setGeometry(0, 0, 1062, 617);
    tabFavoritesUI.gridLayoutFavorites = new QGridLayout(tabFavoritesUI.scrollAreaWidgetContentsFavorites);
    tabFavoritesUI.gridLayoutFavorites->setContentsMargins(10, 10, 10, 10);
    tabFavoritesUI.gridLayoutFavorites->setSpacing(10);
    
    tabFavoritesUI.scrollAreaFavorites->setWidget(tabFavoritesUI.scrollAreaWidgetContentsFavorites);
    tabFavoritesUI.verticalLayoutFav->addWidget(tabFavoritesUI.scrollAreaFavorites);
    
    tabWidget->addTab(tabFavoritesUI.tabFavorites, "Избранное");

    tabCollectionsUI.tabCollections = new QWidget();
    tabCollectionsUI.verticalLayoutCollections = new QVBoxLayout(tabCollectionsUI.tabCollections);
    tabCollectionsUI.verticalLayoutCollections->setContentsMargins(0, 0, 0, 0);
    
    tabCollectionsUI.horizontalLayoutCollectionSelector = new QHBoxLayout();
    tabCollectionsUI.horizontalLayoutCollectionSelector->setContentsMargins(10, 10, 10, 10);
    
    tabCollectionsUI.collectionLabel = new QLabel("Выберите коллекцию:", tabCollectionsUI.tabCollections);
    tabCollectionsUI.horizontalLayoutCollectionSelector->addWidget(tabCollectionsUI.collectionLabel);
    
    tabCollectionsUI.collectionComboBox = new QComboBox(tabCollectionsUI.tabCollections);
    tabCollectionsUI.collectionComboBox->setEditable(false);
    tabCollectionsUI.horizontalLayoutCollectionSelector->addWidget(tabCollectionsUI.collectionComboBox);
    
    tabCollectionsUI.createCollectionButton = new QPushButton("Создать коллекцию", tabCollectionsUI.tabCollections);
    tabCollectionsUI.horizontalLayoutCollectionSelector->addWidget(tabCollectionsUI.createCollectionButton);
    
    tabCollectionsUI.manageCollectionsButton = new QPushButton("Удалить коллекцию", tabCollectionsUI.tabCollections);
    tabCollectionsUI.horizontalLayoutCollectionSelector->addWidget(tabCollectionsUI.manageCollectionsButton);
    
    tabCollectionsUI.horizontalLayoutCollectionSelector->addSpacerItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
    
    tabCollectionsUI.verticalLayoutCollections->addLayout(tabCollectionsUI.horizontalLayoutCollectionSelector);
    
    tabCollectionsUI.scrollAreaCollections = new QScrollArea(tabCollectionsUI.tabCollections);
    tabCollectionsUI.scrollAreaCollections->setWidgetResizable(true);
    tabCollectionsUI.scrollAreaCollections->setFrameShape(QFrame::NoFrame);
    
    tabCollectionsUI.scrollAreaWidgetContentsCollections = new QWidget();
    tabCollectionsUI.scrollAreaWidgetContentsCollections->setGeometry(0, 0, 1062, 617);
    tabCollectionsUI.gridLayoutCollections = new QGridLayout(tabCollectionsUI.scrollAreaWidgetContentsCollections);
    tabCollectionsUI.gridLayoutCollections->setContentsMargins(10, 10, 10, 10);
    tabCollectionsUI.gridLayoutCollections->setSpacing(10);
    
    tabCollectionsUI.scrollAreaCollections->setWidget(tabCollectionsUI.scrollAreaWidgetContentsCollections);
    tabCollectionsUI.verticalLayoutCollections->addWidget(tabCollectionsUI.scrollAreaCollections);
    
    tabWidget->addTab(tabCollectionsUI.tabCollections, "Коллекции");
    
    mainLayout->addWidget(tabWidget, 1);
    
    toolBarUI.mainToolBar = addToolBar("mainToolBar");
    
    toolBarUI.actionHome = new QAction("Главная", this);
    toolBarUI.mainToolBar->addAction(toolBarUI.actionHome);
    
    toolBarUI.actionSortByRating = new QAction("Сортировать по рейтингу", this);
    toolBarUI.mainToolBar->addAction(toolBarUI.actionSortByRating);
    
    toolBarUI.actionTopN = new QAction("Показать топ N", this);
    toolBarUI.mainToolBar->addAction(toolBarUI.actionTopN);
    

    statusbar = statusBar();
}

void MainWindow::setupModelsAndViews() {
    setStyleSheet(R"(
        QMainWindow { background-color: #2b2b2b; }
        QWidget { background-color: #2b2b2b; color: #e0e0e0; }
        QLabel { color: #e0e0e0; background-color: transparent; }
        QLineEdit { background-color: #3a3a3a; color: #e0e0e0; border: 1px solid #555; padding: 5px; border-radius: 4px; }
        QComboBox { background-color: #3a3a3a; color: #e0e0e0; border: 1px solid #555; padding: 5px; border-radius: 4px; }
        QPushButton { background-color: #ff6b35; color: white; border: none; padding: 8px; border-radius: 4px; font-weight: bold; }
        QPushButton:hover { background-color: #ff8555; }
        QPushButton:pressed { background-color: #e55a2b; }
        QTabWidget::pane { border: 1px solid #555; background-color: #2b2b2b; }
        QTabBar::tab { background-color: #3a3a3a; color: #e0e0e0; padding: 8px 16px; border-top-left-radius: 4px; border-top-right-radius: 4px; }
        QTabBar::tab:selected { background-color: #ff6b35; color: white; }
        QTabBar::tab:hover { background-color: #4a4a4a; }
        QScrollArea { background-color: #2b2b2b; border: none; }
        QTextEdit { background-color: #3a3a3a; color: #e0e0e0; border: 1px solid #555; border-radius: 4px; font-size: 11pt; line-height: 1.4; }
        QStatusBar { background-color: #1e1e1e; color: #e0e0e0; }
        QToolBar { background-color: #1e1e1e; border: none; }
        QLabel[name="titleValue"] { font-size: 16pt; font-weight: bold; color: #ff6b35; padding: 5px 0px; }
        QLabel[name="titleLabel"], QLabel[name="ratingLabel"], QLabel[name="yearLabel"], QLabel[name="genreLabel"], QLabel[name="directorLabel"] { font-size: 11pt; font-weight: bold; color: #bbb; padding: 3px 0px; }
        QLabel[name="ratingValue"] { font-size: 14pt; font-weight: bold; color: #46d369; padding: 3px 0px; }
        QLabel[name="yearValue"], QLabel[name="genreValue"], QLabel[name="directorValue"] { font-size: 11pt; color: #e0e0e0; padding: 3px 0px; }
        QLabel[name="posterLabel"] { padding: 10px; }
    )");
}

void MainWindow::populateGenres() {
    searchUI.genreComboBox->clear();
    searchUI.genreComboBox->addItem("");
    QSet<QString> genres;
    for (const auto& m : manager.getAllMovies()) {
        const auto& movieGenres = m.getGenres();
        for (const auto& genre : movieGenres) {
            genres.insert(stdStringToQString(genre));
        }
    }
    auto sorted = QStringList(genres.begin(), genres.end());
    sorted.sort(Qt::CaseInsensitive);
    searchUI.genreComboBox->addItems(sorted);
}

void MainWindow::showMovieInfo(const Movie& movie) {
        auto* dialog = new QDialog(this);
    dialog->setWindowTitle("Информация о фильме");
        dialog->setMinimumSize(1100, 750);
        dialog->setStyleSheet(
            "QDialog { background-color: #2b2b2b; }"
            "QLabel { color: #e0e0e0; background-color: transparent; }"
            "QTextEdit { background-color: #3a3a3a; color: #e0e0e0; border: 1px solid #555; border-radius: 4px; }"
            "QPushButton { background-color: #ff6b35; color: white; border: none; padding: 8px 20px; border-radius: 4px; font-weight: bold; }"
            "QPushButton:hover { background-color: #ff8555; }"
        );
        
        auto* dialogLayout = new QHBoxLayout(dialog);
        dialogLayout->setSpacing(20);
        dialogLayout->setContentsMargins(20, 20, 20, 20);
        
        auto* posterLabel = new QLabel();
        posterLabel->setMinimumSize(450, 675);
        posterLabel->setMaximumSize(450, 675);
        posterLabel->setAlignment(Qt::AlignCenter);
        posterLabel->setStyleSheet("border: 1px solid #555; border-radius: 4px; background-color: #3a3a3a;");
    managers.posterManager->loadPosterToLabel(posterLabel, movie);
        dialogLayout->addWidget(posterLabel);
        
        auto* infoLayout = new QVBoxLayout();
        infoLayout->setSpacing(10);
        
        auto* titleLabel = new QLabel(stdStringToQString(movie.getTitle()));
        titleLabel->setStyleSheet("font-size: 20pt; font-weight: bold; color: #ff6b35;");
        titleLabel->setWordWrap(true);
        infoLayout->addWidget(titleLabel);
        
    auto* ratingLabel = new QLabel(QString("Рейтинг: %1").arg(movie.getRating(), 0, 'f', 1));
        ratingLabel->setStyleSheet("font-size: 14pt; font-weight: bold; color: #46d369;");
        infoLayout->addWidget(ratingLabel);
        
    auto* yearLabel = new QLabel(QString("Год: %1").arg(movie.getYear()));
        yearLabel->setStyleSheet("font-size: 12pt; color: #e0e0e0;");
        infoLayout->addWidget(yearLabel);
        
    auto* genreLabel = new QLabel(QString("Жанр: %1").arg(stdStringToQString(movie.getGenreString())));
        genreLabel->setStyleSheet("font-size: 12pt; color: #e0e0e0;");
        genreLabel->setWordWrap(true);
        infoLayout->addWidget(genreLabel);
        
    QString directorText = stdStringToQString(movie.getDirector());
    if (directorText.isEmpty()) {
        directorText = "Не указано";
    }
    auto* directorLabel = new QLabel(QString("Режиссер: %1").arg(directorText));
        directorLabel->setStyleSheet("font-size: 12pt; color: #e0e0e0;");
        directorLabel->setWordWrap(true);
        infoLayout->addWidget(directorLabel);
        
    QString countryText = stdStringToQString(movie.getCountry());
    if (countryText.isEmpty()) {
        countryText = "Не указано";
    }
    auto* countryLabel = new QLabel(QString("Страна: %1").arg(countryText));
        countryLabel->setStyleSheet("font-size: 12pt; color: #e0e0e0;");
        countryLabel->setWordWrap(true);
        infoLayout->addWidget(countryLabel);
        
    QString actorsText = stdStringToQString(movie.getActors());
    if (actorsText.isEmpty()) {
        actorsText = "Не указано";
    }
    auto* actorsLabel = new QLabel(QString("Актеры: %1").arg(actorsText));
        actorsLabel->setStyleSheet("font-size: 12pt; color: #e0e0e0;");
        actorsLabel->setWordWrap(true);
        infoLayout->addWidget(actorsLabel);
        
    QString durationText = QString("%1 мин").arg(movie.getDuration());
    auto* durationLabel = new QLabel(QString("Длительность: %1").arg(durationText));
        durationLabel->setStyleSheet("font-size: 12pt; color: #e0e0e0;");
        infoLayout->addWidget(durationLabel);
        
    auto* descTitleLabel = new QLabel("Описание:");
        descTitleLabel->setStyleSheet("font-size: 12pt; font-weight: bold; color: #bbb; margin-top: 10px;");
        infoLayout->addWidget(descTitleLabel);
        
        auto* descriptionText = new QTextEdit();
        descriptionText->setPlainText(stdStringToQString(movie.getDescription()));
        descriptionText->setReadOnly(true);
        descriptionText->setMaximumHeight(150);
        infoLayout->addWidget(descriptionText);
        
        infoLayout->addStretch();
        
    auto* closeBtn = new QPushButton("ОК");
        closeBtn->setCursor(Qt::PointingHandCursor);
        connect(closeBtn, &QPushButton::clicked, dialog, &QDialog::accept);
        infoLayout->addWidget(closeBtn, 0, Qt::AlignRight);
        
        dialogLayout->addLayout(infoLayout);
        
        dialog->exec();
        delete dialog;
}

void MainWindow::clearGridLayout(QGridLayout* layout) {
    if (!layout) {
        return;
    }
    QLayoutItem* item;
    while ((item = layout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }
}

void MainWindow::populateGridLayoutWithMovies(QGridLayout* layout, QWidget* parent, const std::vector<Movie>& movies) {
    if (!layout || !parent) {
        return;
    }
    
    clearGridLayout(layout);

    const int columns = 4;
    int row = 0;
    int col = 0;
    
    for (const auto& movie : movies) {
        QWidget* card = managers.cardFactory->createMovieCard(movie, parent);
        if (card) {
            layout->addWidget(card, row, col);
            
            col++;
            if (col >= columns) {
                col = 0;
        row++;
            }
        }
    }
}

void MainWindow::populateAllMovies(const std::vector<Movie>& movies) {
    populateGridLayoutWithMovies(tabAllUI.gridLayoutMovies, tabAllUI.scrollAreaWidgetContentsAll, movies);
    }
    
void MainWindow::populateFavorites() {
    auto favs = manager.getFavoriteMovies();
    populateGridLayoutWithMovies(tabFavoritesUI.gridLayoutFavorites, tabFavoritesUI.scrollAreaWidgetContentsFavorites, favs);
}

void MainWindow::populateCollections() {
    tabCollectionsUI.collectionComboBox->clear();
    auto collectionNames = manager.getAllCollectionNames();
    
    for (const auto& name : collectionNames) {
        tabCollectionsUI.collectionComboBox->addItem(QString::fromUtf8(name.c_str(), name.length()));
    }
    
    if (tabCollectionsUI.collectionComboBox->count() > 0) {
        handleCollectionChanged();
    } else {
        clearGridLayout(tabCollectionsUI.gridLayoutCollections);
    }
}

std::string MainWindow::qStringToStdString(const QString& str) const {
    QByteArray utf8 = str.toUtf8();
    return std::string(utf8.constData(), utf8.length());
}

QString MainWindow::stdStringToQString(const std::string& str) const {
    return QString::fromStdString(str);
}

QString MainWindow::findPosterDirectory() const {
    QString appDir = QApplication::applicationDirPath();
    QString currentDir = QDir::currentPath();
    QString sep = QDir::separator();
    
    QStringList possiblePosterDirs;
    possiblePosterDirs << appDir + sep + "posters";
    possiblePosterDirs << appDir + sep + ".." + sep + "posters";
    possiblePosterDirs << currentDir + sep + "posters";
    possiblePosterDirs << currentDir + sep + ".." + sep + "posters";
    possiblePosterDirs << "posters";
    
    for (const QString& dir : possiblePosterDirs) {
        QDir testDir(dir);
        if (testDir.exists()) {
            return testDir.absolutePath();
        }
    }
    
    QString posterDir = currentDir + sep + "posters";
    QDir().mkpath(posterDir);
    return posterDir;
}

void MainWindow::handleAddMovieToFile(const Movie& movie) {
    try {
        manager.addMovieToFile(movie);
        manager.reloadMovies();
        populateAllMovies(manager.getAllMovies());
        populateFavorites();
        populateGenres();
        statusbar->showMessage("Фильм добавлен в фильмотеку!", 3000);
        QMessageBox::information(this, "Успех", "Фильм успешно добавлен в фильмотеку!");
    } catch (const DuplicateMovieException& e) {
        QMessageBox::warning(this, "Дубликат фильма", stdStringToQString(e.what()));
        statusbar->showMessage("Фильм уже существует в фильмотеке", 3000);
    } catch (const FileNotFoundException& e) {
        QMessageBox::warning(this, "Ошибка", "Не удалось добавить фильм: " + QString(e.what()));
    } catch (const MovieException& e) {
        QMessageBox::warning(this, "Ошибка", "Не удалось добавить фильм: " + QString(e.what()));
    }
}

std::vector<Movie> MainWindow::findMovieIntersection(const std::vector<Movie>& movies1, const std::vector<Movie>& movies2) const {
    std::vector<Movie> intersection;
    for (const auto& movie : movies1) {
        for (const auto& genreMovie : movies2) {
            if (movie.getId() == genreMovie.getId()) {
                intersection.push_back(movie);
                break;
            }
        }
    }
    return intersection;
}

void MainWindow::onMovieSearchSuccess(const Movie& movie, const QString& posterUrl) {
    if (movie.getId() == 0) {
        statusbar->showMessage("Фильм не найден", 3000);
        QMessageBox::information(this, "Результат поиска", "Фильм не найден.\n\nПопробуйте:\n- Проверить название фильма\n- Использовать оригинальное название\n- Добавить API ключ для расширенного поиска");
        return;
    }

    QString infoText = QString("Фильм найден:\n\n") +
                       QString("Название: %1\n").arg(stdStringToQString(movie.getTitle())) +
                       QString("Год: %1\n").arg(movie.getYear()) +
                       QString("Рейтинг: %1\n").arg(movie.getRating());

    if (QString director = stdStringToQString(movie.getDirector()); !director.isEmpty()) {
        infoText += QString("Режиссер: %1\n").arg(director);
    }

    if (QString actors = stdStringToQString(movie.getActors()); !actors.isEmpty()) {
        infoText += QString("Актеры: %1\n").arg(actors);
    }

    infoText += "\nДобавить фильм в фильмотеку?";

    int ret = QMessageBox::question(this, "Фильм найден", infoText,
                                    QMessageBox::Yes | QMessageBox::No);

    if (ret == QMessageBox::Yes) {
        QString posterDir = findPosterDirectory();
        QString sep = QDir::separator();
        QString posterFileName = QString::number(movie.getId()) + ".jpg";
        QString savePath = posterDir + sep + posterFileName;

        if (!posterUrl.isEmpty()) {
            statusbar->showMessage("Загрузка постера...", 0);
            managers.posterManager->downloadPoster(posterUrl, savePath, 
                [this, movie, posterDir, sep, movieId = movie.getId()](bool success) {
                    onPosterDownloadFinished(movie, posterDir, sep, movieId, success);
                });
    } else {
            handleAddMovieToFile(movie);
        }
    }
}

void MainWindow::onPosterDownloadFinished(const Movie& movie, const QString& posterDir, const QString& sep, int movieId, bool success) {
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
        statusbar->showMessage("Постер загружен!", 2000);
    } else {
        relativePath = "posters/" + QString::number(movieId) + ".jpg";
        movieToAdd.setPosterPath(relativePath.toStdString());
    }

    handleAddMovieToFile(movieToAdd);
}

void MainWindow::handleCollectionChanged() {
    if (!tabCollectionsUI.gridLayoutCollections) {
        return;
    }
    QString collectionName = tabCollectionsUI.collectionComboBox->currentText();
    if (collectionName.isEmpty()) {
        clearGridLayout(tabCollectionsUI.gridLayoutCollections);
        return;
    }
    
    auto* collManager = manager.getCollectionManager();
    if (!collManager) {
        return;
    }
    
    const MovieCollection* collection = collManager->getCollection(qStringToStdString(collectionName));
    if (!collection) {
        return;
    }
    
    auto movies = collection->getMovies();
    populateGridLayoutWithMovies(tabCollectionsUI.gridLayoutCollections, tabCollectionsUI.scrollAreaWidgetContentsCollections, movies);
}

void MainWindow::handleSearch() {
    const QString title = searchUI.searchLineEdit->text().trimmed();
    const QString genre = searchUI.genreComboBox->currentText().trimmed();

    try {
        std::vector<Movie> filtered = manager.getAllMovies();
        
        if (!title.isEmpty()) {
            filtered = manager.searchByTitleResults(title.toStdString());
        }
        
        if (genre.isEmpty()) {
            populateAllMovies(filtered);
            statusbar->showMessage(QString("Found %1 movie(s)").arg(filtered.size()), 2000);
            return;
        }
        
        std::vector<Movie> genreFiltered = manager.searchByGenreResults(genre.toStdString());
        
        if (title.isEmpty()) {
            filtered = genreFiltered;
        } else {
            filtered = findMovieIntersection(filtered, genreFiltered);
        }
        
        populateAllMovies(filtered);
        statusbar->showMessage(QString("Found %1 movie(s)").arg(filtered.size()), 2000);
    } catch (const InvalidInputException& e) {
        QMessageBox::warning(this, "Search error", e.what());
    } catch (const MovieException& e) {
        QMessageBox::warning(this, "Search error", e.what());
    }
}

void MainWindow::handleAddMovie() {
    auto* dialog = new QDialog(this);
    dialog->setWindowTitle("Добавить фильм");
    dialog->setMinimumSize(500, 150);
    dialog->setStyleSheet(
        "QDialog { background-color: #2b2b2b; }"
        "QLabel { color: #e0e0e0; background-color: transparent; }"
        "QLineEdit { background-color: #3a3a3a; color: #e0e0e0; border: 1px solid #555; padding: 8px; border-radius: 4px; font-size: 12pt; }"
        "QPushButton { background-color: #ff6b35; color: white; border: none; padding: 10px 20px; border-radius: 4px; font-weight: bold; font-size: 12pt; }"
        "QPushButton:hover { background-color: #ff8555; }"
        "QPushButton:pressed { background-color: #e55a2b; }"
    );
    
    auto* dialogLayout = new QVBoxLayout(dialog);
    dialogLayout->setSpacing(15);
    dialogLayout->setContentsMargins(20, 20, 20, 20);
    
    auto* titleLabel = new QLabel("Введите название фильма для добавления:");
    titleLabel->setStyleSheet("font-size: 12pt; color: #e0e0e0;");
    dialogLayout->addWidget(titleLabel);
    
    auto* searchInput = new QLineEdit();
    searchInput->setPlaceholderText("Название фильма...");
    searchInput->setFocus();
    dialogLayout->addWidget(searchInput);
    
    auto* buttonsLayout = new QHBoxLayout();
    buttonsLayout->addStretch();
    
    auto* cancelBtn = new QPushButton("Отмена");
    cancelBtn->setStyleSheet(
        "background-color: #555; color: #e0e0e0; padding: 10px 20px; border-radius: 4px; font-weight: bold; font-size: 12pt;"
    );
    cancelBtn->setCursor(Qt::PointingHandCursor);
    connect(cancelBtn, &QPushButton::clicked, dialog, &QDialog::reject);
    buttonsLayout->addWidget(cancelBtn);
    
    auto* searchBtn = new QPushButton("Поиск");
    searchBtn->setCursor(Qt::PointingHandCursor);
    connect(searchBtn, &QPushButton::clicked, dialog, &QDialog::accept);
    buttonsLayout->addWidget(searchBtn);
    
    dialogLayout->addLayout(buttonsLayout);
    
    connect(searchInput, &QLineEdit::returnPressed, dialog, &QDialog::accept);
    
    if (dialog->exec() == QDialog::Accepted) {
        QString title = searchInput->text().trimmed();
        delete dialog;
        
        if (title.isEmpty()) {
            QMessageBox::information(this, "Добавить фильм", "Пожалуйста, введите название фильма для добавления");
            return;
        }
        
        managers.apiClient->searchMovie(title, 
        [this](const Movie& movie, const QString& posterUrl) {
            onMovieSearchSuccess(movie, posterUrl);
    },
    [this](const QString& errorMessage) {
        statusbar->showMessage("Ошибка поиска: " + errorMessage, 3000);
        QMessageBox::warning(this, "Ошибка поиска", "Не удалось выполнить поиск фильма:\n" + errorMessage);
    });
    } else {
        delete dialog;
    }
}


void MainWindow::handleSortByRating() {
    try {
        manager.sortByRating();
        isSortedByRating = true;
        populateAllMovies(manager.getAllMovies());
        statusbar->showMessage("Sorted by rating", 2000);
    } catch (const MovieException& e) {
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
        statusbar->showMessage("Showing top N", 2000);
    } catch (const InvalidInputException& e) {
        QMessageBox::warning(this, "Top N error", e.what());
    } catch (const MovieException& e) {
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
        statusbar->showMessage("Home", 1500);
    } catch (const FileNotFoundException& e) {
        QMessageBox::warning(this, "Home", e.what());
    } catch (const MovieException& e) {
        QMessageBox::warning(this, "Home", e.what());
    }
}

void MainWindow::handleAllSelectionChanged() const {
    /* Intentionally empty - reserved for future selection change handling.
     * This method is a placeholder for handling selection changes in the "All Movies" tab.
     * Currently, movie cards handle their own interactions, so this slot remains unimplemented.
     */
}

void MainWindow::handleFavSelectionChanged() const {
    /* Intentionally empty - reserved for future selection change handling.
     * This method is a placeholder for handling selection changes in the "Favorites" tab.
     * Currently, movie cards handle their own interactions, so this slot remains unimplemented.
     */
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
        manager.createCollection(qStringToStdString(name));
        QMessageBox::information(this, "Success",
                                 QString("Collection '%1' created successfully!").arg(name));
        statusbar->showMessage(QString("Collection '%1' created").arg(name), 3000);
        populateCollections();
    } catch (const DuplicateCollectionException& e) {
        QMessageBox::warning(this, "Error", e.what());
    } catch (const InvalidInputException& e) {
        QMessageBox::warning(this, "Error", e.what());
    } catch (const FileNotFoundException& e) {
        QMessageBox::warning(this, "Error", e.what());
    } catch (const MovieException& e) {
        QMessageBox::warning(this, "Error", e.what());
    }
}

void MainWindow::handleDeleteCollection() {
    auto collectionNames = manager.getAllCollectionNames();
    
    if (collectionNames.empty()) {
        QMessageBox::information(this, "Удалить коллекцию",
                                 "Нет доступных коллекций.\nИспользуйте 'Создать коллекцию' для создания новой.");
        return;
    }

    QStringList names;
    for (const auto& name : collectionNames) {
        names << QString::fromUtf8(name.c_str(), name.length());
    }

    bool ok;
    QString selected = QInputDialog::getItem(this, "Удалить коллекцию",
                                            "Выберите коллекцию для удаления:", names, 0, false, &ok);
    if (!ok || selected.isEmpty()) {
        return;
    }

    auto* collManager = manager.getCollectionManager();
    if (!collManager) {
        QMessageBox::warning(this, "Ошибка", "Менеджер коллекций недоступен");
        return;
    }

    int ret = QMessageBox::question(this, "Удалить коллекцию",
                                    QString("Вы уверены, что хотите удалить коллекцию '%1'?")
                                    .arg(selected),
                                    QMessageBox::Yes | QMessageBox::No);
    if (ret == QMessageBox::Yes) {
        try {
            collManager->deleteCollection(qStringToStdString(selected));
            QMessageBox::information(this, "Успех",
                                     QString("Коллекция '%1' удалена").arg(selected));
            statusbar->showMessage("Коллекция удалена", 2000);
            populateCollections();
            handleCollectionChanged();
        } catch (const CollectionNotFoundException& e) {
            QMessageBox::warning(this, "Ошибка", e.what());
        } catch (const FileNotFoundException& e) {
            QMessageBox::warning(this, "Ошибка", e.what());
        } catch (const MovieException& e) {
            QMessageBox::warning(this, "Ошибка", e.what());
        }
    }
}



