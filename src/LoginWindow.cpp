#include "LoginWindow.h"
#include <QPushButton>
#include <QMessageBox>
LoginWindow::LoginWindow(QWidget* parent) : ui(std::make_unique<Ui::LoginWindow>()), db(new LogicDataBase){
    ui->setupUi(this);

    QObject::connect(ui->pushButton,&QPushButton::pressed , this, &LoginWindow::attemptLogin);
}

LoginWindow::~LoginWindow(){

}

void LoginWindow::attemptLogin() {
    if (db->login(ui->lineEdit->text(), ui->lineEdit_2->text())) {
        emit loginSuccessful(db);  
        close(); 
    } else {
        QMessageBox::warning(this, "Login Failed", "Incorrect username or password.");
    }
}

#include <moc_LoginWindow.cpp>