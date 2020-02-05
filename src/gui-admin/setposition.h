#ifndef SETPOSITION_H
#define SETPOSITION_H

#include <QDialog>

namespace Ui {
class setPosition;
}

class setPosition : public QDialog
{
    Q_OBJECT

public:
    explicit setPosition(QWidget *parent = 0);
    ~setPosition();
    std::string uuid;
    float x,y,z;
    bool doActivity;

private slots:
    void on_buttonBox_accepted();

private:
    Ui::setPosition *ui;
};

#endif // SETPOSITION_H
