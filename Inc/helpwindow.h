#ifndef HELPWINDOW_H
#define HELPWINDOW_H

#include <QDialog>

namespace Ui {
    class HelpWIndow;
}

class HelpWIndow : public QDialog {
Q_OBJECT

public:
    explicit HelpWIndow(QWidget *parent = nullptr);

    ~HelpWIndow();

private:
    Ui::HelpWIndow *ui;
};

#endif // HELPWINDOW_H
