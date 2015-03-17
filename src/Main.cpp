#include <QApplication>
#include <QDesktopWidget>
#include <QIcon>

#include "MainWindow.hpp"

int main(int argc, char* argv[]) {
    Q_INIT_RESOURCE(DriverStationDisplay);

    QApplication app(argc, argv);
    app.setOrganizationName("FRC Team 3512");
    app.setApplicationName("DriverStationDisplay");

    QRect screenDims = QApplication::desktop()->screenGeometry();

    MainWindow mainWin(screenDims.width(), screenDims.height() - 240);
    mainWin.setWindowIcon(QIcon(":/images/Spartatroniks.ico"));
    mainWin.setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    mainWin.show();

    return app.exec();
}

