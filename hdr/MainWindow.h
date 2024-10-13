#ifndef __MAINWINDOW_H__
#define __MAINWINDOW_H__

#include <QKeyEvent>
#include <QMainWindow>
#include "logicDb.h"  
#include "ui_mainWindowAdmin.h" 
#include <QLabel>
#include "ui_mainWindowOperator.h"
#pragma once
class MainWindow : public QMainWindow {
    Q_OBJECT 

    int selectedShiftId;
    LogicDataBase* db;  
    std::unique_ptr<Ui::mainWindowOperator> uiOperator;  
    std::unique_ptr<Ui::mainWindowAdmin> uiAdmin; 
    bool isAdmin;
    int idUser;
    QLabel* labelForImage; 
    QMap<QString, QImage> mapOfTheImages;


public:

    explicit MainWindow(QWidget* parent, LogicDataBase* database, bool isAdmin, int idUser);
    ~MainWindow();
    void createShiftForm();
    void keyPressEvent(QKeyEvent *event) override;
    
public slots:
    void addTask();
    void showAllShifts();
    void onShiftSelected(QListWidgetItem *item);
    void editShift();
    void createShiftForm(int indexOfShift);
    void endShift();
    void showWholeInformation();
    void startShift();

};


#endif // __MAINWINDOW_H__