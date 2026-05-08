#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("Digital Design");
    app.setOrganizationName("DigitalDesign");

    MainWindow window;
    window.setWindowTitle("Digital Design");
    window.resize(1200, 800);
    window.show();

    return app.exec();
}
