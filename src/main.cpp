#include <QPushButton>
#include <QApplication>
#include <iostream>
#include "EmployeeLoginForm.h"
#include "DashboardWindow.h"

void applyAppStyle(QApplication& app) {
    QString style = R"(
        QWidget {
            background-color: #1c1c1e;
            color: #e5e5e7;
            font-family: "Segoe UI", sans-serif;
            font-size: 14px;
        }
    
        QPushButton {
            background-color: #2c2c2e;
            color: #ffffff;
            border: 1px solid #3a3a3c;
            border-radius: 6px;
            padding: 6px 12px;
        }
    
        QPushButton:hover {
            background-color: #3a3a3c;
        }
    
        QLineEdit {
            background-color: #2c2c2e;
            border: 1px solid #3a3a3c;
            padding: 4px 8px;
            border-radius: 4px;
            color: #ffffff;
        }
    
        QLabel {
            color: #d0d0d3;
        }
    
        QTableView {
            background-color: #2c2c2e;
            gridline-color: #3a3a3c;
            color: #e5e5e7;
            selection-background-color: #0a84ff;
            selection-color: #ffffff;
        }
    )";
    app.setStyleSheet(style);
}


int main(int argc, char *argv[])
{
    if (sodium_init() < 0) {
        std::cerr << "Libsodium initialization failed!" << std::endl;
        return 1;
    }
    QApplication a(argc, argv);

    applyAppStyle(a);
    EmployeeLoginForm loginForm;
    loginForm.show();
    a.setWindowIcon(QIcon("icon.png"));
    QObject::connect(&loginForm, &EmployeeLoginForm::loginSuccessful, [&](CRMDatabaseManager* databaseManager) {
        DashboardWindow* dashboard = new DashboardWindow(nullptr, databaseManager, databaseManager->getRole() == "admin", databaseManager->getEmployeeId());
        dashboard->show();
    });
    return a.exec();
}
