#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#include <Inc/publicdefine.h>
#include "Inc/module.h"
#include "ui_module.h"
#include "labelingmaching.h"

module::module(QWidget *parent, LabelingMaching *project) :
        QDialog(parent),
        pProject(project),
        ui(new Ui::module) {
    ui->setupUi(this);
    QString modulepath;
    QString outputpath;

    modulepath = QFileDialog::getOpenFileName(this, tr("Select module"), "./Mods", "Mod (*.exe)");
    if (modulepath.isEmpty())
        return;
    outputpath = QFileDialog::getExistingDirectory(this, tr("Select Output Path"), HOME_PATH);
    if (outputpath.isEmpty())
        return;

    Ready = true;
    QStringList Arguments;
    Arguments << "--ProjectPath";
    Arguments << pProject->getProjectPath();
    Arguments << "--XmlPath";
    Arguments << pProject->getXmlPath();
    Arguments << "--ImgPath";
    Arguments << pProject->getImgPath();
    Arguments << "--OutputPath";
    Arguments << outputpath;

    Process = new QProcess(this);
    Process->setProgram(modulepath);
    Process->setArguments(Arguments);
    if (!Process->open()) {
        QMessageBox::warning(this, tr("error"), tr("Create Process failed"));
        this->close();
    }

    connect(Process, &QProcess::readyReadStandardOutput,
            this, &module::QProcess_readyReadStandardOutput);

    connect(Process, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(QProcess_finished(int, QProcess::ExitStatus)));
}

module::~module() {
    if (Ready) {
        disconnect(Process, &QProcess::readyReadStandardOutput,
                   this, &module::QProcess_readyReadStandardOutput);

        disconnect(Process, SIGNAL(finished(int, QProcess::ExitStatus)),
                   this, SLOT(QProcess_finished(int, QProcess::ExitStatus)));
        Process->kill();
    }
    delete ui;
}

void module::QProcess_readyReadStandardOutput() {
    while (Process->bytesAvailable() > 0) {
        QString Line(Process->readLine(100));
        ui->textBrowser->append(Line);
    }
}

void module::QProcess_finished(int exitCode, QProcess::ExitStatus exitStatus) {
    ui->textBrowser->append(QString::asprintf("exit %d %s", exitCode,
                                              exitStatus == 0 ? "NormalExit" : "CrashExit"));
}

void module::on_pushButton_clicked() {
    this->close();
}

