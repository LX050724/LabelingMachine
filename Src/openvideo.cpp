#include "openvideo.h"
#include "ui_openvideo.h"

#include <QFileDialog>
#include <QtDebug>
#include <QMessageBox>

#include <iostream>
#include <Inc/publicdefine.h>

openvideo::openvideo(QWidget *parent) :
        QDialog(parent),
        ui(new Ui::openvideo) {
    ui->setupUi(this);
}

openvideo::~openvideo() {
    delete videoframe;
    delete ui;
}

void openvideo::on_videotoolButton_clicked() {
    QString dirpath = QFileDialog::getOpenFileName(this, tr("Select the video"), HOME_PATH);

    qInfo() << "videopath:" << dirpath;
    ui->videolineEdit->setText(dirpath);
}

void openvideo::on_outtoolButton_clicked() {
    QString dirpath = QFileDialog::getExistingDirectory(this, tr("Select the directory"), HOME_PATH,
                                                        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    qInfo() << "outpath:" << dirpath;
    ui->outlineEdit->setText(dirpath);
}

void openvideo::on_OKpushButton_clicked() {
    if (ui->outlineEdit->text().length() == 0 ||
        ui->videolineEdit->text().length() == 0) {
        QMessageBox::warning(this, tr("Select the file"), tr("Path incomplete"));
        return;
    }

    ui->outlineEdit->setDisabled(true);
    ui->videolineEdit->setDisabled(true);
    ui->outtoolButton->setDisabled(true);
    ui->videotoolButton->setDisabled(true);
    ui->spinBox->setDisabled(true);
    ui->OKpushButton->setDisabled(true);

    std::string VideoPath = ui->videolineEdit->text().toStdString();
    std::string DirPath = ui->outlineEdit->text().toStdString();

    std::string VideoName = VideoPath.substr(VideoPath.find_last_of('/') + 1);
    VideoName = VideoName.substr(0, VideoName.find_last_of('.'));

    JPEGName = DirPath + '/' + VideoName + '_';

    qInfo() << VideoName.c_str();

    video.open(VideoPath);

    if (video.isOpened()) {
        int div = ui->spinBox->value();
        int frame_count = video.get(cv::CAP_PROP_FRAME_COUNT);
        qInfo() << "CAP_PROP_FRAME_COUNT = " << frame_count;
        auto button = QMessageBox::question(this, tr("confirm"),
                                            tr("There are") + QString::number(frame_count) +
                                            tr("frames of video,Extraction of") + QString::number(frame_count / div) +
                                            tr("frames"));
        if (button == QMessageBox::Yes) {
            videoframe = new VideoFrame(this, this);
        }
    } else {
        QMessageBox::warning(this, tr("error"), tr("File opening failed"));

        ui->outlineEdit->setDisabled(false);
        ui->videolineEdit->setDisabled(false);
        ui->outtoolButton->setDisabled(false);
        ui->videotoolButton->setDisabled(false);
        ui->spinBox->setDisabled(false);
        ui->OKpushButton->setDisabled(false);
    }
}

void openvideo::VideoFramecomplete() {
    QMessageBox::information(this, tr("complete"), tr("Video decomposition completed"));

    ui->outlineEdit->setDisabled(false);
    ui->videolineEdit->setDisabled(false);
    ui->outtoolButton->setDisabled(false);
    ui->videotoolButton->setDisabled(false);
    ui->spinBox->setDisabled(false);
    ui->OKpushButton->setDisabled(false);
}

void VideoFrame::run() {
    int count = 0;
    int frame_count = UIclass->video.get(cv::CAP_PROP_FRAME_COUNT);
    int div = UIclass->ui->spinBox->value();
    cv::Mat tmp;

    std::vector<int> FLAG = {
            cv::IMWRITE_JPEG_PROGRESSIVE, 1,
            cv::IMWRITE_JPEG_OPTIMIZE, 1
    };

    while (UIclass->video.read(tmp)) {
        if (count % div == 0) {
            char buffer[10];
            sprintf(buffer, "%05d", count);
            cv::imwrite(UIclass->JPEGName + buffer + ".jpg", tmp, FLAG);
        }
        ++count;
        UIclass->ui->progressBar->setValue(count * 100 / frame_count);
    }
    UIclass->video.release();
    emit complete();
}

VideoFrame::VideoFrame(QObject *parent, openvideo *uiclass) :
        QThread(parent), UIclass(uiclass) {
    this->start();
    connect(this, SIGNAL(complete()), UIclass, SLOT(VideoFramecomplete()),
            Qt::QueuedConnection);
}

VideoFrame::~VideoFrame() {
    disconnect(this, SIGNAL(complete()), UIclass, SLOT(VideoFramecomplete()));
}
