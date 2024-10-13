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

void LogicDataBase::addShift(const QString _operator, const QString _model, const QTime _timeStart, const QTime _timeEnd, const QString logs)
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

    query.prepare("INSERT INTO shifts (operator_id, model_id, start_time, end_time, total_hours, status, created_by,logs) "
                  "VALUES (:operator_id, :model_id, :start_time, :end_time, :total_hours, 'in progress', 1, :logs)");
    query.bindValue(":operator_id", operator_id);
    query.bindValue(":model_id", model_id);
    query.bindValue(":start_time", _timeStart.toString("HH:mm"));
    query.bindValue(":end_time", _timeEnd.toString("HH:mm"));
    query.bindValue (":logs", logs);
    query.bindValue(":total_hours", total_hours);

    if (!query.exec()) {
        QMessageBox::critical(nullptr, "Database Error", query.lastError().text());
        
    }

    
}


void LogicDataBase::deleteShift(const int indexOfShift)
{
    QSqlQuery query;
    query.prepare("DELETE FROM shifts WHERE id = :id");
    query.bindValue(":id", indexOfShift);
    if (query.exec())
    {
        QSqlQuery reorderQuery;
        reorderQuery.prepare("WITH REORDERED AS ( "
                             "   SELECT id, ROW_NUMBER() OVER(ORDER BY id) AS new_id "
                             "   FROM shifts "
                             ") "
                             "UPDATE shifts "
                             "SET id = (SELECT new_id FROM REORDERED WHERE shifts.id = REORDERED.id);");

        if (reorderQuery.exec())
        {
            QSqlQuery resetAutoIncrement;
            resetAutoIncrement.prepare("DELETE FROM sqlite_sequence WHERE name='shifts'");
            resetAutoIncrement.exec();
        }
        else
        {
            qDebug() << "Failed to reorder shift IDs:" << reorderQuery.lastError().text();
        }
    }
    else
    {
        qDebug() << "Failed to delete shift:" << query.lastError().text();
    }
    
}
void LogicDataBase::addDiscription()
{
}
bool LogicDataBase::login(const QString name, const QString _password)
{
    QSqlQuery query;
    query.prepare("SELECT id,name,password from users where name = :name");
    query.bindValue(":name", name);
    query.exec();
    while (query.next())
    {
        QString password = query.value("password").toString();
        qDebug() << password;
        if (_password == password)
        {
            role = name;
            idUser = query.value("id").toInt();   
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

int LogicDataBase::getId()
{
    return idUser;
}
QString LogicDataBase::getRole(){
    return role;
}
void LogicDataBase::uploadImageToDB(const QPixmap &pixmap) {

        QByteArray byteArray;
        QBuffer buffer(&byteArray);
        buffer.open(QIODevice::WriteOnly);
        pixmap.save(&buffer, "PNG");  // Save the image as PNG to the byte array

        QSqlQuery query;
        query.prepare("INSERT INTO shift_screenshots (shift_id, screenshot, uploaded_at) "
                      "VALUES (:shift_id, :screenshot, :uploaded_at)");
        query.bindValue(":shift_id", 1);  // Replace with actual shift ID
        query.bindValue(":screenshot", byteArray);
        query.bindValue(":uploaded_at", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));

        if (!query.exec()) {
            qDebug() << "Failed to upload image: " << query.lastError().text();
        } else {
            qDebug() << "Image uploaded successfully!";
        }
}

void LogicDataBase::updateShift(const QString operatorName, const QString modelName,const QTime _timeStart,const QTime _timeEnd, const QString logs, int indexOfShift)
{
    QSqlQuery query;
        query.prepare("UPDATE shifts SET operator_id = :operatorId, model_id = :modelId, "
                      "start_time = :startTime, end_time = :endTime, logs = :logs WHERE id = :id");
        
        int operator_id = [=]()
        {
            QSqlQuery index;
            index.prepare("SELECT id FROM users WHERE name = :name");
            index.bindValue(":name", operatorName);
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
            index.bindValue(":name", modelName);
            index.exec();
            if (index.next()) {  
                return index.value(0).toInt();  
            }
            return -1;
        }();
        query.bindValue(":operatorId", operator_id);
        query.bindValue(":modelId", model_id);
        query.bindValue(":startTime", _timeStart.toString("HH:mm"));
        query.bindValue(":endTime", _timeEnd.toString("HH:mm"));
        query.bindValue(":id", indexOfShift);
        query.bindValue(":logs", logs);

        if (query.exec()) {
            QMessageBox::information(nullptr, "Shift Updated", "Shift has been successfully updated.");
        } else {
            QMessageBox::warning(nullptr, "Error", "Failed to update shift: " + query.lastError().text());
            qDebug() << query.lastError().text();
        }
}
