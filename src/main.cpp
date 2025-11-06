#include <QApplication>
#include <QSslSocket>
#include <QDebug>
#include <QImageReader>
#include <QDir>
#include "../includes/MainWindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    
    // Проверяем поддержку SSL
    if (!QSslSocket::supportsSsl()) {
        qDebug() << "SSL not supported. Available backends:" << QSslSocket::availableBackends();
        qDebug() << "SSL library version:" << QSslSocket::sslLibraryVersionString();
    } else {
        qDebug() << "SSL supported. Version:" << QSslSocket::sslLibraryVersionString();
    }
    
    // Проверяем доступные форматы изображений
    qDebug() << "Supported image formats:" << QImageReader::supportedImageFormats();
    qDebug() << "Image plugins path:" << QApplication::libraryPaths();
    
    // Добавляем путь к плагинам изображений, если он существует
    QString appDir = QApplication::applicationDirPath();
    QString imageFormatsPath = appDir + "/plugins/imageformats";
    if (QDir(imageFormatsPath).exists()) {
        QApplication::addLibraryPath(appDir);
        qDebug() << "Added image formats plugin path:" << imageFormatsPath;
    }
    
    MainWindow w;
    w.show();
    return app.exec();
}
