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
    : QMainWindow(parent), isSortedByRating(false) {
    setupUI();

    networkManager = new QNetworkAccessManager(this);
    apiClient = new KinopoiskAPIClient(this);
    posterManager = new PosterManager(this);
    posterManager->setNetworkManager(networkManager);
    cardFactory = new MovieCardFactory(&manager, posterManager, statusbar);
    
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

    connect(searchButton, &QPushButton::clicked, this, &MainWindow::handleSearch);
    connect(addMovieButton, &QPushButton::clicked, this, &MainWindow::handleAddMovie);
    

    addMovieButton->setStyleSheet(
        "QPushButton { background-color: #9FFF9F; color: #1a1a1a; border: none; padding: 8px; border-radius: 4px; font-weight: bold; }"
        "QPushButton:hover { background-color: #BFFFBF; }"
        "QPushButton:pressed { background-color: #7FFF7F; }"
    );

    connect(actionSortByRating, &QAction::triggered, this, &MainWindow::handleSortByRating);
    connect(actionTopN, &QAction::triggered, this, &MainWindow::handleShowTopN);
    connect(actionHome, &QAction::triggered, this, &MainWindow::handleHome);
    connect(createCollectionButton, &QPushButton::clicked, this, &MainWindow::handleCreateCollection);
    connect(manageCollectionsButton, &QPushButton::clicked, this, &MainWindow::handleDeleteCollection);
    connect(collectionComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::handleCollectionChanged);
    
    createCollectionButton->setStyleSheet(
        "QPushButton { background-color: #9FFF9F; color: #1a1a1a; border: none; padding: 8px; border-radius: 4px; font-weight: bold; }"
        "QPushButton:hover { background-color: #BFFFBF; }"
        "QPushButton:pressed { background-color: #7FFF7F; }"
    );
    
    manageCollectionsButton->setStyleSheet(
        "QPushButton { background-color: #FF6B6B; color: white; border: none; padding: 8px; border-radius: 4px; font-weight: bold; }"
        "QPushButton:hover { background-color: #FF8E8E; }"
        "QPushButton:pressed { background-color: #E55555;}"
    );
}

MainWindow::~MainWindow() {

}

void MainWindow::setupUI() {

    setWindowTitle("Менеджер фильмов");
    resize(1100, 720);
    
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    searchLayout = new QHBoxLayout();
    searchLayout->setContentsMargins(10, 10, 10, 10);
    
    addMovieButton = new QPushButton("Добавить фильм", centralWidget);
    searchLayout->addWidget(addMovieButton);
    
    searchLineEdit = new QLineEdit(centralWidget);
    searchLineEdit->setPlaceholderText("Поиск по названию...");
    searchLayout->addWidget(searchLineEdit);
    
    genreComboBox = new QComboBox(centralWidget);
    genreComboBox->setEditable(true);
    genreComboBox->setPlaceholderText("Жанр");
    searchLayout->addWidget(genreComboBox);
    
    searchButton = new QPushButton("Поиск", centralWidget);
    searchLayout->addWidget(searchButton);
    
    mainLayout->addLayout(searchLayout);
    

    tabWidget = new QTabWidget(centralWidget);
    tabWidget->setCurrentIndex(0);
    

    tabAll = new QWidget();
    verticalLayoutAll = new QVBoxLayout(tabAll);
    verticalLayoutAll->setContentsMargins(0, 0, 0, 0);
    
    scrollAreaAll = new QScrollArea(tabAll);
    scrollAreaAll->setWidgetResizable(true);
    scrollAreaAll->setFrameShape(QFrame::NoFrame);
    
    scrollAreaWidgetContentsAll = new QWidget();
    scrollAreaWidgetContentsAll->setGeometry(0, 0, 1062, 617);
    gridLayoutMovies = new QGridLayout(scrollAreaWidgetContentsAll);
    gridLayoutMovies->setContentsMargins(10, 10, 10, 10);
    gridLayoutMovies->setSpacing(10);
    
    scrollAreaAll->setWidget(scrollAreaWidgetContentsAll);
    verticalLayoutAll->addWidget(scrollAreaAll);
    
    tabWidget->addTab(tabAll, "Все фильмы");
    

    tabFavorites = new QWidget();
    verticalLayoutFav = new QVBoxLayout(tabFavorites);
    verticalLayoutFav->setContentsMargins(0, 0, 0, 0);
    
    scrollAreaFavorites = new QScrollArea(tabFavorites);
    scrollAreaFavorites->setWidgetResizable(true);
    scrollAreaFavorites->setFrameShape(QFrame::NoFrame);
    
    scrollAreaWidgetContentsFavorites = new QWidget();
    scrollAreaWidgetContentsFavorites->setGeometry(0, 0, 1062, 617);
    gridLayoutFavorites = new QGridLayout(scrollAreaWidgetContentsFavorites);
    gridLayoutFavorites->setContentsMargins(10, 10, 10, 10);
    gridLayoutFavorites->setSpacing(10);
    
    scrollAreaFavorites->setWidget(scrollAreaWidgetContentsFavorites);
    verticalLayoutFav->addWidget(scrollAreaFavorites);
    
    tabWidget->addTab(tabFavorites, "Избранное");

    tabCollections = new QWidget();
    verticalLayoutCollections = new QVBoxLayout(tabCollections);
    verticalLayoutCollections->setContentsMargins(0, 0, 0, 0);
    
    horizontalLayoutCollectionSelector = new QHBoxLayout();
    horizontalLayoutCollectionSelector->setContentsMargins(10, 10, 10, 10);
    
    collectionLabel = new QLabel("Выберите коллекцию:", tabCollections);
    horizontalLayoutCollectionSelector->addWidget(collectionLabel);
    
    collectionComboBox = new QComboBox(tabCollections);
    collectionComboBox->setEditable(false);
    horizontalLayoutCollectionSelector->addWidget(collectionComboBox);
    
    createCollectionButton = new QPushButton("Создать коллекцию", tabCollections);
    horizontalLayoutCollectionSelector->addWidget(createCollectionButton);
    
    manageCollectionsButton = new QPushButton("Удалить коллекцию", tabCollections);
    horizontalLayoutCollectionSelector->addWidget(manageCollectionsButton);
    
    horizontalLayoutCollectionSelector->addSpacerItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
    
    verticalLayoutCollections->addLayout(horizontalLayoutCollectionSelector);
    
    scrollAreaCollections = new QScrollArea(tabCollections);
    scrollAreaCollections->setWidgetResizable(true);
    scrollAreaCollections->setFrameShape(QFrame::NoFrame);
    
    scrollAreaWidgetContentsCollections = new QWidget();
    scrollAreaWidgetContentsCollections->setGeometry(0, 0, 1062, 617);
    gridLayoutCollections = new QGridLayout(scrollAreaWidgetContentsCollections);
    gridLayoutCollections->setContentsMargins(10, 10, 10, 10);
    gridLayoutCollections->setSpacing(10);
    
    scrollAreaCollections->setWidget(scrollAreaWidgetContentsCollections);
    verticalLayoutCollections->addWidget(scrollAreaCollections);
    
    tabWidget->addTab(tabCollections, "Коллекции");
    
    mainLayout->addWidget(tabWidget, 1);
    
    mainToolBar = addToolBar("mainToolBar");
    
    actionHome = new QAction("Главная", this);
    mainToolBar->addAction(actionHome);
    
    actionSortByRating = new QAction("Сортировать по рейтингу", this);
    mainToolBar->addAction(actionSortByRating);
    
    actionTopN = new QAction("Показать топ N", this);
    mainToolBar->addAction(actionTopN);
    

    statusbar = statusBar();
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
    genreComboBox->clear();
    genreComboBox->addItem("");
    QSet<QString> genres;
    for (const auto& m : manager.getAllMovies()) {
        const auto& movieGenres = m.getGenres();
        for (const auto& genre : movieGenres) {
            genres.insert(QString::fromStdString(genre));
        }
    }
    QStringList sorted = QStringList(genres.begin(), genres.end());
    sorted.sort(Qt::CaseInsensitive);
    genreComboBox->addItems(sorted);
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
        
        auto* mainLayout = new QHBoxLayout(dialog);
        mainLayout->setSpacing(20);
        mainLayout->setContentsMargins(20, 20, 20, 20);
        
        auto* posterLabel = new QLabel();
        posterLabel->setMinimumSize(450, 675);
        posterLabel->setMaximumSize(450, 675);
        posterLabel->setAlignment(Qt::AlignCenter);
        posterLabel->setStyleSheet("border: 1px solid #555; border-radius: 4px; background-color: #3a3a3a;");
    posterManager->loadPosterToLabel(posterLabel, movie);
        mainLayout->addWidget(posterLabel);
        
        auto* infoLayout = new QVBoxLayout();
        infoLayout->setSpacing(10);
        
        auto* titleLabel = new QLabel(QString::fromStdString(movie.getTitle()));
        titleLabel->setStyleSheet("font-size: 20pt; font-weight: bold; color: #ff6b35;");
        titleLabel->setWordWrap(true);
        infoLayout->addWidget(titleLabel);
        
    auto* ratingLabel = new QLabel(QString("Рейтинг: %1").arg(movie.getRating(), 0, 'f', 1));
        ratingLabel->setStyleSheet("font-size: 14pt; font-weight: bold; color: #46d369;");
        infoLayout->addWidget(ratingLabel);
        
    auto* yearLabel = new QLabel(QString("Год: %1").arg(movie.getYear()));
        yearLabel->setStyleSheet("font-size: 12pt; color: #e0e0e0;");
        infoLayout->addWidget(yearLabel);
        
    auto* genreLabel = new QLabel(QString("Жанр: %1").arg(QString::fromStdString(movie.getGenreString())));
        genreLabel->setStyleSheet("font-size: 12pt; color: #e0e0e0;");
        genreLabel->setWordWrap(true);
        infoLayout->addWidget(genreLabel);
        
    QString directorText = QString::fromStdString(movie.getDirector());
    if (directorText.isEmpty()) {
        directorText = "Не указано";
    }
    auto* directorLabel = new QLabel(QString("Режиссер: %1").arg(directorText));
        directorLabel->setStyleSheet("font-size: 12pt; color: #e0e0e0;");
        directorLabel->setWordWrap(true);
        infoLayout->addWidget(directorLabel);
        
    QString countryText = QString::fromStdString(movie.getCountry());
    if (countryText.isEmpty()) {
        countryText = "Не указано";
    }
    auto* countryLabel = new QLabel(QString("Страна: %1").arg(countryText));
        countryLabel->setStyleSheet("font-size: 12pt; color: #e0e0e0;");
        countryLabel->setWordWrap(true);
        infoLayout->addWidget(countryLabel);
        
    QString actorsText = QString::fromStdString(movie.getActors());
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
        descriptionText->setPlainText(QString::fromStdString(movie.getDescription()));
        descriptionText->setReadOnly(true);
        descriptionText->setMaximumHeight(150);
        infoLayout->addWidget(descriptionText);
        
        infoLayout->addStretch();
        
    auto* closeBtn = new QPushButton("ОК");
        closeBtn->setCursor(Qt::PointingHandCursor);
        connect(closeBtn, &QPushButton::clicked, dialog, &QDialog::accept);
        infoLayout->addWidget(closeBtn, 0, Qt::AlignRight);
        
        mainLayout->addLayout(infoLayout);
        
        dialog->exec();
        delete dialog;
}

void MainWindow::populateAllMovies(const std::vector<Movie>& movies) {
    if (!gridLayoutMovies) {
        return;
    }
    
    QLayoutItem* item;
    while ((item = gridLayoutMovies->takeAt(0)) != nullptr) {
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }

    const int columns = 4;
    int row = 0, col = 0;
    
    for (const auto& movie : movies) {
        QWidget* card = cardFactory->createMovieCard(movie, scrollAreaWidgetContentsAll);
        if (card) {
            gridLayoutMovies->addWidget(card, row, col);
            
            col++;
            if (col >= columns) {
                col = 0;
        row++;
            }
        }
    }
}

void MainWindow::populateFavorites() {
    if (!gridLayoutFavorites) {
        return;
    }
    
    auto favs = manager.getFavoriteMovies();
    QLayoutItem* item;
    while ((item = gridLayoutFavorites->takeAt(0)) != nullptr) {
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }

    const int columns = 4;
    int row = 0, col = 0;
    
    for (const auto& movie : favs) {
        QWidget* card = cardFactory->createMovieCard(movie, scrollAreaWidgetContentsFavorites);
        if (card) {
            gridLayoutFavorites->addWidget(card, row, col);
            
            col++;
            if (col >= columns) {
                col = 0;
        row++;
            }
        }
    }
}

void MainWindow::populateCollections() {
    collectionComboBox->clear();
    auto collectionNames = manager.getAllCollectionNames();
    
    for (const auto& name : collectionNames) {
        collectionComboBox->addItem(QString::fromUtf8(name.c_str(), name.length()));
    }
    
    if (collectionComboBox->count() > 0) {
        handleCollectionChanged();
    } else {
        if (gridLayoutCollections) {
            QLayoutItem* item;
            while ((item = gridLayoutCollections->takeAt(0)) != nullptr) {
                if (item->widget()) {
                    item->widget()->deleteLater();
                }
                delete item;
            }
        }
    }
}

void MainWindow::handleCollectionChanged() {
    if (!gridLayoutCollections) {
        return;
    }
    QString collectionName = collectionComboBox->currentText();
    if (collectionName.isEmpty()) {
        QLayoutItem* item;
        while ((item = gridLayoutCollections->takeAt(0)) != nullptr) {
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
    while ((item = gridLayoutCollections->takeAt(0)) != nullptr) {
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }

    const int columns = 4;
    int row = 0, col = 0;
    
    for (const auto& movie : movies) {
        QWidget* card = cardFactory->createMovieCard(movie, scrollAreaWidgetContentsCollections);
        if (card) {
            gridLayoutCollections->addWidget(card, row, col);
            
            col++;
            if (col >= columns) {
                col = 0;
                row++;
            }
        }
    }
}

void MainWindow::handleSearch() {
    const QString title = searchLineEdit->text().trimmed();
    const QString genre = genreComboBox->currentText().trimmed();

    try {
        std::vector<Movie> filtered = manager.getAllMovies();
        
        if (!title.isEmpty()) {
            filtered = manager.searchByTitleResults(title.toStdString());
        }
        
        if (!genre.isEmpty()) {
            std::vector<Movie> genreFiltered = manager.searchByGenreResults(genre.toStdString());
            
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
        
        apiClient->searchMovie(title, 
        [this](const Movie& movie, const QString& posterUrl) {
        if (movie.getId() == 0) {
            statusbar->showMessage("Фильм не найден", 3000);
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
                statusbar->showMessage("Загрузка постера...", 0);
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
                        statusbar->showMessage("Постер загружен!", 2000);
            } else {
                        relativePath = "posters/" + QString::number(movieId) + ".jpg";
                        movieToAdd.setPosterPath(relativePath.toStdString());
                    }

                    try {
                        manager.addMovieToFile(movieToAdd);
                        manager.reloadMovies();
                        populateAllMovies(manager.getAllMovies());
                        populateFavorites();
                        populateGenres();
                        statusbar->showMessage("Фильм добавлен в фильмотеку!", 3000);
                        QMessageBox::information(this, "Успех", "Фильм успешно добавлен в фильмотеку!");
                    } catch (const DuplicateMovieException& e) {
                        QMessageBox::warning(this, "Дубликат фильма", QString::fromStdString(e.what()));
                        statusbar->showMessage("Фильм уже существует в фильмотеке", 3000);
                    } catch (const FileNotFoundException& e) {
                        QMessageBox::warning(this, "Ошибка", "Не удалось добавить фильм: " + QString(e.what()));
                    } catch (const MovieException& e) {
                        QMessageBox::warning(this, "Ошибка", "Не удалось добавить фильм: " + QString(e.what()));
                    }
                });
            } else {
                try {
                    manager.addMovieToFile(movie);
                    manager.reloadMovies();
                    populateAllMovies(manager.getAllMovies());
                    populateFavorites();
                    populateGenres();
                    statusbar->showMessage("Фильм добавлен в фильмотеку!", 3000);
                    QMessageBox::information(this, "Успех", "Фильм успешно добавлен в фильмотеку!");
                } catch (const DuplicateMovieException& e) {
                    QMessageBox::warning(this, "Дубликат фильма", QString::fromStdString(e.what()));
                    statusbar->showMessage("Фильм уже существует в фильмотеке", 3000);
                } catch (const FileNotFoundException& e) {
                    QMessageBox::warning(this, "Ошибка", "Не удалось добавить фильм: " + QString(e.what()));
                } catch (const MovieException& e) {
                    QMessageBox::warning(this, "Ошибка", "Не удалось добавить фильм: " + QString(e.what()));
                }
            }
        }
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
            QByteArray utf8Selected = selected.toUtf8();
            collManager->deleteCollection(std::string(utf8Selected.constData(), utf8Selected.length()));
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

