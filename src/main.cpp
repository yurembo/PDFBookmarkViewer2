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
    app.setStyle(QStyleFactory::create("Fusion"));

    QPalette pal;
    pal.setColor(QPalette::Window, QColor(30, 30, 34));
    pal.setColor(QPalette::WindowText, Qt::white);
    pal.setColor(QPalette::Base, QColor(25, 25, 28));
    pal.setColor(QPalette::AlternateBase, QColor(35, 35, 40));
    pal.setColor(QPalette::Text, Qt::white);
    pal.setColor(QPalette::Button, QColor(45, 45, 48));
    pal.setColor(QPalette::ButtonText, Qt::white);
    pal.setColor(QPalette::Highlight, QColor(0, 120, 215));
    pal.setColor(QPalette::HighlightedText, Qt::white);
    app.setPalette(pal);

    QFont font("Segoe UI", 9);
    app.setFont(font);

    MainWindow window;
    window.show();

    return app.exec();
}
