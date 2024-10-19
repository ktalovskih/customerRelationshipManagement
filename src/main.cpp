#include <QPushButton>
#include <QApplication>
#include <iostream>
#include "LoginWindow.h"
#include "MainWindow.h"



int main(int argc, char *argv[])
{
    QRandomGenerator rand;
    QApplication a(argc, argv);
    LoginWindow lg;
    lg.show();
    a.setWindowIcon(QIcon("icon.png"));
    QObject::connect(&lg, &LoginWindow::loginSuccessful, [&](LogicDataBase* db){
        MainWindow* w = new MainWindow(nullptr,db,db->getRole() == "admin" ? true : false, db->getId());
        w->show();
    });
    
    return a.exec();
}