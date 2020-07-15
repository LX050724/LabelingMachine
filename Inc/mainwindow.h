#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QGraphicsScene>
#include <QListWidgetItem>
#include <QTableWidgetItem>

#include "bandboxview.h"
#include "labelingmaching.h"
#include "classeditor.h"
#include "TCP_Server.h"
#include "ClientUI.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class ServerUI;

class MainWindow : public QMainWindow {
    friend class ServerUI;
    friend class ClientUI;

Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow() override;

private slots:

    void lastimg_pushButton_clicked();

    void nextimg_pushButton_clicked();

    void comboBox_currentIndexChanged(int);

    void class_tableWidget_cellClicked(int, int);

    void imgs_listWidget_itemClicked(QListWidgetItem *);

    void graphicsView_drawBandBox(const BandBox &);

    void graphicsView_deletBox();

    void graphicsView_Keypress(int);

    void save_triggered();

    void on_openVideo_triggered();

    void on_open_triggered();

    void on_creat_triggered();

    void on_pushButton_3_clicked();

    void on_help_triggered();

    void on_about_triggered();

    void on_ClientModeAction_triggered();

    void on_HostModeAction_triggered();

    void on_Program_Conversion_triggered();

    void on_actionChange_XmlPath_and_ImgPath_triggered();

private:
    enum {
        all, Marked, NoMarked
    };

    Ui::MainWindow *ui;
    ServerUI *pServerUi = nullptr;
    ClientUI *pClientUi = nullptr;

    QString imgpath;
    int img_Index_row = 0;

    LabelingMaching Project;

    void loadimg(const QString &filename);

    void updateclass();

    void updatelabel(const ImageData &_Imagedata);

    void updateimglist(int mod);

    void closeEvent(QCloseEvent *event) override;
};
#endif // MAINWINDOW_H
