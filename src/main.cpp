#include <QPushButton>
#include <QApplication>
#include <iostream>
#include "EmployeeLoginForm.h"
#include "DashboardWindow.h"



int main(int argc, char *argv[])
{
    if (sodium_init() < 0) {
        std::cerr << "Libsodium initialization failed!" << std::endl;
        return 1;
    }

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
