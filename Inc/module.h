#ifndef MODULE_H
#define MODULE_H

#include <QDialog>
#include <QProcess>

#include "labelingmaching.h"

namespace Ui {
    class module;
}

class module : public QDialog {
Q_OBJECT
    LabelingMaching *pProject;
    QProcess *Process;
    volatile bool Ready = false;
public:
    explicit module(QWidget *parent = nullptr, LabelingMaching *project = nullptr);

    ~module();

    inline bool isReady() const { return Ready; }

private slots:

    void on_pushButton_clicked();

    void QProcess_readyReadStandardOutput();

    void QProcess_finished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    Ui::module *ui;
};

#endif // MODULE_H
