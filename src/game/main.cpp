#include "ui/login.h"
#include <QApplication>
int main(int argc, char *argv[]){
    QApplication a(argc, argv);
    login g;
    g.show();
    return a.exec();
}
