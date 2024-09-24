#include <QPushButton>
#include <QApplication>
#include <iostream>
#include "LoginWindow.h"
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    LoginWindow lg;
    lg.show();
    
    QObject::connect(&lg, &LoginWindow::loginSuccessful, [&](LogicDataBase* db){
        MainWindow* w = new MainWindow(nullptr,db);
        w->show();
    });
    
    return a.exec();
}