#include <QApplication>
#include <QDir>
#include "../includes/MainWindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    
    QString appDir = QApplication::applicationDirPath();
    if (QString imageFormatsPath = appDir + "/plugins/imageformats"; QDir(imageFormatsPath).exists()) {
        QApplication::addLibraryPath(appDir);
    }
    
    MainWindow w;
    w.show();
    return QApplication::exec();
}
