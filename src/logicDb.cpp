#include "logicDb.h"
#include "MainWindow.h"
#include <QTime>
#include <QPixmap>
#include <QLabel>
#include <iostream>
#include <filesystem>
#include <QMessageBox>
LogicDataBase::LogicDataBase()
{
    db.setDatabaseName("test.db");
    if (!db.open())
        qDebug() << "problem!";
    QSqlQuery query;
    if (!query.exec("CREATE TABLE IF NOT EXISTS tasks (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT NOT NULL, description TEXT, time DATE, image BLOB)"))
    {
        qDebug() << "Error creating table:" << query.lastError();
    }
    if (!query.exec("CREATE TABLE IF NOT EXISTS users(id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT NOT NULL,password text not null ,role TEXT not null)"))
    {
        qDebug() << "Error creating table:" << query.lastError();
    }
}

LogicDataBase::~LogicDataBase() {}

void LogicDataBase::addShift(const QString _operator, const QString _model, const QTime _timeStart, const QTime _timeEnd)
{
    if (_timeStart >= _timeEnd) {
        QMessageBox::critical(nullptr, "Error", "Start time must be earlier than end time.");
    }
    QSqlQuery query;

    int operator_id = [=]()
    {
        QSqlQuery index;
        index.prepare("SELECT id FROM users WHERE name = :name");
        index.bindValue(":name", _operator);
        index.exec();
        if (index.next()) {  
            return index.value(0).toInt();  
        }
        return -1;
    }();

    
    int model_id = [=]()
    {
        QSqlQuery index;
        index.prepare("SELECT id FROM models WHERE name = :name");
        index.bindValue(":name", _model);
        index.exec();
        if (index.next()) {  
            return index.value(0).toInt();  
        }
        return -1;
    }();

    if (operator_id == -1 || model_id == -1) {
        QMessageBox::critical(nullptr, "Error", "Model or operator was not found!");
        return ;  
    }

   
    int total_seconds = _timeStart.secsTo(_timeEnd);
    float total_hours = total_seconds / 3600.0; 

    query.prepare("INSERT INTO shifts (operator_id, model_id, start_time, end_time, total_hours, status, created_by) "
                  "VALUES (:operator_id, :model_id, :start_time, :end_time, :total_hours, 'in progress', 1)");
    query.bindValue(":operator_id", operator_id);
    query.bindValue(":model_id", model_id);
    query.bindValue(":start_time", _timeStart.toString("HH:mm"));
    query.bindValue(":end_time", _timeEnd.toString("HH:mm"));
    query.bindValue(":total_hours", total_hours);

    if (!query.exec()) {
        QMessageBox::critical(nullptr, "Database Error", query.lastError().text());
        
    }

    
}


void LogicDataBase::deleteTask()
{
}
void LogicDataBase::addDiscription()
{
}
bool LogicDataBase::login(const QString name, const QString _password)
{
    QSqlQuery query;
    query.prepare("SELECT name,password from users where name = :name");
    query.bindValue(":name", name);
    query.exec();
    while (query.next())
    {
        QString password = query.value("password").toString();
        qDebug() << password;
        if (_password == password)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    qDebug() << db.lastError();
    return 0;
}