#include "../includes/MovieCardFactory.h"
#include "../includes/MovieManager.h"
#include "../includes/PosterManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QMenu>
#include <QAction>
#include <QInputDialog>
#include <QMessageBox>
#include <QDialog>
#include <QTextEdit>
#include <QPoint>
#include <QStatusBar>
#include <QObject>
#include <QDesktopServices>
#include <QUrl>

MovieCardFactory::MovieCardFactory(MovieManager* manager, PosterManager* posterManager, QStatusBar* statusBar)
    : movieManager(manager), posterManager(posterManager), statusBar(statusBar) {
}

void MovieCardFactory::setOnFavoritesChanged(std::function<void()> callback) {
    onFavoritesChanged = callback;
}

void MovieCardFactory::setOnCollectionsChanged(std::function<void()> callback) {
    onCollectionsChanged = callback;
}

void MovieCardFactory::setOnMoviesChanged(std::function<void()> callback) {
    onMoviesChanged = callback;
}

void MovieCardFactory::setOnGenresChanged(std::function<void()> callback) {
    onGenresChanged = callback;
}

void MovieCardFactory::setOnShowInfo(std::function<void(const Movie&)> callback) {
    onShowInfo = callback;
}

QWidget* MovieCardFactory::createMovieCard(const Movie& movie, QWidget* parent) {
    if (!parent) {
        return nullptr;
    }
    
    QWidget* card = new QWidget(parent);
    card->setFixedSize(320, 600);
    card->setStyleSheet(
        "QWidget { background-color: #3a3a3a; border-radius: 8px; border: 1px solid #555; }"
        "QLabel { background-color: transparent; color: #e0e0e0; }"
    );

    QVBoxLayout* cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(10, 10, 10, 10);
    cardLayout->setSpacing(6);
    cardLayout->setAlignment(Qt::AlignTop);

    QLabel* posterLabel = new QLabel();
    posterLabel->setFixedSize(300, 450);
    posterLabel->setAlignment(Qt::AlignCenter);
    posterLabel->setScaledContents(false);
    posterLabel->setStyleSheet("background-color: #2b2b2b; border-radius: 4px;");
    posterManager->loadPosterToLabel(posterLabel, movie);
    cardLayout->addWidget(posterLabel, 0, Qt::AlignHCenter);
    cardLayout->addSpacing(4);

    QLabel* titleLabel = new QLabel(QString::fromStdString(movie.getTitle()));
    titleLabel->setStyleSheet("font-size: 15px; font-weight: bold; color: #ff6b35;");
    titleLabel->setWordWrap(true);
    titleLabel->setMaximumHeight(40);
    titleLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    cardLayout->addWidget(titleLabel);

    QString infoText = QString("%1, %2 • %3")
        .arg(movie.getYear())
        .arg(QString::fromStdString(movie.getCountry()))
        .arg(QString::fromStdString(movie.getGenreString()));
    QLabel* infoLabel = new QLabel(infoText);
    infoLabel->setStyleSheet("font-size: 11px; color: #bbb;");
    infoLabel->setWordWrap(true);
    infoLabel->setMaximumHeight(30);
    infoLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    cardLayout->addWidget(infoLabel);

    QString directorText = QString::fromStdString(movie.getDirector());
    if (directorText.isEmpty()) {
        directorText = "Режиссер: Не указано";
    } else {
        directorText = "Режиссер: " + directorText;
    }
    QLabel* directorLabel = new QLabel(directorText);
    directorLabel->setStyleSheet("font-size: 10px; color: #999;");
    directorLabel->setWordWrap(true);
    directorLabel->setMaximumHeight(25);
    directorLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    cardLayout->addWidget(directorLabel);

    QHBoxLayout* ratingButtonsLayout = new QHBoxLayout();
    QLabel* ratingLabel = new QLabel(QString::number(movie.getRating(), 'f', 1));
    ratingLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #46d369;");
    ratingButtonsLayout->addWidget(ratingLabel);
    ratingButtonsLayout->addStretch();

    QHBoxLayout* buttonsLayout = new QHBoxLayout();
    buttonsLayout->setSpacing(5);
    
    QPushButton* playBtn = new QPushButton("▶");
    playBtn->setFixedSize(35, 35);
    playBtn->setToolTip("Открыть на Кинопоиске");
    playBtn->setStyleSheet(
        "background-color: #46d369;"
        "color: white; font-size: 16px; font-weight: bold; border-radius: 4px; border: none;"
    );
    playBtn->setCursor(Qt::PointingHandCursor);
    
    QPushButton* favoriteBtn = new QPushButton("★");
    favoriteBtn->setFixedSize(35, 35);
    favoriteBtn->setToolTip("Добавить в избранное");
    bool isFav = movieManager->isFavorite(movie.getId());
    if (isFav) {
        favoriteBtn->setStyleSheet("background-color: #ffd700; color: #000; font-size: 18px; font-weight: bold; border-radius: 4px; border: none;");
    } else {
        favoriteBtn->setStyleSheet("background-color: #555; color: #ccc; font-size: 18px; border-radius: 4px; border: 1px solid #777;");
    }
    
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

    buttonsLayout->addWidget(playBtn);
    buttonsLayout->addWidget(favoriteBtn);
    buttonsLayout->addWidget(moreBtn);
    buttonsLayout->addWidget(infoBtn);
    ratingButtonsLayout->addLayout(buttonsLayout);
    cardLayout->addLayout(ratingButtonsLayout);
    
    cardLayout->addStretch();

    card->setProperty("movieId", movie.getId());
    
    QObject::connect(playBtn, &QPushButton::clicked, [movie]() {
        int movieId = movie.getId();
        if (movieId > 0) {
            QString url = QString("https://www.ggkinopoisk.ru/film/%1/").arg(movieId);
            QDesktopServices::openUrl(QUrl(url));
        }
    });
    
    QObject::connect(favoriteBtn, &QPushButton::clicked, [this, movie, favoriteBtn]() {
        try {
            if (movieManager->isFavorite(movie.getId())) {
                movieManager->removeFromFavorites(movie.getId());
                favoriteBtn->setStyleSheet("background-color: #555; color: #ccc; font-size: 18px; border-radius: 4px; border: 1px solid #777;");
                if (statusBar) statusBar->showMessage("Удалено из избранного", 2000);
            } else {
                movieManager->addToFavorites(movie.getId());
                favoriteBtn->setStyleSheet("background-color: #ffd700; color: #000; font-size: 18px; font-weight: bold; border-radius: 4px; border: none;");
                if (statusBar) statusBar->showMessage("Добавлено в избранное", 2000);
            }
            if (onFavoritesChanged) onFavoritesChanged();
        } catch (const std::exception& e) {
            QMessageBox::warning(nullptr, "Ошибка", e.what());
        }
    });

    QObject::connect(moreBtn, &QPushButton::clicked, [this, movie, moreBtn]() {
        QMenu menu;
        QAction* addToCollectionAction = menu.addAction("Добавить в коллекцию");
        
        auto* collManager = movieManager->getCollectionManager();
        QStringList collectionsWithMovie;
        if (collManager) {
            auto allCollections = movieManager->getAllCollectionNames();
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
            removeFromCollectionAction = menu.addAction("Удалить из коллекции");
        }
        
        menu.addSeparator();
        QAction* deleteMovieAction = menu.addAction("Удалить фильм");
        
        QAction* selectedAction = menu.exec(moreBtn->mapToGlobal(QPoint(0, moreBtn->height())));
        
        if (selectedAction == addToCollectionAction) {
            auto collections = movieManager->getAllCollectionNames();
            if (collections.empty()) {
                QMessageBox::information(nullptr, "Коллекции", "Сначала создайте коллекцию через меню.");
            } else {
                QStringList items;
                for (const auto& name : collections) {
                    items << QString::fromUtf8(name.c_str(), name.length());
                }
                bool ok;
                QString selected = QInputDialog::getItem(nullptr, "Выбор коллекции",
                                                        "Выберите коллекцию:", items, 0, false, &ok);
                if (ok && !selected.isEmpty()) {
                    if (collManager) {
                        QByteArray utf8Selected = selected.toUtf8();
                        MovieCollection* collection = collManager->getCollection(std::string(utf8Selected.constData(), utf8Selected.length()));
                        if (collection) {
                            try {
                                collection->addMovie(movie);
                                QMessageBox::information(nullptr, "Успех", 
                                    QString("Фильм добавлен в коллекцию '%1'").arg(selected));
                                if (onCollectionsChanged) onCollectionsChanged();
                            } catch (const std::exception& e) {
                                QMessageBox::warning(nullptr, "Ошибка", e.what());
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
                            QMessageBox::information(nullptr, "Успех", 
                                QString("Фильм удален из коллекции '%1'").arg(collectionName));
                            if (onCollectionsChanged) onCollectionsChanged();
                        } catch (const std::exception& e) {
                            QMessageBox::warning(nullptr, "Ошибка", e.what());
                        }
                    }
                }
            } else {
                bool ok;
                QString selected = QInputDialog::getItem(nullptr, "Удаление из коллекции",
                                                        "Выберите коллекцию, из которой удалить фильм:", 
                                                        collectionsWithMovie, 0, false, &ok);
                if (ok && !selected.isEmpty()) {
                    if (collManager) {
                        QByteArray utf8Selected = selected.toUtf8();
                        MovieCollection* collection = collManager->getCollection(std::string(utf8Selected.constData(), utf8Selected.length()));
                        if (collection) {
                            try {
                                collection->removeMovie(movie.getId());
                                QMessageBox::information(nullptr, "Успех", 
                                    QString("Фильм удален из коллекции '%1'").arg(selected));
                                if (onCollectionsChanged) onCollectionsChanged();
                            } catch (const std::exception& e) {
                                QMessageBox::warning(nullptr, "Ошибка", e.what());
                            }
                        }
                    }
                }
            }
        } else if (selectedAction == deleteMovieAction) {
            int ret = QMessageBox::question(nullptr, "Удаление фильма",
                                            QString("Вы уверены, что хотите удалить '%1'?\n\nЭто действие нельзя отменить.")
                                            .arg(QString::fromStdString(movie.getTitle())),
                                            QMessageBox::Yes | QMessageBox::No,
                                            QMessageBox::No);
            
            if (ret == QMessageBox::Yes) {
                try {
                    int movieId = movie.getId();
                    movieManager->removeMovie(movieId);
                    movieManager->reloadMovies();
                    
                    if (onMoviesChanged) onMoviesChanged();
                    if (onGenresChanged) onGenresChanged();
                    
                    if (statusBar) {
                        statusBar->showMessage(QString("Фильм '%1' успешно удален")
                                              .arg(QString::fromStdString(movie.getTitle())), 3000);
                    }
                } catch (const std::exception& e) {
                    QMessageBox::warning(nullptr, "Ошибка", QString("Не удалось удалить фильм: %1").arg(e.what()));
                }
            }
        }
    });

    QObject::connect(infoBtn, &QPushButton::clicked, [this, movie]() {
        if (onShowInfo) {
            onShowInfo(movie);
        }
    });

    return card;
}

