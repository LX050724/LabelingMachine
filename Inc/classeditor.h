#ifndef CLASSEDITOR_H
#define CLASSEDITOR_H

#include <QDialog>

#include <Labels.h>

namespace Ui {
class classeditor;
}

class classeditor : public QDialog
{
    Q_OBJECT

public:
    explicit classeditor(QWidget *parent = nullptr);
    classeditor(QWidget *parent = nullptr, Labels* _labels = nullptr);
    ~classeditor();

private slots:
    void on_deletepushButton_clicked();

    void on_tableWidget_cellChanged(int row, int column);

    void on_addpushButton_clicked();

    void on_okpushButton_clicked();

    void on_camcelpushButton_clicked();

private:
    Ui::classeditor *ui;

protected:
    Labels* labels;

};

#endif // CLASSEDITOR_H
