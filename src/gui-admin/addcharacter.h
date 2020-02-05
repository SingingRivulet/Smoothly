#ifndef ADDCHARACTER_H
#define ADDCHARACTER_H

#include <QDialog>

namespace Ui {
class addCharacter;
}

class addCharacter : public QDialog
{
    Q_OBJECT

public:
    explicit addCharacter(QWidget *parent = 0);
    ~addCharacter();
    std::string user;
    int id;
    float x,y,z;
    bool doActivity;

private slots:
    void on_buttonBox_accepted();

private:
    Ui::addCharacter *ui;
};

#endif // ADDCHARACTER_H
