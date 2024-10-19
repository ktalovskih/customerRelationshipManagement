#ifndef __MAINWINDOW_H__
#define __MAINWINDOW_H__
#include <map>
#include <QKeyEvent>
#include <QMainWindow>
#include "LogicDataBase.h"  
#include "ui_mainWindowAdmin.h" 
#include <QLabel>
#include "ui_mainWindowOperator.h"
#include "StartEndWidget.h"

#pragma once
class MainWindow : public QMainWindow {
    
    Q_OBJECT 

    int selectedShiftId;
    LogicDataBase* db;  
    std::unique_ptr<Ui::mainWindowOperator> uiOperator;  
    std::unique_ptr<Ui::mainWindowAdmin> uiAdmin; 
    std::map<int, StartEndWidget*> startEndWidgets; 
    bool isAdmin;
    int idUser;
    QTimer* updateTimer;
    

public:

    explicit MainWindow(QWidget* parent, LogicDataBase* database, bool isAdmin, int idUser);
    ~MainWindow();
    void createShiftForm();
    
public slots:
    
    void addTask();
    void showAllShifts();
    void onShiftSelected(QListWidgetItem *item);
    void editShift();
    void createShiftForm(int indexOfShift);
    void showReport();
    void showWholeInformation();
    void addNewUser();
};


#endif // __MAINWINDOW_H__