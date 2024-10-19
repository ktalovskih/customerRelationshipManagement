#ifndef __LOGINWINDOW_H__
#define __LOGINWINDOW_H__

#include <ui_loginWindow.h>
#include <QMainWindow>
#include <LogicDataBase.h>
#include "MainWindow.h"
#pragma once

class LoginWindow : public QWidget{
    Q_OBJECT
    LogicDataBase* db;   
    std::unique_ptr<Ui::LoginWindow> ui; 
public:
    explicit LoginWindow(QWidget* parent = nullptr);
    ~LoginWindow();
    void keyPressEvent(QKeyEvent *event) override;

signals:
    void loginSuccessful(LogicDataBase* db); 
private slots:
    void attemptLogin();
    
};


#endif // __LOGINWINDOW_H__