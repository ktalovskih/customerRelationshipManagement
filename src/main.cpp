#include <QPushButton>
#include <QApplication>
#include <iostream>
#include "EmployeeLoginForm.h"
#include "DashboardWindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    EmployeeLoginForm loginForm;
    loginForm.show();
    a.setWindowIcon(QIcon("icon.png"));
    QObject::connect(&loginForm, &EmployeeLoginForm::loginSuccessful, [&](CRMDatabaseManager* databaseManager) {
        DashboardWindow* dashboard = new DashboardWindow(nullptr, databaseManager, databaseManager->getRole() == "admin", databaseManager->getEmployeeId());
        dashboard->show();
    });
    return a.exec();
}
