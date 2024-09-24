#include "MainWindow.h"
#include <QPushButton>
#include <QLineEdit>
#include <QCompleter>
#include <QTimeEdit>
#include <QTime>
#include <QLabel>
#include <QMessageBox>
MainWindow::MainWindow(QWidget* parent, LogicDataBase* database)
    : QMainWindow(parent), db(database), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    showAllShifts();
    QObject::connect(ui->listWidget, &QListWidget::itemClicked, this, &MainWindow::onShiftSelected);
    QObject::connect(ui->pushButton, &QPushButton::pressed, this, &MainWindow::addTask);
    QObject::connect(ui->pushButton_2, &QPushButton::pressed, this, &MainWindow::editShift);
}
MainWindow::~MainWindow(){
    delete db;
};
void MainWindow::onShiftSelected(QListWidgetItem *item) {
    selectedShiftId = item->data(Qt::UserRole).toInt();  
}
void MainWindow::editShift() {
    QListWidgetItem *selectedItem = ui->listWidget->currentItem();
    if (!selectedItem) {
        QMessageBox::warning(this, "No Selection", "Please select a shift to edit.");
        return;
    }

    int shiftId = selectedItem->data(Qt::UserRole).toInt();

    createShiftForm(shiftId);
}

void MainWindow::addTask(){
    createShiftForm(-1);
}
void MainWindow::createShiftForm(int indexOfShift){
    
    QSqlQueryModel *model1 = new QSqlQueryModel(this);
    QSqlQueryModel *model2 = new QSqlQueryModel(this);

    QWidget* wigdet = new QWidget;
    QHBoxLayout* layout = new QHBoxLayout;
    auto* label3 = new QLineEdit();

    auto* startTime = new QTimeEdit();
    auto* endTime = new QTimeEdit();
    startTime->setDisplayFormat("HH:mm");
    endTime->setDisplayFormat("HH:mm");
    startTime->setTime(QTime::currentTime());
    endTime->setTime(QTime::currentTime());

    auto button = new QPushButton("Create Shift");

    model1->setQuery("SELECT name FROM models");  
    model2->setQuery("select name from users where role = 'operator'");

    QComboBox *comboBoxModel = new QComboBox; 
    QComboBox *comboBoxOperator = new QComboBox; 

    comboBoxModel->setModel(model1);
    comboBoxModel->setModelColumn(0);  

    comboBoxOperator->setModel(model2);
    comboBoxOperator->setModelColumn(0);

    QCompleter *completer1 = new QCompleter(model1, this);
    completer1->setCompletionColumn(0);  
    completer1->setCompletionMode(QCompleter::PopupCompletion);  
    completer1->setCaseSensitivity(Qt::CaseInsensitive);  

    comboBoxModel->setCompleter(completer1);

    //
    QCompleter *completer2 = new QCompleter(model2, this);
    completer2->setCompletionColumn(0);  
    completer2->setCompletionMode(QCompleter::PopupCompletion);  
    completer2->setCaseSensitivity(Qt::CaseInsensitive);  

    comboBoxOperator->setCompleter(completer1);
    //

    comboBoxModel->setEditable(true);
    comboBoxOperator->setEditable(true);

    comboBoxOperator->setPlaceholderText("operator");
    comboBoxModel->setPlaceholderText("model");

LoginWindow::LoginWindow(QWidget* parent) : ui(std::make_unique<Ui::Form>()), db(new LogicDataBase){
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
}    endTime->setKeyboardTracking(true);

    wigdet->setLayout(layout);
    layout->addWidget(comboBoxOperator);
    layout->addWidget(comboBoxModel);
    layout->addWidget(startTime);
    layout->addWidget(endTime);
    layout->addWidget(button);
    wigdet->show();
    


    QObject::connect(button, &QPushButton::pressed, [=]() {
        if (indexOfShift == -1){
            comboBoxOperator->currentText();
            db->addShift(comboBoxOperator->currentText(), comboBoxModel->currentText(),startTime->time(), endTime->time());
        }
        else{
            QSqlQuery query;
            
            query.prepare("UPDATE shifts SET operator_id = :operatorId, model_id = :modelId, "
              "start_time = :startTime, end_time = :endTime WHERE id = :id");
            query.bindValue(":operator", comboBoxOperator->currentText());
            query.bindValue(":model", comboBoxModel->currentText());
            query.bindValue(":startTime", startTime->time());
            query.bindValue(":endTime", endTime->time());
            query.bindValue(":id", indexOfShift+1);
            qDebug() << indexOfShift;
            if (query.exec()) {
                QMessageBox::information(this, "Shift Updated", "Shift has been successfully updated.");
            } else {
                QMessageBox::warning(this, "Error", "Failed to update shift: " + query.lastError().text());
            }
        }
        ui->listWidget->clear();
        showAllShifts();
        wigdet->deleteLater();

    });
}

void MainWindow::showAllShifts(){
    QSqlQuery query;
    query.prepare("SELECT id, operator_id, model_id, start_time, end_time, total_hours, status FROM shifts");

    if (!query.exec()) {
        qDebug() << "Query execution error: " << query.lastError().text();
        return;
    }

    ui->listWidget->clear();

    while (query.next()){
        QString operatorId = [=](){
            query.value("operator_id").toString(); 
            QSqlQuery newQuery;
            newQuery.prepare("select name from users where id = :_id");
            newQuery.bindValue(":_id", query.value("operator_id").toString());
            newQuery.exec();
            newQuery.next();
            return newQuery.value(0).toString();
        }();
        QString modelId = [=](){
            query.value("operator_id").toString(); 
            QSqlQuery newQuery;
            newQuery.prepare("select name from models where id = :_id");
            newQuery.bindValue(":_id", query.value("model_id").toString());
            newQuery.exec();
            newQuery.next();
            return newQuery.value(0).toString();
        }();
        
        QString startTime = query.value("start_time").toString(); 
        QString endTime = query.value("end_time").toString(); // End Time
        QString totalHours = query.value("total_hours").toString(); // Total Hours
        QString status = query.value("status").toString(); // Status

        // Create a formatted string with labels
        QString buffer = QString("Operator: %2 | Model: %3 | Start: %4 | End: %5 | Hours: %6 | Status: %7")
                            
                            .arg(operatorId)
                            .arg(modelId)
                            .arg(startTime)
                            .arg(endTime)
                            .arg(totalHours.isEmpty() ? "N/A" : totalHours) // Handling NULL values
                            .arg(status);

        qDebug() << "Adding item: " << buffer;
        ui->listWidget->addItem(buffer);
    }
}

#include <moc_MainWindow.cpp>