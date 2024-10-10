#ifndef __LOGINWINDOW_H__
#define __LOGINWINDOW_H__

#include <ui_loginWindow.h>
#include <QMainWindow>
#include <logicDb.h>
#include "MainWindow.h"

class LoginWindow : public QWidget{
    Q_OBJECT
    LogicDataBase* db;   
    std::unique_ptr<Ui::LoginWindow> ui; 
public:
    explicit LoginWindow(QWidget* parent = nullptr);
    ~LoginWindow();
signals:
    void loginSuccessful(LogicDataBase* db); 
private slots:
    void attemptLogin();
    
};


#endif // __LOGINWINDOW_H__