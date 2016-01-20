#include "mainwindow.h"
#include <QApplication>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QString locale = QLocale::system().name().section('_', 0, 0);
    QTranslator translator;
    translator.load(QString(":/l10n/darkflow_") + locale);
    a.installTranslator(&translator);


    MainWindow w;
    if (argc == 2)
        w.load(argv[1]);
    w.show();

    return a.exec();
}
