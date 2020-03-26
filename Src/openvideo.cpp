#include "openvideo.h"
#include "ui_openvideo.h"

#include <QFileDialog>
#include <QtDebug>
#include <QMessageBox>

#include <iostream>

openvideo::openvideo(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::openvideo)
{
    ui->setupUi(this);
}

openvideo::~openvideo()
{
    delete videoframe;
    delete ui;
}

void openvideo::on_videotoolButton_clicked()
{
#ifdef linux
    QString dirpath = QFileDialog::getExistingDirectory(this, tr("选择目录"), "/home", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
#else
    QString dirpath = QFileDialog::getExistingDirectory(this, tr("选择目录"), "C:/Users", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
#endif
    qInfo() << "videopath:" << dirpath;
    ui->videolineEdit->setText(dirpath);
}

void openvideo::on_outtoolButton_clicked()
{
#ifdef linux
    QString dirpath = QFileDialog::getExistingDirectory(this, tr("选择目录"), "/home", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
#else
    QString dirpath = QFileDialog::getExistingDirectory(this, tr("选择目录"), "C:/Users", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
#endif
    qInfo() << "outpath:" << dirpath;
    ui->outlineEdit->setText(dirpath);
}

void openvideo::on_OKpushButton_clicked()
{
    if(ui->outlineEdit->text().length() == 0 ||
       ui->videolineEdit->text().length() == 0)
    {
        QMessageBox::warning(this, "选择文件", "路径不完整");
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

    if(video.isOpened())
    {
        int div = ui->spinBox->value();
        int frame_count = video.get(cv::CAP_PROP_FRAME_COUNT);
        qInfo() << "CAP_PROP_FRAME_COUNT = " << frame_count;
        if(QMessageBox::question(this, "确认", QString::asprintf("视频共%d帧，抽取%d帧", frame_count, frame_count / div)) == QMessageBox::Yes) {
            videoframe = new VideoFrame(this, this);
        }
    } else {
        QMessageBox::warning(this, "错误", "打开文件失败");

        ui->outlineEdit->setDisabled(false);
        ui->videolineEdit->setDisabled(false);
        ui->outtoolButton->setDisabled(false);
        ui->videotoolButton->setDisabled(false);
        ui->spinBox->setDisabled(false);
        ui->OKpushButton->setDisabled(false);
    }
}

void openvideo::VideoFramecomplete() {
    QMessageBox::information(this, "完成", "视频已分解");

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
        if(count % div == 0)
        {
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
    QThread(parent), UIclass(uiclass)
{
    this->start();
    connect(this, SIGNAL(complete()), UIclass, SLOT(VideoFramecomplete()),
            Qt::QueuedConnection);
}

VideoFrame::~VideoFrame() {
    disconnect(this, SIGNAL(complete()), UIclass, SLOT(VideoFramecomplete()));
}
