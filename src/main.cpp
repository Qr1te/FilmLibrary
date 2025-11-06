#include <QApplication>
#include <QDir>
#include "../includes/MainWindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    
    QString appDir = QApplication::applicationDirPath();
    QString imageFormatsPath = appDir + "/plugins/imageformats";
    if (QDir(imageFormatsPath).exists()) {
        QApplication::addLibraryPath(appDir);
    }
    
    MainWindow w;
    w.show();
    return app.exec();
}
