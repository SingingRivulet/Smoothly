#ifndef SETMAINCONTROL_H
#define SETMAINCONTROL_H

#include <QDialog>

namespace Ui {
class setMainControl;
}

class setMainControl : public QDialog
{
    Q_OBJECT

public:
    explicit setMainControl(QWidget *parent = 0);
    ~setMainControl();
    std::string user,uuid;
    bool doActivity;

private slots:
    void on_buttonBox_accepted();

private:
    Ui::setMainControl *ui;
};

#endif // SETMAINCONTROL_H
