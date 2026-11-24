#include "mainwindow.h"
#include <QApplication>
#include <QStyleFactory>
#include <QPalette>
#include <QFont>

int main(int argc, char *argv[])
{
    QApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

    QApplication app(argc, argv);

    // Тёмная тема Fusion
    // Fusion style blocks QFileDialog on macOS — removed
    // QPalette pal;
    // pal.setColor(QPalette::Window, QColor(30, 30, 34));
    // ...
    // QFont font("Segoe UI", 9);
    // app.setFont(font);

    MainWindow window;
    window.show();

    return app.exec();
}
