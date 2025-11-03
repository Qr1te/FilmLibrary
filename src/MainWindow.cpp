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

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    setupModelsAndViews();
    populateGenres();
    
    // Проверяем, есть ли фильмы
    if (manager.getMoviesCount() == 0) {
        // Если нет фильмов, создаем примерный файл
        try {
            manager.createSampleMoviesFile();
        } catch (...) {
            // Игнорируем ошибки
        }
    }
    
    populateAllMovies(manager.getAllMovies());
    populateFavorites();
    populateCollections();
    updateStats();

    connect(ui->searchButton, &QPushButton::clicked, this, &MainWindow::handleSearch);
    connect(ui->resetFiltersButton, &QPushButton::clicked, this, &MainWindow::handleReset);

    connect(ui->actionCreateSample, &QAction::triggered, this, &MainWindow::handleCreateSample);
    connect(ui->actionSortByRating, &QAction::triggered, this, &MainWindow::handleSortByRating);
    connect(ui->actionTopN, &QAction::triggered, this, &MainWindow::handleShowTopN);
    connect(ui->actionRefresh, &QAction::triggered, this, &MainWindow::handleRefresh);
    connect(ui->actionCreateCollection, &QAction::triggered, this, &MainWindow::handleCreateCollection);
    connect(ui->actionManageCollections, &QAction::triggered, this, &MainWindow::handleManageCollections);
    connect(ui->collectionComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::handleCollectionChanged);

    // Удалены соединения для таблиц, так как теперь используем карточки
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::setupModelsAndViews() {
    // Установка серо-оранжевой цветовой схемы
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
        // Стили для детальной страницы
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
    // Collect unique genres from all movies (поддержка множественных жанров)
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
    // Создаем карточку фильма
    if (!parent) {
        parent = ui->scrollAreaWidgetContentsAll;  // По умолчанию для All Movies
    }
    QWidget* card = new QWidget(parent);  // Указываем родителя
    card->setFixedSize(320, 580);
    card->setStyleSheet(
        "QWidget { background-color: #3a3a3a; border-radius: 8px; border: 1px solid #555; }"
        "QLabel { background-color: transparent; color: #e0e0e0; }"
    );

    QVBoxLayout* cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(10, 10, 10, 10);
    cardLayout->setSpacing(8);

    // Постер
    QLabel* posterLabel = new QLabel();
    QString posterPath = QString::fromStdString(movie.getPosterPath());
    
    // Получаем директорию исполняемого файла
    QString appDir = QApplication::applicationDirPath();
    QString currentDir = QDir::currentPath();
    
    // Извлекаем имя файла из пути
    QString fileName = QFileInfo(posterPath).fileName();
    
    // Отладочная информация (только для первого фильма, чтобы не засорять вывод)
    static int posterDebugCount = 0;
    bool debugThis = (posterDebugCount++ < 2);
    
    if (debugThis) {
        qDebug() << "=== Poster search #" << posterDebugCount << "===";
        qDebug() << "Movie:" << QString::fromStdString(movie.getTitle());
        qDebug() << "File name:" << fileName;
        qDebug() << "App dir:" << appDir;
        qDebug() << "Current dir:" << currentDir;
    }
    
    // Пробуем разные пути для поиска постеров
    QStringList searchDirs;
    QString sep = QDir::separator();
    
    // Преобразуем пути с / в правильные для Windows
    auto addDir = [&searchDirs](const QString& path) {
        QString normalized = QDir::cleanPath(path);
        if (!normalized.isEmpty() && !searchDirs.contains(normalized)) {
            searchDirs << normalized;
        }
    };
    
    // Добавляем все возможные варианты путей
    addDir(appDir + sep + "posters");  // posters рядом с exe (самый частый случай)
    addDir(appDir + sep + ".." + sep + "posters");  // posters на уровень выше
    addDir(appDir + sep + ".." + sep + ".." + sep + "posters");  // posters на два уровня выше
    addDir(currentDir + sep + "posters");  // posters в текущей директории
    addDir(currentDir + sep + ".." + sep + "posters");  // posters на уровень выше от текущей
    addDir("posters");  // posters в текущей директории (короткий путь)
    addDir(".." + sep + "posters");  // Относительно рабочей директории
    addDir(".." + sep + ".." + sep + "posters");  // Еще выше
    addDir(appDir);  // Директория exe (может быть posters рядом)
    
    // Также пробуем абсолютные пути от корня проекта
    QDir projectRoot = QDir(currentDir);
    if (projectRoot.cdUp()) {  // Переходим на уровень выше
        addDir(projectRoot.absolutePath() + sep + "posters");
    }
    
    // Убираем дубликаты и несуществующие директории
    QStringList validDirs;
    for (const QString& dir : searchDirs) {
        QDir testDir(dir);
        if (testDir.exists()) {
            QString absPath = testDir.absolutePath();
            if (!validDirs.contains(absPath)) {
                validDirs << absPath;
                if (debugThis) {
                    qDebug() << "Found posters dir:" << absPath;
                    // Показываем файлы в этой директории для отладки (первые 3)
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
    
    // Сначала пробуем найти файл по точному пути
    QStringList possiblePaths;
    possiblePaths << posterPath;
    possiblePaths << appDir + sep + posterPath;
    possiblePaths << appDir + sep + ".." + sep + posterPath;
    possiblePaths << appDir + sep + ".." + sep + ".." + sep + posterPath;
    possiblePaths << currentDir + sep + posterPath;
    possiblePaths << "posters" + sep + fileName;
    possiblePaths << ".." + sep + "posters" + sep + fileName;
    
    // Также пробуем все найденные директории posters
    for (const QString& dir : searchDirs) {
        possiblePaths << dir + sep + fileName;
    }
    
    for (const QString& path : possiblePaths) {
        QString normalizedPath = QDir::cleanPath(path);
        // Преобразуем в абсолютный путь
        if (!QDir::isAbsolutePath(normalizedPath)) {
            normalizedPath = QDir(currentDir).absoluteFilePath(normalizedPath);
        }
        // Проверяем существование
        if (QFile::exists(normalizedPath)) {
            fullPath = QDir::cleanPath(normalizedPath);
            found = true;
            if (debugThis) {
                qDebug() << "Found by exact path:" << fullPath;
            }
            break;
        }
    }
    
    // Если не нашли по точному пути, ищем по имени файла в разных директориях
    if (!found && !searchDirs.isEmpty()) {
        for (const QString& dir : searchDirs) {
            QDir searchDir(dir);
            if (searchDir.exists()) {
                QString testPath;
                // Пробуем точное имя файла (только если fileName не пустой)
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
                
                // Если имя файла содержит путь (например, "posters/film.png"), пробуем без "posters/"
                QString simpleFileName = fileName;
                if (simpleFileName.contains(sep)) {
                    simpleFileName = QFileInfo(simpleFileName).fileName();
                }
                testPath = searchDir.absoluteFilePath(simpleFileName);
                if (QFile::exists(testPath)) {
                    fullPath = testPath;
                    found = true;
                    // qDebug() << "Found poster by simple name:" << fullPath;
                    break;
                }
                
                // Пробуем найти файлы, содержащие название фильма
                QString movieTitle = QString::fromStdString(movie.getTitle());
                QFileInfoList files = searchDir.entryInfoList(QStringList() << "*.png" << "*.jpg" << "*.jpeg", 
                                                               QDir::Files);
                for (const QFileInfo& fileInfo : files) {
                    QString baseName = fileInfo.baseName();
                    // Проверяем, содержит ли имя файла название фильма (без учета регистра и пробелов)
                    QString normalizedBaseName = baseName.toLower().remove(' ').remove('-').remove('_');
                    QString normalizedTitle = movieTitle.toLower().remove(' ').remove('-').remove('_');
                    
                    // Более гибкое сравнение
                    if (normalizedBaseName == normalizedTitle ||
                        normalizedBaseName.contains(normalizedTitle) || 
                        normalizedTitle.contains(normalizedBaseName) ||
                        baseName.toLower().contains(movieTitle.toLower()) ||
                        movieTitle.toLower().contains(baseName.toLower())) {
                        fullPath = fileInfo.absoluteFilePath();
                        found = true;
                        // qDebug() << "Found poster by title match:" << fullPath;
                        break;
                    }
                }
                if (found) break;
            }
        }
    }
    
    // Если все еще не нашли, пробуем прямой поиск всех PNG файлов
    if (!found) {
        for (const QString& dir : searchDirs) {
            QDir searchDir(dir);
            if (searchDir.exists()) {
                QStringList filters;
                filters << "*.png" << "*.PNG" << "*.jpg" << "*.JPG" << "*.jpeg" << "*.JPEG";
                QFileInfoList files = searchDir.entryInfoList(filters, QDir::Files);
                // Если в папке только один файл или мало файлов, пробуем взять первый
                // Это не идеально, но может помочь для тестирования
                if (!files.isEmpty() && files.size() <= 100) {
                    // Пробуем найти хотя бы какой-то постер для теста
                    fullPath = files.first().absoluteFilePath();
                    found = true;
                    break;
                }
            }
        }
    }
    
    // Преобразуем найденный путь в абсолютный перед использованием
    if (found && !fullPath.isEmpty()) {
        // Убеждаемся, что путь абсолютный
        if (!QDir::isAbsolutePath(fullPath)) {
            // Если это относительный путь, пробуем разные базовые директории
            QString testPath1 = QDir(currentDir).absoluteFilePath(fullPath);
            QString testPath2 = QDir(appDir).absoluteFilePath(fullPath);
            if (QFile::exists(testPath1)) {
                fullPath = testPath1;
            } else if (QFile::exists(testPath2)) {
                fullPath = testPath2;
            } else {
                fullPath = testPath1;  // Используем по умолчанию
            }
        }
        fullPath = QDir::cleanPath(fullPath);
        
        if (QFile::exists(fullPath)) {
            if (debugThis) {
                qDebug() << "Loading pixmap from:" << fullPath;
                qDebug() << "File exists:" << QFile::exists(fullPath);
                qDebug() << "File size:" << QFileInfo(fullPath).size();
            }
            // Используем QImageReader для более надежной загрузки с диагностикой ошибок
            QString absolutePath = QFileInfo(fullPath).absoluteFilePath();
            
            QImage image;
            bool loaded = false;
            
            // Метод 1: QImageReader (наиболее надежный)
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
            
            // Метод 2: Прямой QImage конструктор
            if (!loaded) {
                image = QImage(absolutePath);
                if (!image.isNull()) {
                    loaded = true;
                    if (debugThis) {
                        qDebug() << "Success via QImage constructor! Original size:" << image.size();
                    }
                }
            }
            
            // Метод 3: Чтение файла и loadFromData с явным указанием формата
            if (!loaded) {
                QFile file(absolutePath);
                if (file.open(QIODevice::ReadOnly)) {
                    QByteArray imageData = file.readAll();
                    file.close();
                    
                    // Пробуем разные форматы
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
                        // Пробуем без указания формата (автоопределение)
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
            
            // Если загрузили, отображаем
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

    // Название
    QLabel* titleLabel = new QLabel(QString::fromStdString(movie.getTitle()));
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #ff6b35;");
    titleLabel->setWordWrap(true);
    cardLayout->addWidget(titleLabel);

    // Информация (год, страна, жанр)
    QString infoText = QString("%1, %2 • %3")
        .arg(movie.getYear())
        .arg(QString::fromStdString(movie.getCountry()))
        .arg(QString::fromStdString(movie.getGenreString()));
    QLabel* infoLabel = new QLabel(infoText);
    infoLabel->setStyleSheet("font-size: 12px; color: #bbb;");
    infoLabel->setWordWrap(true);
    cardLayout->addWidget(infoLabel);

    // Режиссер
    QLabel* directorLabel = new QLabel(QString("Режиссёр: %1").arg(QString::fromStdString(movie.getDirector())));
    directorLabel->setStyleSheet("font-size: 11px; color: #bbb;");
    directorLabel->setWordWrap(true);
    cardLayout->addWidget(directorLabel);

    // Рейтинг
    QHBoxLayout* ratingLayout = new QHBoxLayout();
    QLabel* ratingLabel = new QLabel(QString::number(movie.getRating(), 'f', 1));
    ratingLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #46d369;");
    ratingLayout->addWidget(ratingLabel);
    ratingLayout->addStretch();
    cardLayout->addLayout(ratingLayout);

    // Кнопки
    QHBoxLayout* buttonsLayout = new QHBoxLayout();
    
    // Кнопка избранного
    QPushButton* favoriteBtn = new QPushButton("★");
    favoriteBtn->setFixedSize(35, 35);
    favoriteBtn->setToolTip("Добавить в избранное");
    // Проверяем, уже ли фильм в избранном
    bool isFav = manager.isFavorite(movie.getId());
    if (isFav) {
        favoriteBtn->setStyleSheet("background-color: #ffd700; color: #000; font-size: 18px; font-weight: bold; border-radius: 4px; border: none;");
    } else {
        favoriteBtn->setStyleSheet("background-color: #555; color: #ccc; font-size: 18px; border-radius: 4px; border: 1px solid #777;");
    }
    
    // Кнопка дополнительных опций
    QPushButton* moreBtn = new QPushButton("⋯");
    moreBtn->setFixedSize(35, 35);
    moreBtn->setToolTip("Дополнительные опции");
    moreBtn->setStyleSheet("background-color: #555; color: #ccc; font-size: 22px; border-radius: 4px; border: 1px solid #777;");
    
    QPushButton* infoBtn = new QPushButton("Информация");
    infoBtn->setStyleSheet(
        "background-color: #ff6b35;"
        "color: white; font-weight: bold; padding: 8px; border-radius: 4px; border: none;"
        "min-width: 100px;"
    );

    buttonsLayout->addWidget(favoriteBtn);
    buttonsLayout->addWidget(moreBtn);
    buttonsLayout->addWidget(infoBtn);
    cardLayout->addLayout(buttonsLayout);

    // Сохраняем ID фильма в карточке для дальнейшего использования
    card->setProperty("movieId", movie.getId());

    // Подключаем сигналы
    connect(favoriteBtn, &QPushButton::clicked, [this, movie, favoriteBtn]() {
        try {
            if (manager.isFavorite(movie.getId())) {
                manager.removeFromFavorites(movie.getId());
                favoriteBtn->setStyleSheet("background-color: #555; color: #ccc; font-size: 18px; border-radius: 4px; border: 1px solid #777;");
                ui->statusbar->showMessage("Удалено из избранного", 2000);
            } else {
                manager.addToFavorites(movie.getId());
                favoriteBtn->setStyleSheet("background-color: #ffd700; color: #000; font-size: 18px; font-weight: bold; border-radius: 4px; border: none;");
                ui->statusbar->showMessage("Добавлено в избранное", 2000);
            }
            updateStats();
            populateFavorites();  // Обновляем вкладку избранных
        } catch (const std::exception& e) {
            QMessageBox::warning(this, "Ошибка", e.what());
        }
    });

    connect(moreBtn, &QPushButton::clicked, [this, movie, moreBtn]() {
        QMenu menu;
        QAction* addToCollectionAction = menu.addAction("Добавить в коллекцию");
        
        // Проверяем, в каких коллекциях находится фильм
        auto* collManager = manager.getCollectionManager();
        QStringList collectionsWithMovie;
        if (collManager) {
            auto allCollections = manager.getAllCollectionNames();
            for (const auto& name : allCollections) {
                MovieCollection* collection = collManager->getCollection(name);
                if (collection && collection->containsMovie(movie.getId())) {
                    collectionsWithMovie << QString::fromStdString(name);
                }
            }
        }
        
        QAction* removeFromCollectionAction = nullptr;
        if (!collectionsWithMovie.isEmpty()) {
            menu.addSeparator();
            removeFromCollectionAction = menu.addAction("Удалить из коллекции");
        }
        
        menu.addSeparator();
        QAction* viewDetailsAction = menu.addAction("Подробности");
        QAction* selectedAction = menu.exec(moreBtn->mapToGlobal(QPoint(0, moreBtn->height())));
        
        if (selectedAction == addToCollectionAction) {
            // Получаем список коллекций
            auto collections = manager.getAllCollectionNames();
            if (collections.empty()) {
                QMessageBox::information(this, "Коллекции", "Сначала создайте коллекцию через меню.");
            } else {
                QStringList items;
                for (const auto& name : collections) {
                    items << QString::fromStdString(name);
                }
                bool ok;
                QString selected = QInputDialog::getItem(this, "Выбор коллекции",
                                                        "Выберите коллекцию:", items, 0, false, &ok);
                if (ok && !selected.isEmpty()) {
                    if (collManager) {
                        MovieCollection* collection = collManager->getCollection(selected.toStdString());
                        if (collection) {
                            try {
                                collection->addMovie(movie);
                                QMessageBox::information(this, "Успех", 
                                    QString("Фильм добавлен в коллекцию '%1'").arg(selected));
                                populateCollections();  // Обновляем список коллекций
                                handleCollectionChanged();  // Обновляем текущую коллекцию
                            } catch (const std::exception& e) {
                                QMessageBox::warning(this, "Ошибка", e.what());
                            }
                        }
                    }
                }
            }
        } else if (selectedAction == removeFromCollectionAction) {
            // Показываем список коллекций, из которых можно удалить
            if (collectionsWithMovie.size() == 1) {
                // Если фильм только в одной коллекции, удаляем сразу
                QString collectionName = collectionsWithMovie.first();
                if (collManager) {
                    MovieCollection* collection = collManager->getCollection(collectionName.toStdString());
                    if (collection) {
                        try {
                            collection->removeMovie(movie.getId());
                            QMessageBox::information(this, "Успех", 
                                QString("Фильм удален из коллекции '%1'").arg(collectionName));
                            populateCollections();
                            handleCollectionChanged();
                        } catch (const std::exception& e) {
                            QMessageBox::warning(this, "Ошибка", e.what());
                        }
                    }
                }
            } else {
                // Если фильм в нескольких коллекциях, предлагаем выбрать
                bool ok;
                QString selected = QInputDialog::getItem(this, "Удаление из коллекции",
                                                        "Выберите коллекцию, из которой удалить фильм:", 
                                                        collectionsWithMovie, 0, false, &ok);
                if (ok && !selected.isEmpty()) {
                    if (collManager) {
                        MovieCollection* collection = collManager->getCollection(selected.toStdString());
                        if (collection) {
                            try {
                                collection->removeMovie(movie.getId());
                                QMessageBox::information(this, "Успех", 
                                    QString("Фильм удален из коллекции '%1'").arg(selected));
                                populateCollections();
                                handleCollectionChanged();
                            } catch (const std::exception& e) {
                                QMessageBox::warning(this, "Ошибка", e.what());
                            }
                        }
                    }
                }
            }
        } else if (selectedAction == viewDetailsAction) {
            updateDetailsFromSelection(movie.getId());
            ui->tabWidget->setCurrentWidget(ui->tabDetails);
        }
    });

    connect(infoBtn, &QPushButton::clicked, [this, movie]() {
        updateDetailsFromSelection(movie.getId());
        ui->tabWidget->setCurrentWidget(ui->tabDetails);
    });

    connect(card, &QWidget::destroyed, card, [card]() {});

    return card;
}

void MainWindow::populateAllMovies(const std::vector<Movie>& movies) {
    // Проверяем, что layout существует
    if (!ui->gridLayoutMovies) {
        return;
    }
    
    // Очищаем существующие карточки
    QLayoutItem* item;
    while ((item = ui->gridLayoutMovies->takeAt(0)) != nullptr) {
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }

    // Создаем новые карточки
    const int columns = 4;  // 4 карточки в ряд
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
    // Проверяем, что layout существует
    if (!ui->gridLayoutFavorites) {
        return;
    }
    
    auto favs = manager.getFavoriteMovies();
    
    // Очищаем существующие карточки
    QLayoutItem* item;
    while ((item = ui->gridLayoutFavorites->takeAt(0)) != nullptr) {
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }

    // Создаем новые карточки для избранных фильмов
    const int columns = 4;  // 4 карточки в ряд
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
    // Обновляем список коллекций в ComboBox
    ui->collectionComboBox->clear();
    auto collectionNames = manager.getAllCollectionNames();
    
    for (const auto& name : collectionNames) {
        ui->collectionComboBox->addItem(QString::fromStdString(name));
    }
    
    // Если есть коллекции, показываем первую
    if (ui->collectionComboBox->count() > 0) {
        handleCollectionChanged();
    } else {
        // Очищаем layout если нет коллекций
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
    // Проверяем, что layout существует
    if (!ui->gridLayoutCollections) {
        return;
    }
    
    // Получаем выбранную коллекцию
    QString collectionName = ui->collectionComboBox->currentText();
    if (collectionName.isEmpty()) {
        // Очищаем layout если нет выбранной коллекции
        QLayoutItem* item;
        while ((item = ui->gridLayoutCollections->takeAt(0)) != nullptr) {
            if (item->widget()) {
                item->widget()->deleteLater();
            }
            delete item;
        }
        return;
    }
    
    // Получаем коллекцию
    auto* collManager = manager.getCollectionManager();
    if (!collManager) {
        return;
    }
    
    MovieCollection* collection = collManager->getCollection(collectionName.toStdString());
    if (!collection) {
        return;
    }
    
    // Получаем фильмы из коллекции
    auto movies = collection->getMovies();
    
    // Очищаем существующие карточки
    QLayoutItem* item;
    while ((item = ui->gridLayoutCollections->takeAt(0)) != nullptr) {
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }

    // Создаем новые карточки для фильмов из коллекции
    const int columns = 4;  // 4 карточки в ряд
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
    
    // Список директорий для поиска
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
    
    // Убираем несуществующие директории
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
    
    // Пробуем найти файл
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
    
    // Если не нашли, ищем по имени файла
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
            
            // Поиск по названию фильма
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
    
    // Загружаем постер
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
                // Масштабируем постер для детальной страницы (больше, чем в карточках)
                pixmap = pixmap.scaled(350, 525, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                label->setPixmap(pixmap);
                label->setAlignment(Qt::AlignCenter);
                label->setStyleSheet("border-radius: 4px;");
                return;
            }
        }
    }
    
    // Если постер не найден
    label->setText("No Poster\n" + QString::fromStdString(movie.getTitle()));
    label->setStyleSheet("background-color: #4a4a4a; color: #999; border-radius: 4px;");
    label->setAlignment(Qt::AlignCenter);
}

void MainWindow::updateDetailsFromSelection(int movieId) {
    const Movie* m = manager.findMovieById(movieId);
    if (!m) {
        ui->titleValue->setText("-");
        ui->ratingValue->setText("-");
        ui->yearValue->setText("-");
        ui->genreValue->setText("-");
        ui->directorValue->setText("-");
        ui->descriptionText->clear();
        ui->posterLabel->setText("No Poster");
        ui->posterLabel->setPixmap(QPixmap());
        return;
    }
    ui->titleValue->setText(QString::fromStdString(m->getTitle()));
    ui->ratingValue->setText(QString::number(m->getRating(), 'f', 1));
    ui->yearValue->setText(QString::number(m->getYear()));
    ui->genreValue->setText(QString::fromStdString(m->getGenreString()));
    ui->directorValue->setText(QString::fromStdString(m->getDirector()));
    ui->descriptionText->setPlainText(QString::fromStdString(m->getDescription()));
    loadPosterToLabel(ui->posterLabel, *m);
}

void MainWindow::updateStats() {
    ui->statsLabel->setText(
        QString("Total movies: %1 | Favorites: %2")
            .arg(static_cast<qint64>(manager.getMoviesCount()))
            .arg(static_cast<qint64>(manager.getFavoritesCount())));
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
        updateStats();
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
    // Теперь выбор происходит через клик по карточке
    // Этот метод может быть использован для получения ID из кликнутой карточки
    return -1;  // Реализовано через сигналы карточек
}

int MainWindow::selectedMovieIdFromFavorites() const {
    return -1;  // Реализовано через сигналы карточек
}


void MainWindow::handleRefresh() {
    try {
        // Re-load movies from file implicitly via createSample or simply repopulate current memory
        populateAllMovies(manager.getAllMovies());
        populateFavorites();
        populateCollections();
        populateGenres();
        updateStats();
        ui->statusbar->showMessage("Refreshed", 1500);
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Refresh", e.what());
    }
}

void MainWindow::handleAllSelectionChanged() {
    // Теперь выбор происходит через клики на карточки
}

void MainWindow::handleFavSelectionChanged() {
    // Теперь выбор происходит через клики на карточки
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
        MovieCollection* collection = manager.createCollection(name.toStdString());
        QMessageBox::information(this, "Success",
                                 QString("Collection '%1' created successfully!").arg(name));
        ui->statusbar->showMessage(QString("Collection '%1' created").arg(name), 3000);
        populateCollections();  // Обновляем список коллекций
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
        names << QString::fromStdString(name);
    }

    bool ok;
    QString selected = QInputDialog::getItem(this, "Manage Collections",
                                            "Select collection:", names, 0, false, &ok);
    if (!ok || selected.isEmpty()) {
        return;
    }

    // Получаем коллекцию
    auto* collManager = manager.getCollectionManager();
    if (!collManager) {
        QMessageBox::warning(this, "Error", "Collection manager not available");
        return;
    }

    MovieCollection* collection = collManager->getCollection(selected.toStdString());
    if (!collection) {
        QMessageBox::warning(this, "Error", "Collection not found");
        return;
    }

    // Диалог для управления коллекцией
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
                populateCollections();  // Обновляем отображение коллекций
                handleCollectionChanged();  // Обновляем текущую коллекцию
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
            populateCollections();  // Обновляем отображение коллекций
            handleCollectionChanged();  // Обновляем текущую коллекцию
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
                collManager->deleteCollection(selected.toStdString());
                QMessageBox::information(this, "Success",
                                         QString("Collection '%1' deleted").arg(selected));
                ui->statusbar->showMessage("Collection deleted", 2000);
                populateCollections();  // Обновляем список коллекций
            } catch (const std::exception& e) {
                QMessageBox::warning(this, "Error", e.what());
            }
        }
    }
}


