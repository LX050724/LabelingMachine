#include "mainwindow.h"

#include <QApplication>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QTranslator *qtTranslator = new QTranslator;
    if(qtTranslator->load("./LabelingMachine_zh_CN.qm")) {
        a.installTranslator(qtTranslator);
    } else qDebug("error");
    MainWindow w;
    w.show();
    return a.exec();
}
