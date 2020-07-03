#ifndef LOGIN_H
#define LOGIN_H

#include <QMainWindow>
#include <QThread>
#include "missioneditor.h"
#include "../game.h"

namespace Ui {
class login;
}

class GameThread:public QThread{
        Q_OBJECT

    public:
        std::string host,user,pwd;
        short port;
        GameThread(QObject *parent);
        void run();//线程入口函数（工作线程的主函数）
    signals:
        void exitSignal();
        void missionEditSignal(smoothly::game *);
};


class login : public QMainWindow{
        Q_OBJECT

    public:
        explicit login(QWidget *parent = 0);
        ~login();

    private slots:
        void on_pushButton_login_clicked();
        void gameOver();
        void missionEdit(smoothly::game *);

    private:
        Ui::login *ui;
        missionEditor medit;

};

#endif // LOGIN_H
