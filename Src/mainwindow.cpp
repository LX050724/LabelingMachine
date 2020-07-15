#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QFileDialog>
#include <Inc/module.h>
#include <Inc/publicdefine.h>

#include "openvideo.h"
#include "helpwindow.h"
#include "aboutwindow.h"
#include "ClientUI.h"
#include "ServerUI.h"

MainWindow::MainWindow(QWidget *parent) :
        QMainWindow(parent),
        ui(new Ui::MainWindow) {
    ui->setupUi(this);

    QObject::connect(ui->nextimg_pushButton, &QPushButton::clicked,
                     this, &MainWindow::nextimg_pushButton_clicked);
    QObject::connect(ui->lastimg_pushButton, &QPushButton::clicked,
                     this, &MainWindow::lastimg_pushButton_clicked);
    QObject::connect(ui->graphicsView, &BandBoxView::Keypress,
                     this, &MainWindow::graphicsView_Keypress);
    QObject::connect(ui->graphicsView, &BandBoxView::deletBox,
                     this, &MainWindow::graphicsView_deletBox);
    QObject::connect(ui->graphicsView, &BandBoxView::drawBandBox,
                     this, &MainWindow::graphicsView_drawBandBox);
    QObject::connect(ui->imgs_listWidget, &QListWidget::itemClicked,
                     this, &MainWindow::imgs_listWidget_itemClicked);
    QObject::connect(ui->class_tableWidget, &QTableWidget::cellClicked,
                     this, &MainWindow::class_tableWidget_cellClicked);
    QObject::connect(ui->comboBox, SIGNAL(currentIndexChanged(int)),
                     this, SLOT(comboBox_currentIndexChanged(int)));
    QObject::connect(ui->save, &QAction::triggered,
                     this, &MainWindow::save_triggered);

    ui->Program_Conversion->setDisabled(true);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event) {
    Project.save();
    if (pClientUi)pClientUi->close();
    if (pServerUi)pServerUi->close();
    QWidget::closeEvent(event);
}

void MainWindow::lastimg_pushButton_clicked() {
    if (ui->imgs_listWidget->count() > 0) {
        int row = ui->imgs_listWidget->currentRow() - 1;
        QListWidgetItem *Item = ui->imgs_listWidget->item(row);
        if (Item != nullptr) {
            ui->imgs_listWidget->setCurrentRow(row);
            loadimg(Item->text());
        }
    }
}

void MainWindow::nextimg_pushButton_clicked() {
    if (ui->imgs_listWidget->count() > 0) {
        int row = ui->imgs_listWidget->currentRow() + 1;
        QListWidgetItem *Item = ui->imgs_listWidget->item(row);
        if (Item != nullptr) {
            ui->imgs_listWidget->setCurrentRow(row);
            loadimg(Item->text());
        }
    }
}

void MainWindow::comboBox_currentIndexChanged(int index) {
    if (Project.isOpened())
        updateimglist(index);
    else
        ui->comboBox->setCurrentIndex(0);
}

void MainWindow::on_pushButton_3_clicked() {
    if (Project.isOpened()) {
        classeditor classeditorwindow(this, &Project.labels);
        classeditorwindow.exec();
        Project.save();
        updateclass();
    }
}

void MainWindow::imgs_listWidget_itemClicked(QListWidgetItem *item) {
    if (!item->text().isEmpty())
        loadimg(item->text());
}

void MainWindow::graphicsView_drawBandBox(const BandBox &Box) {
    int count = ui->label_tableWidget->rowCount();
    ui->label_tableWidget->setRowCount(count + 1);
    auto Label_Item = new QTableWidgetItem(Box.Label);
    auto Locate_Item = new QTableWidgetItem(QString::asprintf("(%d,%d)", Box.Rect.x(), Box.Rect.y()));
    ui->label_tableWidget->setItem(count, 0, Label_Item);
    ui->label_tableWidget->setItem(count, 1, Locate_Item);
}

void MainWindow::graphicsView_deletBox() {
    updatelabel(ui->graphicsView->getImageData());
}

void MainWindow::graphicsView_Keypress(int Key) {
    switch (Key) {
        case Qt::Key_Q:
            lastimg_pushButton_clicked();
            break;
        case Qt::Key_E:
            nextimg_pushButton_clicked();
            break;
    }

    if (Key == Qt::Key_QuoteLeft || (Qt::Key_0 <= Key && Key <= Qt::Key_9)) {
        if (Key == Qt::Key_QuoteLeft)
            Key = 0;
        else
            Key -= Qt::Key_0;

        int count = ui->class_tableWidget->rowCount();
        if (Key < count) {
            ui->class_tableWidget->setCurrentCell(Key, 0);
            QTableWidgetItem *Item = ui->class_tableWidget->item(Key, 1);
            if (Item != nullptr)
                ui->graphicsView->setLabel(Key, Item->text(), Project.labels.LabelCount());
            else
                ui->graphicsView->setLabel(0, QString(), 0);
        }
    }
}

void MainWindow::save_triggered() {
    Project.save();
}

void MainWindow::on_openVideo_triggered() {
    openvideo openvideosubwindow;
    openvideosubwindow.exec();
}

void MainWindow::on_open_triggered() {
    QString dirpath = QFileDialog::getOpenFileName(this, tr("Select the Project"), HOME_PATH, "Project (Project.xml)");
    if (!dirpath.isEmpty()) {
        Project.setProjectPath(dirpath);
        if (!Project.load()) {
            QMessageBox::warning(this, tr("error"),
                                 tr("File opening failed, confirm whether it is a LabelingMachine project file"));
            return;
        }

        imgpath = Project.getImgPath();

        ui->graphicsView->setLabel(0, Project.labels[0], Project.labels.LabelCount());

        updateimglist(all);
        updateclass();
        ui->open->setDisabled(true);
        ui->creat->setDisabled(true);
        ui->ClientModeAction->setDisabled(true);
        ui->Program_Conversion->setEnabled(true);
        ui->actionChange_XmlPath_and_ImgPath->setEnabled(true);
    }
}

void MainWindow::on_creat_triggered() {
    QString Imgpath = QFileDialog::getExistingDirectory(this, tr("Select the gallery folder"), HOME_PATH,
                                                        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (Imgpath.isEmpty()) {
        QMessageBox::warning(this, tr("Have no choice"), tr("Path is empty"));
        return;
    }

    QString xmlpath = QFileDialog::getExistingDirectory(this, tr("Select an XML folder"), Imgpath,
                                                        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (xmlpath.isEmpty()) {
        QMessageBox::warning(this, tr("Have no choice"), tr("Path is empty"));
        return;
    }

    QDir ProjectDir(Imgpath);
    if (!ProjectDir.exists("Project")) {
        qInfo() << "Create dir \"Project\"";
        if (!ProjectDir.mkdir("Project")) {
            qWarning() << "Failed";
            QMessageBox::warning(this, tr("error"), tr("The Project folder creation failed"));
            return;
        }
    } else {
        ProjectDir.cd("Project");
        if (ProjectDir.exists("Project.xml")) {
            auto button = QMessageBox::question(this, tr("found Project.xml"),
                                                tr("Discover project.xml, whether to load the tag and regenerate the list of images"));
            if (button == QMessageBox::Yes) {
                Project.setProjectPath(Imgpath + "/Project/Project.xml");
                if (!Project.load()) {
                    QMessageBox::warning(this, tr("error"),
                                         tr("File opening failed, confirm whether it is a LabelingMachine project file"));
                    return;
                }
            }
        }
    }

    Project.Images.clear();
    Project.setProjectPath(Imgpath + "/Project/Project.xml");

    imgpath = Imgpath;
    QDir dir(Imgpath);
    dir.setFilter(QDir::Files);
    dir.setSorting(QDir::DirsFirst);
    QFileInfoList list = dir.entryInfoList();

    if (list.empty()) {
        QMessageBox::warning(this, tr("error"), tr("No files"));
        return;
    }

    Project.setImgCount(list.size());
    Project.setImgPath(imgpath);
    Project.setXmlPath(xmlpath);

    int i = 0;
    do {
        const QFileInfo &fileInfo = list.at(i);
        if (fileInfo.fileName() == "." || fileInfo.fileName() == "..") {
            i++;
            continue;
        }
        ImageData image(fileInfo.path(), fileInfo.fileName(), xmlpath, fileInfo.fileName() + ".xml");
        image.saveXml();

        Project.addImage(image);
    } while (++i < list.size());

    updateimglist(all);
    Project.save();
    updateclass();
    ui->graphicsView->setLabel(0, Project.labels[0], Project.labels.LabelCount());
    ui->open->setDisabled(true);
    ui->creat->setDisabled(true);
    ui->ClientModeAction->setDisabled(true);
    ui->Program_Conversion->setEnabled(true);
}

void MainWindow::class_tableWidget_cellClicked(int row, int column) {
    QTableWidgetItem *item = ui->class_tableWidget->item(row, 1);
    ui->graphicsView->setLabel(row, item->text(), Project.labels.LabelCount());
    (void) column;
}

void MainWindow::loadimg(const QString &filename) {
    if (filename != "") {
        int index = Project.findImage(filename);
        if (index >= 0) {
            ui->graphicsView->loadimg(&Project.Images[index]);
            updatelabel(Project.Images[index]);
        }

        QTableWidgetItem *classitem = ui->class_tableWidget->item(0, 1);
        if (classitem != nullptr) {
            ui->graphicsView->setLabel(0, classitem->text(), Project.labels.LabelCount());
            ui->class_tableWidget->setCurrentCell(0, 1);
        } else ui->graphicsView->setLabel(0, QString(), 0);
    }
}

void MainWindow::updateclass() {
    ui->class_tableWidget->clearContents();
    ui->class_tableWidget->setRowCount(0);
    for (int i = 0; i < Project.labels.LabelCount(); ++i) {
        auto ID_Item = new QTableWidgetItem(QString::number(i));
        auto class_Item = new QTableWidgetItem(Project.labels.getLabels()[i]);
        ui->class_tableWidget->insertRow(ui->class_tableWidget->rowCount());
        ui->class_tableWidget->setItem(i, 0, ID_Item);
        ui->class_tableWidget->setItem(i, 1, class_Item);
    }
    ui->class_tableWidget->setCurrentCell(0, 0);
}

void MainWindow::updatelabel(const ImageData &_Imagedata) {
    ui->label_tableWidget->clearContents();
    ui->label_tableWidget->setRowCount(0);
    int row = 0;
    for (const BandBox &i : _Imagedata.getBandBoxs()) {
        auto Label_Item = new QTableWidgetItem(i.Label);
        auto Locate_Item = new QTableWidgetItem(QString::asprintf("(%d,%d)", i.Rect.x(), i.Rect.y()));
        ui->label_tableWidget->insertRow(ui->label_tableWidget->rowCount());
        ui->label_tableWidget->setItem(row, 0, Label_Item);
        ui->label_tableWidget->setItem(row, 1, Locate_Item);
        ++row;
    }
}

void MainWindow::updateimglist(int mod) {
    QStringList imgfile;
    switch (mod) {
        case all:
            for (const auto &i : Project.all_Img())
                imgfile.push_back(i.getImageFilename());
            break;
        case Marked:
            for (const auto &i : Project.has_label_Img())
                imgfile.push_back(i.getImageFilename());
            break;
        case NoMarked:
            for (const auto &i : Project.no_label_Img())
                imgfile.push_back(i.getImageFilename());
            break;
    }
    ui->imgs_listWidget->clear();
    ui->imgs_listWidget->addItems(imgfile);
}

void MainWindow::on_help_triggered() {
    HelpWIndow help;
    help.exec();
}

void MainWindow::on_about_triggered() {
    Aboutwindow about;
    about.exec();
}

void MainWindow::on_ClientModeAction_triggered() {
    if (pClientUi == nullptr)
        pClientUi = new ClientUI(nullptr, this);
    pClientUi->show();
    ui->HostModeAction->setDisabled(true);
    ui->open->setDisabled(true);
    ui->creat->setDisabled(true);
    ui->pushButton_3->setDisabled(true);
}

void MainWindow::on_HostModeAction_triggered() {
    if (Project.isOpened()) {

        if (pServerUi == nullptr)
            pServerUi = new ServerUI(nullptr, this);
        pServerUi->show();

        while (!pServerUi->isReady()) {
            if (pServerUi->isFailed()) {
                QMessageBox::warning(this, tr("error"), tr("TCP startup failure"));
                delete pServerUi;
                pServerUi = nullptr;
                return;
            }
        }
        ui->ClientModeAction->setDisabled(true);
        ui->pushButton_3->setDisabled(true);
    }
}

void MainWindow::on_Program_Conversion_triggered() {
    module m(this, &Project);
    if (m.isReady()) {
        m.exec();
    }
}

void MainWindow::on_actionChange_XmlPath_and_ImgPath_triggered() {

    QString Imgpath = QFileDialog::getExistingDirectory(this, tr("Select the Img folder"), HOME_PATH,
                                                        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (Imgpath.isEmpty()) {
        QMessageBox::warning(this, tr("Have no choice"), tr("Path is empty"));
        return;
    }

    QString XmlPath = QFileDialog::getExistingDirectory(this, tr("Select the Xml folder"), Imgpath,
                                                        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (XmlPath.isEmpty()) {
        QMessageBox::warning(this, tr("Have no choice"), tr("Path is empty"));
        return;
    }

    Project.setImgPath(Imgpath);
    Project.setXmlPath(XmlPath);
    for (ImageData &i : Project.Images) {
        i.setXmlPath(XmlPath);
        i.setImagePath(Imgpath);
    }
    Project.save();
}
