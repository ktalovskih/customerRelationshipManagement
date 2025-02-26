#include "EmployeeLoginForm.h"
#include <QPushButton>
#include <QMessageBox>

EmployeeLoginForm::EmployeeLoginForm(QWidget* parent) : ui(std::make_unique<Ui::LoginWindow>()), databaseManager(new CRMDatabaseManager) {
    ui->setupUi(this);
    QObject::connect(ui->pushButton, &QPushButton::pressed, this, &EmployeeLoginForm::attemptLogin);
}

EmployeeLoginForm::~EmployeeLoginForm() { }

void EmployeeLoginForm::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        attemptLogin();
    }
}

void EmployeeLoginForm::attemptLogin() {
    QString username = ui->lineEdit->text();
    QString password = ui->lineEdit_2->text();

    if (databaseManager->login(username, password)) {
        emit loginSuccessful(databaseManager);
        close();
    } else {
        QMessageBox::warning(this, "Login Failed", "Incorrect username or password.");
    }
}

// std::string hashPassword(const std::string &password) {
//     char hashedPassword[crypto_pwhash_STRBYTES];

//     if (crypto_pwhash_str(hashedPassword, password.c_str(), password.length(),
//                           crypto_pwhash_OPSLIMIT_INTERACTIVE, crypto_pwhash_MEMLIMIT_INTERACTIVE) != 0) {
//         throw std::runtime_error("Password hashing failed");
//     }

//     return std::string(hashedPassword);
// }

#include <moc_EmployeeLoginForm.cpp>
