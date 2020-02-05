#ifndef SETDATA_H
#define SETDATA_H

#include <QDialog>

namespace Ui {
class setData;
}

class setData : public QDialog
{
    Q_OBJECT

public:
    explicit setData(QWidget *parent = 0);
    ~setData();
    std::string key,val;
    bool doActivity;

private slots:
    void on_buttonBox_accepted();

private:
    Ui::setData *ui;
};

#endif // SETDATA_H
