#ifndef __DASHBOARDWINDOW_H__
#define __DASHBOARDWINDOW_H__

#include <map>
#include <QKeyEvent>
#include <QMainWindow>
#include "CRMDatabaseManager.h"  
#include "ui_mainWindowAdmin.h" 
#include <QLabel>
#include "ui_mainWindowOperator.h"
#include "SessionDetailWidget.h"
#include <QTimer>

#pragma once
class DashboardWindow : public QMainWindow {
    
    Q_OBJECT 

    int selectedSessionId;
    CRMDatabaseManager* databaseManager;  
    std::unique_ptr<Ui::mainWindowOperator> uiOperator;  
    std::unique_ptr<Ui::mainWindowAdmin> uiAdmin; 
    std::map<int, SessionDetailWidget*> sessionDetailWidgets; 
    bool isAdmin;
    int employeeId;
    QTimer* updateTimer;
    
public:
    explicit DashboardWindow(QWidget* parent, CRMDatabaseManager* database, bool isAdmin, int employeeId);
    ~DashboardWindow();
    void createSessionForm();
    
public slots:
    void addTask();
    void showAllSessions();
    void onSessionSelected(QListWidgetItem *item);
    void editSession();
    void createSessionForm(int indexOfSession);
    void showReport();
    void showWholeInformation();
    void addNewUser();
};

#endif // __DASHBOARDWINDOW_H__
