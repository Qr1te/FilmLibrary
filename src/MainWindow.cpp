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

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    setupModelsAndViews();
    populateGenres();
    
    if (manager.getMoviesCount() == 0) {
        try {
            manager.createSampleMoviesFile();
        } catch (...) {
        }
    }
    
    populateAllMovies(manager.getAllMovies());
    populateFavorites();
    populateCollections();
    populatePlayer();

    connect(ui->searchButton, &QPushButton::clicked, this, &MainWindow::handleSearch);
    connect(ui->resetFiltersButton, &QPushButton::clicked, this, &MainWindow::handleReset);

    connect(ui->actionCreateSample, &QAction::triggered, this, &MainWindow::handleCreateSample);
    connect(ui->actionSortByRating, &QAction::triggered, this, &MainWindow::handleSortByRating);
    connect(ui->actionTopN, &QAction::triggered, this, &MainWindow::handleShowTopN);
    connect(ui->actionRefresh, &QAction::triggered, this, &MainWindow::handleRefresh);
    connect(ui->actionCreateCollection, &QAction::triggered, this, &MainWindow::handleCreateCollection);
    connect(ui->actionManageCollections, &QAction::triggered, this, &MainWindow::handleManageCollections);
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

QWidget* MainWindow::createMovieCard(const Movie& movie, QWidget* parent) {
    if (!parent) {
        parent = ui->scrollAreaWidgetContentsAll;
    }
    QWidget* card = new QWidget(parent);
    card->setFixedSize(320, 580);
    card->setStyleSheet(
        "QWidget { background-color: #3a3a3a; border-radius: 8px; border: 1px solid #555; }"
        "QLabel { background-color: transparent; color: #e0e0e0; }"
    );

    QVBoxLayout* cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(10, 10, 10, 10);
    cardLayout->setSpacing(8);

    QLabel* posterLabel = new QLabel();
    QString posterPath = QString::fromStdString(movie.getPosterPath());
    
    QString appDir = QApplication::applicationDirPath();
    QString currentDir = QDir::currentPath();
    
    QString fileName = QFileInfo(posterPath).fileName();
    static int posterDebugCount = 0;
    bool debugThis = (posterDebugCount++ < 2);
    
    if (debugThis) {
        qDebug() << "=== Poster search #" << posterDebugCount << "===";
        qDebug() << "Movie:" << QString::fromStdString(movie.getTitle());
        qDebug() << "File name:" << fileName;
        qDebug() << "App dir:" << appDir;
        qDebug() << "Current dir:" << currentDir;
    }
    
    QStringList searchDirs;
    QString sep = QDir::separator();
    auto addDir = [&searchDirs](const QString& path) {
        QString normalized = QDir::cleanPath(path);
        if (!normalized.isEmpty() && !searchDirs.contains(normalized)) {
            searchDirs << normalized;
        }
    };
    
    addDir(appDir + sep + "posters");
    addDir(appDir + sep + ".." + sep + "posters");
    addDir(appDir + sep + ".." + sep + ".." + sep + "posters");
    addDir(currentDir + sep + "posters");
    addDir(currentDir + sep + ".." + sep + "posters");
    addDir("posters");
    addDir(".." + sep + "posters");
    addDir(".." + sep + ".." + sep + "posters");
    addDir(appDir);
    
    QDir projectRoot = QDir(currentDir);
    if (projectRoot.cdUp()) {
        addDir(projectRoot.absolutePath() + sep + "posters");
    }
    QStringList validDirs;
    for (const QString& dir : searchDirs) {
        QDir testDir(dir);
        if (testDir.exists()) {
            QString absPath = testDir.absolutePath();
            if (!validDirs.contains(absPath)) {
                validDirs << absPath;
                if (debugThis) {
                    qDebug() << "Found posters dir:" << absPath;
                    QFileInfoList files = testDir.entryInfoList(QStringList() << "*.png", QDir::Files);
                    if (files.size() > 0) {
                        qDebug() << "  Files in dir:" << files.size() << "First:" << files.first().fileName();
                    }
                }
            }
        }
    }
    searchDirs = validDirs;
    
    if (debugThis) {
        if (searchDirs.isEmpty()) {
            qDebug() << "WARNING: No posters directories found!";
        } else {
            qDebug() << "Found" << searchDirs.size() << "poster directories";
            for (const QString& d : searchDirs) {
                qDebug() << "  -" << d;
            }
        }
    }
    
    QString fullPath;
    bool found = false;
    
    QStringList possiblePaths;
    possiblePaths << posterPath;
    possiblePaths << appDir + sep + posterPath;
    possiblePaths << appDir + sep + ".." + sep + posterPath;
    possiblePaths << appDir + sep + ".." + sep + ".." + sep + posterPath;
    possiblePaths << currentDir + sep + posterPath;
    possiblePaths << "posters" + sep + fileName;
    possiblePaths << ".." + sep + "posters" + sep + fileName;
    
    for (const QString& dir : searchDirs) {
        possiblePaths << dir + sep + fileName;
    }
    
    for (const QString& path : possiblePaths) {
        QString normalizedPath = QDir::cleanPath(path);
        if (!QDir::isAbsolutePath(normalizedPath)) {
            normalizedPath = QDir(currentDir).absoluteFilePath(normalizedPath);
        }
        if (QFile::exists(normalizedPath)) {
            fullPath = QDir::cleanPath(normalizedPath);
            found = true;
            if (debugThis) {
                qDebug() << "Found by exact path:" << fullPath;
            }
            break;
        }
    }
    
    if (!found && !searchDirs.isEmpty()) {
        for (const QString& dir : searchDirs) {
            QDir searchDir(dir);
            if (searchDir.exists()) {
                QString testPath;
                if (!fileName.isEmpty()) {
                    testPath = searchDir.absoluteFilePath(fileName);
                    if (debugThis) {
                        qDebug() << "Trying:" << testPath << "exists:" << QFile::exists(testPath);
                    }
                    if (QFile::exists(testPath)) {
                        fullPath = QDir::cleanPath(testPath);
                        found = true;
                        if (debugThis) {
                            qDebug() << "*** FOUND:" << fullPath;
                        }
                        break;
                    }
                }
                
                QString simpleFileName = fileName;
                if (simpleFileName.contains(sep)) {
                    simpleFileName = QFileInfo(simpleFileName).fileName();
                }
                testPath = searchDir.absoluteFilePath(simpleFileName);
                if (QFile::exists(testPath)) {
                    fullPath = testPath;
                    found = true;
                    break;
                }
                
                QString movieTitle = QString::fromStdString(movie.getTitle());
                QFileInfoList files = searchDir.entryInfoList(QStringList() << "*.png" << "*.jpg" << "*.jpeg", 
                                                               QDir::Files);
                for (const QFileInfo& fileInfo : files) {
                    QString baseName = fileInfo.baseName();
                    QString normalizedBaseName = baseName.toLower().remove(' ').remove('-').remove('_');
                    QString normalizedTitle = movieTitle.toLower().remove(' ').remove('-').remove('_');
                    if (normalizedBaseName == normalizedTitle ||
                        normalizedBaseName.contains(normalizedTitle) || 
                        normalizedTitle.contains(normalizedBaseName) ||
                        baseName.toLower().contains(movieTitle.toLower()) ||
                        movieTitle.toLower().contains(baseName.toLower())) {
                        fullPath = fileInfo.absoluteFilePath();
                        found = true;
                        break;
                    }
                }
                if (found) break;
            }
        }
    }
    
    if (!found) {
        for (const QString& dir : searchDirs) {
            QDir searchDir(dir);
            if (searchDir.exists()) {
                QStringList filters;
                filters << "*.png" << "*.PNG" << "*.jpg" << "*.JPG" << "*.jpeg" << "*.JPEG";
                QFileInfoList files = searchDir.entryInfoList(filters, QDir::Files);
                if (!files.isEmpty() && files.size() <= 100) {
                    fullPath = files.first().absoluteFilePath();
                    found = true;
                    break;
                }
            }
        }
    }
    
    if (found && !fullPath.isEmpty()) {
        if (!QDir::isAbsolutePath(fullPath)) {
            QString testPath1 = QDir(currentDir).absoluteFilePath(fullPath);
            QString testPath2 = QDir(appDir).absoluteFilePath(fullPath);
            if (QFile::exists(testPath1)) {
                fullPath = testPath1;
            } else if (QFile::exists(testPath2)) {
                fullPath = testPath2;
            } else {
                fullPath = testPath1;
            }
        }
        fullPath = QDir::cleanPath(fullPath);
        
        if (QFile::exists(fullPath)) {
            if (debugThis) {
                qDebug() << "Loading pixmap from:" << fullPath;
                qDebug() << "File exists:" << QFile::exists(fullPath);
                qDebug() << "File size:" << QFileInfo(fullPath).size();
            }
            QString absolutePath = QFileInfo(fullPath).absoluteFilePath();
            
            QImage image;
            bool loaded = false;
            QImageReader reader(absolutePath);
            if (reader.canRead()) {
                reader.setAutoTransform(true);
                image = reader.read();
                if (!image.isNull()) {
                    loaded = true;
                    if (debugThis) {
                        qDebug() << "Success via QImageReader! Original size:" << image.size();
                        qDebug() << "Format:" << reader.format();
                    }
                } else {
                    if (debugThis) {
                        qDebug() << "QImageReader::read() returned null. Error:" << reader.errorString();
                    }
                }
            } else {
                if (debugThis) {
                    qDebug() << "QImageReader::canRead() returned false. Error:" << reader.errorString();
                }
            }
            
            if (!loaded) {
                image = QImage(absolutePath);
                if (!image.isNull()) {
                    loaded = true;
                    if (debugThis) {
                        qDebug() << "Success via QImage constructor! Original size:" << image.size();
                    }
                }
            }
            
            if (!loaded) {
                QFile file(absolutePath);
                if (file.open(QIODevice::ReadOnly)) {
                    QByteArray imageData = file.readAll();
                    file.close();
                    
                    QStringList formats = {"PNG", "JPEG", "JPG", "BMP", "GIF"};
                    for (const QString& fmt : formats) {
                        QImage image2;
                        if (image2.loadFromData(imageData, fmt.toUtf8().constData())) {
                            image = image2;
                            loaded = true;
                            if (debugThis) {
                                qDebug() << "Success via loadFromData with format" << fmt << "! Original size:" << image.size();
                            }
                            break;
                        }
                    }
                    
                    if (!loaded) {
                        if (image.loadFromData(imageData)) {
                            loaded = true;
                            if (debugThis) {
                                qDebug() << "Success via loadFromData (auto format)! Original size:" << image.size();
                            }
                        } else {
                            if (debugThis) {
                                qDebug() << "ERROR: All loadFromData attempts failed. Path:" << fullPath;
                                qDebug() << "Data size:" << imageData.size() << "bytes";
                                qDebug() << "First 16 bytes (hex):" << imageData.left(16).toHex();
                            }
                        }
                    }
                } else {
                    if (debugThis) {
                        qDebug() << "ERROR: Cannot open file for reading. Path:" << fullPath;
                        qDebug() << "File readable:" << QFileInfo(fullPath).isReadable();
                    }
                }
            }
            
            if (loaded && !image.isNull()) {
                QPixmap pixmap = QPixmap::fromImage(image);
                pixmap = pixmap.scaled(300, 440, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                posterLabel->setPixmap(pixmap);
                posterLabel->setStyleSheet("border-radius: 4px;");
            } else {
                if (debugThis) {
                    qDebug() << "ERROR: All loading methods failed. Path:" << fullPath;
                }
                posterLabel->setText("No Poster\n" + QString::fromStdString(movie.getTitle()));
                posterLabel->setStyleSheet("background-color: #ccc; min-height: 380px; border-radius: 4px;");
            }
        } else {
            if (debugThis) {
                qDebug() << "ERROR: File does not exist:" << fullPath;
            }
            posterLabel->setText("No Poster\n" + QString::fromStdString(movie.getTitle()));
            posterLabel->setStyleSheet("background-color: #ccc; min-height: 380px; border-radius: 4px;");
        }
    } else {
        if (debugThis) {
            qDebug() << "ERROR: Poster not found. File name:" << fileName;
            if (fileName.isEmpty()) {
                qDebug() << "  Poster path was empty or invalid:" << posterPath;
            }
        }
        posterLabel->setText("No Poster\n" + QString::fromStdString(movie.getTitle()));
        posterLabel->setStyleSheet("background-color: #4a4a4a; color: #999; min-height: 440px; border-radius: 4px;");
    }
    posterLabel->setAlignment(Qt::AlignCenter);
    posterLabel->setWordWrap(true);
    posterLabel->setScaledContents(false);
    cardLayout->addWidget(posterLabel);

    QLabel* titleLabel = new QLabel(QString::fromStdString(movie.getTitle()));
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #ff6b35;");
    titleLabel->setWordWrap(true);
    cardLayout->addWidget(titleLabel);

    QString infoText = QString("%1, %2 • %3")
        .arg(movie.getYear())
        .arg(QString::fromStdString(movie.getCountry()))
        .arg(QString::fromStdString(movie.getGenreString()));
    QLabel* infoLabel = new QLabel(infoText);
    infoLabel->setStyleSheet("font-size: 12px; color: #bbb;");
    infoLabel->setWordWrap(true);
    cardLayout->addWidget(infoLabel);

    QLabel* directorLabel = new QLabel(QString("Director: %1").arg(QString::fromStdString(movie.getDirector())));
    directorLabel->setStyleSheet("font-size: 11px; color: #bbb;");
    directorLabel->setWordWrap(true);
    cardLayout->addWidget(directorLabel);

    QHBoxLayout* ratingLayout = new QHBoxLayout();
    QLabel* ratingLabel = new QLabel(QString::number(movie.getRating(), 'f', 1));
    ratingLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #46d369;");
    ratingLayout->addWidget(ratingLabel);
    ratingLayout->addStretch();
    cardLayout->addLayout(ratingLayout);

    QHBoxLayout* buttonsLayout = new QHBoxLayout();
    QPushButton* favoriteBtn = new QPushButton("★");
    favoriteBtn->setFixedSize(35, 35);
    favoriteBtn->setToolTip("Add to favorites");
    bool isFav = manager.isFavorite(movie.getId());
    if (isFav) {
        favoriteBtn->setStyleSheet("background-color: #ffd700; color: #000; font-size: 18px; font-weight: bold; border-radius: 4px; border: none;");
    } else {
        favoriteBtn->setStyleSheet("background-color: #555; color: #ccc; font-size: 18px; border-radius: 4px; border: 1px solid #777;");
    }
    
    QPushButton* moreBtn = new QPushButton("⋯");
    moreBtn->setFixedSize(35, 35);
    moreBtn->setToolTip("More options");
    moreBtn->setStyleSheet("background-color: #555; color: #ccc; font-size: 22px; border-radius: 4px; border: 1px solid #777;");
    
    QPushButton* infoBtn = new QPushButton("Info");
    infoBtn->setStyleSheet(
        "background-color: #ff6b35;"
        "color: white; font-weight: bold; padding: 8px; border-radius: 4px; border: none;"
        "min-width: 100px;"
    );

    buttonsLayout->addWidget(favoriteBtn);
    buttonsLayout->addWidget(moreBtn);
    buttonsLayout->addWidget(infoBtn);
    cardLayout->addLayout(buttonsLayout);

    card->setProperty("movieId", movie.getId());
    connect(favoriteBtn, &QPushButton::clicked, [this, movie, favoriteBtn]() {
        try {
            if (manager.isFavorite(movie.getId())) {
                manager.removeFromFavorites(movie.getId());
                favoriteBtn->setStyleSheet("background-color: #555; color: #ccc; font-size: 18px; border-radius: 4px; border: 1px solid #777;");
                ui->statusbar->showMessage("Removed from favorites", 2000);
            } else {
                manager.addToFavorites(movie.getId());
                favoriteBtn->setStyleSheet("background-color: #ffd700; color: #000; font-size: 18px; font-weight: bold; border-radius: 4px; border: none;");
                ui->statusbar->showMessage("Added to favorites", 2000);
            }
            populateFavorites();
        } catch (const std::exception& e) {
            QMessageBox::warning(this, "Error", e.what());
        }
    });

    connect(moreBtn, &QPushButton::clicked, [this, movie, moreBtn]() {
        QMenu menu;
        QAction* addToCollectionAction = menu.addAction("Add to collection");
        
        auto* collManager = manager.getCollectionManager();
        QStringList collectionsWithMovie;
        if (collManager) {
            auto allCollections = manager.getAllCollectionNames();
            for (const auto& name : allCollections) {
                MovieCollection* collection = collManager->getCollection(name);
                if (collection && collection->containsMovie(movie.getId())) {
                    collectionsWithMovie << QString::fromUtf8(name.c_str(), name.length());
                }
            }
        }
        
        QAction* removeFromCollectionAction = nullptr;
        if (!collectionsWithMovie.isEmpty()) {
            menu.addSeparator();
            removeFromCollectionAction = menu.addAction("Remove from collection");
        }
        
        menu.addSeparator();
        QAction* selectedAction = menu.exec(moreBtn->mapToGlobal(QPoint(0, moreBtn->height())));
        
        if (selectedAction == addToCollectionAction) {
            auto collections = manager.getAllCollectionNames();
            if (collections.empty()) {
                QMessageBox::information(this, "Collections", "Please create a collection first through the menu.");
            } else {
                QStringList items;
                for (const auto& name : collections) {
                    items << QString::fromUtf8(name.c_str(), name.length());
                }
                bool ok;
                QString selected = QInputDialog::getItem(this, "Select Collection",
                                                        "Choose collection:", items, 0, false, &ok);
                if (ok && !selected.isEmpty()) {
                    if (collManager) {
                        QByteArray utf8Selected = selected.toUtf8();
                        MovieCollection* collection = collManager->getCollection(std::string(utf8Selected.constData(), utf8Selected.length()));
                        if (collection) {
                            try {
                                collection->addMovie(movie);
                                QMessageBox::information(this, "Success", 
                                    QString("Movie added to collection '%1'").arg(selected));
                                populateCollections();
                                handleCollectionChanged();
                            } catch (const std::exception& e) {
                                QMessageBox::warning(this, "Error", e.what());
                            }
                        }
                    }
                }
            }
        } else if (selectedAction == removeFromCollectionAction) {
            if (collectionsWithMovie.size() == 1) {
                QString collectionName = collectionsWithMovie.first();
                if (collManager) {
                    MovieCollection* collection = collManager->getCollection(collectionName.toStdString());
                    if (collection) {
                        try {
                            collection->removeMovie(movie.getId());
                            QMessageBox::information(this, "Success", 
                                QString("Movie removed from collection '%1'").arg(collectionName));
                            populateCollections();
                            handleCollectionChanged();
                        } catch (const std::exception& e) {
                            QMessageBox::warning(this, "Error", e.what());
                        }
                    }
                }
            } else {
                bool ok;
                QString selected = QInputDialog::getItem(this, "Remove from Collection",
                                                        "Select collection to remove movie from:", 
                                                        collectionsWithMovie, 0, false, &ok);
                if (ok && !selected.isEmpty()) {
                    if (collManager) {
                        QByteArray utf8Selected = selected.toUtf8();
                        MovieCollection* collection = collManager->getCollection(std::string(utf8Selected.constData(), utf8Selected.length()));
                        if (collection) {
                            try {
                                collection->removeMovie(movie.getId());
                                QMessageBox::information(this, "Success", 
                                    QString("Movie removed from collection '%1'").arg(selected));
                                populateCollections();
                                handleCollectionChanged();
                            } catch (const std::exception& e) {
                                QMessageBox::warning(this, "Error", e.what());
                            }
                        }
                    }
                }
            }
        }
    });

    connect(infoBtn, &QPushButton::clicked, [this, movie]() {
        QDialog* dialog = new QDialog(this);
        dialog->setWindowTitle("Movie Information");
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
        loadPosterToLabel(posterLabel, movie);
        mainLayout->addWidget(posterLabel);
        
        QVBoxLayout* infoLayout = new QVBoxLayout();
        infoLayout->setSpacing(10);
        
        QLabel* titleLabel = new QLabel(QString::fromStdString(movie.getTitle()));
        titleLabel->setStyleSheet("font-size: 20pt; font-weight: bold; color: #ff6b35;");
        titleLabel->setWordWrap(true);
        infoLayout->addWidget(titleLabel);
        
        QLabel* ratingLabel = new QLabel(QString("Rating: %1").arg(movie.getRating(), 0, 'f', 1));
        ratingLabel->setStyleSheet("font-size: 14pt; font-weight: bold; color: #46d369;");
        infoLayout->addWidget(ratingLabel);
        
        QLabel* yearLabel = new QLabel(QString("Year: %1").arg(movie.getYear()));
        yearLabel->setStyleSheet("font-size: 12pt; color: #e0e0e0;");
        infoLayout->addWidget(yearLabel);
        
        QLabel* genreLabel = new QLabel(QString("Genre: %1").arg(QString::fromStdString(movie.getGenreString())));
        genreLabel->setStyleSheet("font-size: 12pt; color: #e0e0e0;");
        genreLabel->setWordWrap(true);
        infoLayout->addWidget(genreLabel);
        
        QLabel* directorLabel = new QLabel(QString("Director: %1").arg(QString::fromStdString(movie.getDirector())));
        directorLabel->setStyleSheet("font-size: 12pt; color: #e0e0e0;");
        directorLabel->setWordWrap(true);
        infoLayout->addWidget(directorLabel);
        
        QLabel* countryLabel = new QLabel(QString("Country: %1").arg(QString::fromStdString(movie.getCountry())));
        countryLabel->setStyleSheet("font-size: 12pt; color: #e0e0e0;");
        countryLabel->setWordWrap(true);
        infoLayout->addWidget(countryLabel);
        
        QLabel* actorsLabel = new QLabel(QString("Actors: %1").arg(QString::fromStdString(movie.getActors())));
        actorsLabel->setStyleSheet("font-size: 12pt; color: #e0e0e0;");
        actorsLabel->setWordWrap(true);
        infoLayout->addWidget(actorsLabel);
        
        QString durationText = QString("%1 min").arg(movie.getDuration());
        QLabel* durationLabel = new QLabel(QString("Duration: %1").arg(durationText));
        durationLabel->setStyleSheet("font-size: 12pt; color: #e0e0e0;");
        infoLayout->addWidget(durationLabel);
        
        QLabel* descTitleLabel = new QLabel("Description:");
        descTitleLabel->setStyleSheet("font-size: 12pt; font-weight: bold; color: #bbb; margin-top: 10px;");
        infoLayout->addWidget(descTitleLabel);
        
        QTextEdit* descriptionText = new QTextEdit();
        descriptionText->setPlainText(QString::fromStdString(movie.getDescription()));
        descriptionText->setReadOnly(true);
        descriptionText->setMaximumHeight(150);
        infoLayout->addWidget(descriptionText);
        
        infoLayout->addStretch();
        
        QPushButton* closeBtn = new QPushButton("OK");
        closeBtn->setCursor(Qt::PointingHandCursor);
        connect(closeBtn, &QPushButton::clicked, dialog, &QDialog::accept);
        infoLayout->addWidget(closeBtn, 0, Qt::AlignRight);
        
        mainLayout->addLayout(infoLayout);
        
        dialog->exec();
        delete dialog;
    });

    connect(card, &QWidget::destroyed, card, [card]() {});

    return card;
}

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
        QWidget* card = createMovieCard(movie);
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
        QWidget* card = createMovieCard(movie, ui->scrollAreaWidgetContentsFavorites);
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
        QWidget* card = createMovieCard(movie, ui->scrollAreaWidgetContentsCollections);
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

void MainWindow::loadPosterToLabel(QLabel* label, const Movie& movie) {
    if (!label) return;
    
    QString posterPath = QString::fromStdString(movie.getPosterPath());
    QString appDir = QApplication::applicationDirPath();
    QString currentDir = QDir::currentPath();
    QString fileName = QFileInfo(posterPath).fileName();
    QString sep = QDir::separator();
    
    QStringList searchDirs;
    auto addDir = [&searchDirs](const QString& path) {
        QString normalized = QDir::cleanPath(path);
        if (!normalized.isEmpty() && !searchDirs.contains(normalized)) {
            searchDirs << normalized;
        }
    };
    
    addDir(appDir + sep + "posters");
    addDir(appDir + sep + ".." + sep + "posters");
    addDir(appDir + sep + ".." + sep + ".." + sep + "posters");
    addDir(currentDir + sep + "posters");
    addDir(currentDir + sep + ".." + sep + "posters");
    addDir("posters");
    addDir(".." + sep + "posters");
    addDir(".." + sep + ".." + sep + "posters");
    addDir(appDir);
    
    QStringList validDirs;
    for (const QString& dir : searchDirs) {
        QDir testDir(dir);
        if (testDir.exists()) {
            QString absPath = testDir.absolutePath();
            if (!validDirs.contains(absPath)) {
                validDirs << absPath;
            }
        }
    }
    
    QString fullPath;
    bool found = false;
    
    QStringList possiblePaths;
    possiblePaths << posterPath;
    possiblePaths << appDir + sep + posterPath;
    possiblePaths << currentDir + sep + posterPath;
    possiblePaths << "posters" + sep + fileName;
    
    for (const QString& dir : validDirs) {
        possiblePaths << dir + sep + fileName;
    }
    
    for (const QString& path : possiblePaths) {
        QString normalizedPath = QDir::cleanPath(path);
        if (!QDir::isAbsolutePath(normalizedPath)) {
            normalizedPath = QDir(currentDir).absoluteFilePath(normalizedPath);
        }
        if (QFile::exists(normalizedPath)) {
            fullPath = QDir::cleanPath(normalizedPath);
            found = true;
            break;
        }
    }
    
    if (!found && !validDirs.isEmpty()) {
        for (const QString& dir : validDirs) {
            QDir searchDir(dir);
            if (!fileName.isEmpty()) {
                QString testPath = searchDir.absoluteFilePath(fileName);
                if (QFile::exists(testPath)) {
                    fullPath = QDir::cleanPath(testPath);
                    found = true;
                    break;
                }
            }
            
            QString movieTitle = QString::fromStdString(movie.getTitle());
            QFileInfoList files = searchDir.entryInfoList(QStringList() << "*.png" << "*.jpg" << "*.jpeg", QDir::Files);
            for (const QFileInfo& fileInfo : files) {
                QString baseName = fileInfo.baseName();
                QString normalizedBaseName = baseName.toLower().remove(' ').remove('-').remove('_');
                QString normalizedTitle = movieTitle.toLower().remove(' ').remove('-').remove('_');
                
                if (normalizedBaseName == normalizedTitle ||
                    normalizedBaseName.contains(normalizedTitle) || 
                    normalizedTitle.contains(normalizedBaseName)) {
                    fullPath = fileInfo.absoluteFilePath();
                    found = true;
                    break;
                }
            }
            if (found) break;
        }
    }
    
    if (found && !fullPath.isEmpty()) {
        if (!QDir::isAbsolutePath(fullPath)) {
            QString testPath1 = QDir(currentDir).absoluteFilePath(fullPath);
            QString testPath2 = QDir(appDir).absoluteFilePath(fullPath);
            if (QFile::exists(testPath1)) {
                fullPath = testPath1;
            } else if (QFile::exists(testPath2)) {
                fullPath = testPath2;
            }
        }
        fullPath = QDir::cleanPath(fullPath);
        
        if (QFile::exists(fullPath)) {
            QImageReader reader(QFileInfo(fullPath).absoluteFilePath());
            QImage image;
            
            if (reader.canRead()) {
                reader.setAutoTransform(true);
                image = reader.read();
            }
            
            if (image.isNull()) {
                image = QImage(fullPath);
            }
            
            if (!image.isNull()) {
                QPixmap pixmap = QPixmap::fromImage(image);
                QSize labelSize = label->size();
                // Если размер еще не установлен, используем maximumSize или minimumSize
                if (labelSize.width() <= 0 || labelSize.height() <= 0) {
                    labelSize = label->maximumSize();
                    if (labelSize.width() <= 0 || labelSize.height() <= 0) {
                        labelSize = label->minimumSize();
                    }
                    // Если и они не установлены, используем дефолтный размер
                    if (labelSize.width() <= 0 || labelSize.height() <= 0) {
                        labelSize = QSize(450, 675);
                    }
                }
                pixmap = pixmap.scaled(labelSize.width(), labelSize.height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
                label->setPixmap(pixmap);
                label->setAlignment(Qt::AlignCenter);
                label->setStyleSheet("border-radius: 4px;");
                return;
            }
        }
    }
    
    label->setText("No Poster\n" + QString::fromStdString(movie.getTitle()));
    label->setStyleSheet("background-color: #4a4a4a; color: #999; border-radius: 4px;");
    label->setAlignment(Qt::AlignCenter);
}


void MainWindow::handleSearch() {
    try {
        const QString title = ui->searchLineEdit->text().trimmed();
        const QString genre = ui->genreComboBox->currentText().trimmed();

        std::vector<Movie> base = manager.getAllMovies();
        std::vector<Movie> filtered = base;

        if (!title.isEmpty()) {
            filtered = manager.searchByTitleResults(title.toStdString());
        }
        if (!genre.isEmpty()) {
            auto byGenre = manager.searchByGenreResults(genre.toStdString());
            if (title.isEmpty()) {
                filtered = byGenre;
            } else {

                std::vector<Movie> intersection;
                for (const auto& m : filtered) {
                    for (const auto& g : byGenre) {
                        if (m.getId() == g.getId()) { intersection.push_back(m); break; }
                    }
                }
                filtered = std::move(intersection);
            }
        }
        populateAllMovies(filtered);
        ui->statusbar->showMessage("Search applied", 2000);
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Search error", e.what());
    }
}

void MainWindow::handleReset() {
    ui->searchLineEdit->clear();
    ui->genreComboBox->setCurrentIndex(0);
    populateAllMovies(manager.getAllMovies());
    ui->statusbar->showMessage("Filters reset", 2000);
}

void MainWindow::handleCreateSample() {
    try {
        manager.createSampleMoviesFile();
        populateAllMovies(manager.getAllMovies());
        populateFavorites();
        populateGenres();
        ui->statusbar->showMessage("Sample file created", 3000);
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Error", e.what());
    }
}

void MainWindow::handleSortByRating() {
    try {
        manager.sortByRating();
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


void MainWindow::handleRefresh() {
    try {
        populateAllMovies(manager.getAllMovies());
        populateFavorites();
        populateCollections();
        populateGenres();
        ui->statusbar->showMessage("Refreshed", 1500);
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Refresh", e.what());
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

void MainWindow::populatePlayer() {
    if (!ui->gridLayoutPlayer) {
        return;
    }
    
    QLayoutItem* item;
    while ((item = ui->gridLayoutPlayer->takeAt(0)) != nullptr) {
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }
    
    QString appDir = QApplication::applicationDirPath();
    QString currentDir = QDir::currentPath();
    QString sep = QDir::separator();
    
    QStringList possiblePaths;
    possiblePaths << currentDir + sep + "filmstoplay";
    possiblePaths << appDir + sep + "filmstoplay";
    possiblePaths << appDir + sep + ".." + sep + "filmstoplay";
    possiblePaths << appDir + sep + ".." + sep + ".." + sep + "filmstoplay";
    possiblePaths << "filmstoplay";
    
    QString filmstoplayPath;
    bool found = false;
    
    for (const QString& path : possiblePaths) {
        QString normalizedPath = QDir::cleanPath(path);
        if (!QDir::isAbsolutePath(normalizedPath)) {
            normalizedPath = QDir(currentDir).absoluteFilePath(normalizedPath);
        }
        QDir dir(normalizedPath);
        if (dir.exists()) {
            filmstoplayPath = dir.absolutePath();
            found = true;
            break;
        }
    }
    
    if (!found) {
        ui->statusbar->showMessage("filmstoplay folder not found", 3000);
        return;
    }
    
    QDir dir(filmstoplayPath);
    QStringList filters;
    filters << "*.mp4" << "*.avi" << "*.mkv" << "*.mov" << "*.wmv" << "*.flv";
    QFileInfoList videoFiles = dir.entryInfoList(filters, QDir::Files);
    
    if (videoFiles.isEmpty()) {
        ui->statusbar->showMessage("Video files not found in filmstoplay folder", 3000);
        return;
    }
    
    const int columns = 4;
    int row = 0, col = 0;
    
    for (const QFileInfo& fileInfo : videoFiles) {
        QString fileName = fileInfo.baseName();
        QString filePath = fileInfo.absoluteFilePath();
        
        QWidget* card = new QWidget(ui->scrollAreaWidgetContentsPlayer);
        card->setFixedSize(320, 480);
        card->setStyleSheet(
            "QWidget { background-color: #3a3a3a; border-radius: 8px; border: 1px solid #555; }"
            "QLabel { background-color: transparent; color: #e0e0e0; }"
        );
        
        QVBoxLayout* cardLayout = new QVBoxLayout(card);
        cardLayout->setContentsMargins(10, 10, 10, 10);
        cardLayout->setSpacing(8);
        
        QLabel* posterLabel = new QLabel();
        posterLabel->setFixedSize(300, 360);
        loadPosterToLabelByTitle(posterLabel, fileName);
        cardLayout->addWidget(posterLabel);
        
        QLabel* titleLabel = new QLabel(fileName);
        titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #ff6b35;");
        titleLabel->setWordWrap(true);
        cardLayout->addWidget(titleLabel);
        
        QPushButton* playBtn = new QPushButton("▶ Play");
        playBtn->setStyleSheet(
            "background-color: #ff6b35;"
            "color: white; font-weight: bold; font-size: 14px; padding: 10px; border-radius: 4px; border: none;"
            "min-width: 100px;"
        );
        playBtn->setCursor(Qt::PointingHandCursor);
        
        playBtn->setProperty("videoPath", filePath);
        
        connect(playBtn, &QPushButton::clicked, [this, filePath]() {
            playVideoFile(filePath);
        });
        
        cardLayout->addWidget(playBtn);
        
        ui->gridLayoutPlayer->addWidget(card, row, col);
        
        col++;
        if (col >= columns) {
            col = 0;
            row++;
        }
    }
    
    ui->statusbar->showMessage(QString("Loaded %1 video files").arg(videoFiles.size()), 2000);
}

void MainWindow::loadPosterToLabelByTitle(QLabel* label, const QString& movieTitle) {
    if (!label) return;
    
    QString appDir = QApplication::applicationDirPath();
    QString currentDir = QDir::currentPath();
    QString sep = QDir::separator();
    
    QStringList searchDirs;
    auto addDir = [&searchDirs](const QString& path) {
        QString normalized = QDir::cleanPath(path);
        if (!normalized.isEmpty() && !searchDirs.contains(normalized)) {
            searchDirs << normalized;
        }
    };
    
    addDir(appDir + sep + "posters");
    addDir(appDir + sep + ".." + sep + "posters");
    addDir(appDir + sep + ".." + sep + ".." + sep + "posters");
    addDir(currentDir + sep + "posters");
    addDir(currentDir + sep + ".." + sep + "posters");
    addDir("posters");
    addDir(".." + sep + "posters");
    addDir(".." + sep + ".." + sep + "posters");
    addDir(appDir);
    
    QStringList validDirs;
    for (const QString& dir : searchDirs) {
        QDir testDir(dir);
        if (testDir.exists()) {
            QString absPath = testDir.absolutePath();
            if (!validDirs.contains(absPath)) {
                validDirs << absPath;
            }
        }
    }
    
    QString fullPath;
    bool found = false;
    
    if (!validDirs.isEmpty()) {
        for (const QString& dir : validDirs) {
            QDir searchDir(dir);
            QFileInfoList files = searchDir.entryInfoList(QStringList() << "*.png" << "*.jpg" << "*.jpeg", QDir::Files);
            for (const QFileInfo& fileInfo : files) {
                QString baseName = fileInfo.baseName();
                QString normalizedBaseName = baseName.toLower().remove(' ').remove('-').remove('_').remove('\'');
                QString normalizedTitle = movieTitle.toLower().remove(' ').remove('-').remove('_').remove('\'');
                
                if (normalizedBaseName == normalizedTitle ||
                    normalizedBaseName.contains(normalizedTitle) || 
                    normalizedTitle.contains(normalizedBaseName) ||
                    baseName.toLower().contains(movieTitle.toLower()) ||
                    movieTitle.toLower().contains(baseName.toLower())) {
                    fullPath = fileInfo.absoluteFilePath();
                    found = true;
                    break;
                }
            }
            if (found) break;
        }
    }
    
    if (found && !fullPath.isEmpty()) {
        if (!QDir::isAbsolutePath(fullPath)) {
            QString testPath1 = QDir(currentDir).absoluteFilePath(fullPath);
            QString testPath2 = QDir(appDir).absoluteFilePath(fullPath);
            if (QFile::exists(testPath1)) {
                fullPath = testPath1;
            } else if (QFile::exists(testPath2)) {
                fullPath = testPath2;
            }
        }
        fullPath = QDir::cleanPath(fullPath);
        
        if (QFile::exists(fullPath)) {
            QImageReader reader(QFileInfo(fullPath).absoluteFilePath());
            QImage image;
            
            if (reader.canRead()) {
                reader.setAutoTransform(true);
                image = reader.read();
            }
            
            if (image.isNull()) {
                image = QImage(fullPath);
            }
            
            if (!image.isNull()) {
                QPixmap pixmap = QPixmap::fromImage(image);
                pixmap = pixmap.scaled(300, 360, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                label->setPixmap(pixmap);
                label->setAlignment(Qt::AlignCenter);
                label->setStyleSheet("border-radius: 4px;");
                return;
            }
        }
    }
    
    label->setText("🎬\n" + movieTitle);
    label->setStyleSheet("background-color: #2b2b2b; color: #999; border-radius: 4px; border: 2px solid #555; min-height: 360px;");
    label->setAlignment(Qt::AlignCenter);
    label->setWordWrap(true);
}

void MainWindow::playVideoFile(const QString& filePath) {
    if (!QFile::exists(filePath)) {
        QMessageBox::warning(this, "Error", "File not found: " + filePath);
        return;
    }
    
    QUrl fileUrl = QUrl::fromLocalFile(filePath);
    bool success = QDesktopServices::openUrl(fileUrl);
    
    if (success) {
        ui->statusbar->showMessage("Playing video: " + QFileInfo(filePath).fileName(), 3000);
    } else {
        #ifdef Q_OS_WIN
        QProcess::startDetached("cmd", QStringList() << "/c" << "start" << "\"\"" << filePath);
        #elif defined(Q_OS_MAC)
        QProcess::startDetached("open", QStringList() << filePath);
        #else
        QProcess::startDetached("xdg-open", QStringList() << filePath);
        #endif
        ui->statusbar->showMessage("Playing video: " + QFileInfo(filePath).fileName(), 3000);
    }
}


