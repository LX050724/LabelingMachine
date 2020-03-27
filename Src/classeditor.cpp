#include "classeditor.h"
#include "ui_classeditor.h"

#include <QtDebug>
#include <QMessageBox>

classeditor::classeditor(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::classeditor)
{
    ui->setupUi(this);
}    

classeditor::classeditor(QWidget *parent, Labels* _labels) :
    QDialog(parent),
    ui(new Ui::classeditor)
{
    ui->setupUi(this);
    labels = _labels;

    for (int i = 0; i < labels->labels.size(); ++i)
    {
        auto ID_Item = new QTableWidgetItem(QString::number(i));
        auto class_Item = new QTableWidgetItem(labels->labels[i]);
        ui->tableWidget->insertRow(ui->tableWidget->rowCount());
        ui->tableWidget->setItem(i, 0, ID_Item);
        ui->tableWidget->setItem(i, 1, class_Item);
    }
}

classeditor::~classeditor()
{
    delete ui;
}

void classeditor::on_addpushButton_clicked()
{
    int rowCount = ui->tableWidget->rowCount();
    ui->tableWidget->insertRow(rowCount);
    for(int i = 0; i < ui->tableWidget->rowCount(); ++i)
    {
        QTableWidgetItem* Item = ui->tableWidget->item(i, 0);
        if(Item != nullptr)
            Item->setText(QString::number(i));
        else
        {
            Item = new QTableWidgetItem(QString::number(i));
            ui->tableWidget->setItem(i, 0, Item);
        }
    }
}


void classeditor::on_deletepushButton_clicked() {
    int seletrow = ui->tableWidget->currentRow();
    ui->tableWidget->removeRow(seletrow);
    for(int i = 0; i < ui->tableWidget->rowCount(); ++i) {
        QTableWidgetItem* Item = ui->tableWidget->item(i, 0);
        if(Item != nullptr)
            Item->setText(QString::number(i));
        else {
            Item = new QTableWidgetItem(QString::number(i));
            ui->tableWidget->setItem(i, 0, Item);
        }
    }
}

void classeditor::on_tableWidget_cellChanged(int row, int column) {
    QTableWidgetItem* Item = ui->tableWidget->item(row, column);
    if(column == 0)
        Item->setText(QString::number(row));
}


void classeditor::on_okpushButton_clicked() {
    for (int i = 0; i < ui->tableWidget->rowCount(); ++i) {
        QTableWidgetItem* Item = ui->tableWidget->item(i, 1);
        if(Item == nullptr) {
            QMessageBox::warning(this, tr("error"), QString::asprintf("The %d is empty", i));
            return;
        }
        if(Item->text().isEmpty()) {
            QMessageBox::warning(this, tr("error"), QString::asprintf("The %d is empty", i));
            return;
        }
    }

    labels->labels.clear();
    for (int i = 0; i < ui->tableWidget->rowCount(); ++i) {
        QTableWidgetItem* Item = ui->tableWidget->item(i, 1);
        labels->labels.push_back(Item->text());
    }
    close();
}

void classeditor::on_camcelpushButton_clicked() {
    close();
}
