#ifndef MISSIONEDITOR_H
#define MISSIONEDITOR_H

#include <QDialog>
#include "../game.h"

namespace Ui {
class missionEditor;
}

class missionEditor : public QDialog
{
        Q_OBJECT

    public:
        explicit missionEditor(QWidget *parent = 0);
        ~missionEditor();
        void closeEvent(QCloseEvent *)override;

        smoothly::game * game;

    private slots:
        void on_pushButton_clicked();

    private:
        Ui::missionEditor *ui;
};

#endif // MISSIONEDITOR_H
