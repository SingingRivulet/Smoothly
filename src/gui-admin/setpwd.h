#ifndef SETPWD_H
#define SETPWD_H

#include <QDialog>

namespace Ui {
class setPwd;
}

class setPwd : public QDialog
{
    Q_OBJECT

public:
    explicit setPwd(QWidget *parent = 0);
    ~setPwd();
    std::string user,pwd;
    bool doActivity;

private slots:
    void on_buttonBox_accepted();

private:
    Ui::setPwd *ui;
};

#endif // SETPWD_H
