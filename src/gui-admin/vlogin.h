#ifndef VLOGIN_H
#define VLOGIN_H

#include <QDialog>

namespace Ui {
class vlogin;
}

class vlogin : public QDialog
{
    Q_OBJECT

public:
    explicit vlogin(QWidget *parent = 0);
    ~vlogin();
    std::string user,pwd;
    bool doActivity;

private slots:
    void on_buttonBox_accepted();

private:
    Ui::vlogin *ui;
};

#endif // VLOGIN_H
