#include "helpwindow.h"
#include "ui_helpwindow.h"

HelpWIndow::HelpWIndow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::HelpWIndow)
{
    ui->setupUi(this);
}

HelpWIndow::~HelpWIndow()
{
    delete ui;
}
