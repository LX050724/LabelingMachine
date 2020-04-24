#ifndef OPENVIDEO_H
#define OPENVIDEO_H

#include <QDialog>
#include <QThread>
#include <opencv2/opencv.hpp>

namespace Ui {
    class openvideo;
}

class VideoFrame;

class openvideo : public QDialog {
Q_OBJECT
    friend class VideoFrame;
    cv::VideoCapture video;
    std::string JPEGName;
    VideoFrame *videoframe = nullptr;
public:
    explicit openvideo(QWidget *parent = nullptr);

    ~openvideo();

private slots:

    void on_videotoolButton_clicked();

    void on_outtoolButton_clicked();

    void on_OKpushButton_clicked();

    void VideoFramecomplete();

private:
    Ui::openvideo *ui;
};

class VideoFrame : public QThread {

Q_OBJECT

    openvideo *UIclass;

    void run() override;

public:
    VideoFrame(QObject *parent = nullptr, openvideo *uiclass = nullptr);

    ~VideoFrame() override;

Q_SIGNALS:

    void complete();
};

#endif // OPENVIDEO_H
