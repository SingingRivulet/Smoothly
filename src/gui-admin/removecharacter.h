#ifndef REMOVECHARACTER_H
#define REMOVECHARACTER_H

#include <QDialog>

namespace Ui {
class removeCharacter;
}

class removeCharacter : public QDialog
{
    Q_OBJECT

public:
    explicit removeCharacter(QWidget *parent = 0);
    ~removeCharacter();
    std::string uuid;
    bool doActivity;

private slots:
    void on_buttonBox_accepted();

private:
    Ui::removeCharacter *ui;
};

#endif // REMOVECHARACTER_H
