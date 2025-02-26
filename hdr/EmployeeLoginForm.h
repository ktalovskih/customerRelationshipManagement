#ifndef __EMPLOYEELOGINFORM_H__
#define __EMPLOYEELOGINFORM_H__

#include <ui_loginWindow.h>
#include <QMainWindow>
#include "CRMDatabaseManager.h"
#include "DashboardWindow.h"
#include <sodium.h>
#pragma once

class EmployeeLoginForm : public QWidget {
    Q_OBJECT
    CRMDatabaseManager* databaseManager;
    std::unique_ptr<Ui::LoginWindow> ui;
public:
    explicit EmployeeLoginForm(QWidget* parent = nullptr);
    ~EmployeeLoginForm();
    void keyPressEvent(QKeyEvent *event) override;
signals:
    void loginSuccessful(CRMDatabaseManager* databaseManager);
private slots:
    void attemptLogin();
};

#endif // __EMPLOYEELOGINFORM_H__
