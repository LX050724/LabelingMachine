#include "mainwindow.h"

#include <QApplication>
#include <QTranslator>
#include <QtCore>
#include <spdlogger.h>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOption({{"l", "log"}, "设置日志文件路径，默认不开启", "log"});
    parser.process(a);
    QString logFile = parser.value("log");
    if (!logFile.isEmpty()) {
        spdlogger::allLogger_logToFile(logFile.toStdString());
    }
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [thread %t] [%^%-8l%$]: %v");
    spdlogger logger(__FUNCTION__);
    logger.info("Build Time: {} {}", __DATE__, __TIME__);
#if defined(__DEBUG__)
    spdlog::set_level(spdlog::level::debug);
    logger.info("Build type: Debug");
#else
    logger.info("Build type: Release");
#endif
    auto *qtTranslator = new QTranslator;
    if (qtTranslator->load(":/new/prefix1/LabelingMachine_zh_CN.qm")) {
        a.installTranslator(qtTranslator);
    }
    MainWindow w;
    w.show();
    return a.exec();
}
