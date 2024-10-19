#include "LogicDataBase.h"
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

LogicDataBase::~LogicDataBase() {

}

void LogicDataBase::addShift(const QString& _operator, const QString& _model, const QTime _timeStart, const QTime _timeEnd, const QString& logs)
{
    if (_timeStart >= _timeEnd)
    {
        QMessageBox::critical(nullptr, "Error", "Start time must be earlier than end time.");
    }
    QSqlQuery query;

    int operator_id = [=]()
    {
        QSqlQuery index;
        index.prepare("SELECT id FROM users WHERE name = :name");
        index.bindValue(":name", _operator);
        index.exec();
        if (index.next())
        {
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
        if (index.next())
        {
            return index.value(0).toInt();
        }
        return -1;
    }();

    if (operator_id == -1 || model_id == -1)
    {
        QMessageBox::critical(nullptr, "Error", "Model or operator was not found!");
        return;
    }

    int total_seconds = _timeStart.secsTo(_timeEnd);
    float total_hours = total_seconds / 3600.0;

    query.prepare("INSERT INTO shifts (operator_id, model_id, start_time, end_time, total_hours, status, created_by,logs) "
                  "VALUES (:operator_id, :model_id, :start_time, :end_time, :total_hours, 'Создан', 1, :logs)");
    query.bindValue(":operator_id", operator_id);
    query.bindValue(":model_id", model_id);
    query.bindValue(":start_time", _timeStart.toString("HH:mm"));
    query.bindValue(":end_time", _timeEnd.toString("HH:mm"));
    query.bindValue(":logs", logs);
    query.bindValue(":total_hours", total_hours);

    if (!query.exec())
    {
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

bool LogicDataBase::login(const QString& name, const QString&_password)
{
    QSqlQuery query;
    query.prepare("SELECT id,name,password from users where name = :name");
    query.bindValue(":name", name);
    query.exec();
    while (query.next())
    {
        QString password = query.value("password").toString();
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

QString LogicDataBase::getRole()
{
    return role;
}

void LogicDataBase::uploadImageToDB(const QPixmap &pixmap, int index, bool isShift)
{
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::WriteOnly);
    pixmap.save(&buffer, "PNG"); 

    if (!QSqlDatabase::database().isOpen()) {
        qDebug() << "Database is not open";
        return;
    }

    QSqlQuery query;

    if (isShift) {
        query.prepare("INSERT INTO shift_screenshots (shift_id, screenshot, upload_time) "
                      "VALUES (:id, :screenshot, :uploaded_time)");
    } else {
        query.prepare("INSERT INTO report_screenshots (report_id, screenshot, upload_time) "
                      "VALUES (:id, :screenshot, :uploaded_time)");
    }

    query.bindValue(":id", index);
    query.bindValue(":screenshot", byteArray);
    query.bindValue(":uploaded_time", QDateTime::currentDateTime());

    if (!query.exec()) {
        qDebug() << "Failed to upload image: " << query.lastError().text();
        qDebug() << "Executed Query: " << query.lastQuery();
    } else {
        if (query.numRowsAffected() > 0) {
            qDebug() << "Image uploaded successfully!";
        } else {
            qDebug() << "Query executed, but no rows were affected. Verify your data.";
        }
    }
}

void LogicDataBase::updateShift(const QString& operatorName, const QString &modelName, const QTime _timeStart, const QTime _timeEnd, const QString& logs, int indexOfShift)
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
        if (index.next())
        {
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
        if (index.next())
        {
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

    if (query.exec())
    {
        QMessageBox::information(nullptr, "Shift Updated", "Shift has been successfully updated.");
    }
    else
    {
        QMessageBox::warning(nullptr, "Error", "Failed to update shift: " + query.lastError().text());
        qDebug() << query.lastError().text();
    }
}

bool LogicDataBase::startShift(int indexOfShift)
{
    QSqlQuery query;
    qDebug() << indexOfShift;
    query.prepare("UPDATE shifts SET start_time = :start_time, status = 'В работе' WHERE id = :id");
    query.bindValue(":id", indexOfShift);
    query.bindValue(":start_time", QTime::currentTime().toString("HH:mm"));
    
    if (!query.exec())
    {
        qDebug() << "Error starting shift:" << query.lastError().text();
        return false;  
    }
    return true;  
}

QSqlQueryModel* LogicDataBase::getModels() {
    QSqlQueryModel* model = new QSqlQueryModel();
    model->setQuery("SELECT name FROM models");
    return model;
}

QSqlQueryModel* LogicDataBase::getOperators() {
    QSqlQueryModel* model = new QSqlQueryModel();
    model->setQuery("SELECT name FROM users WHERE role = 'operator'");
    return model;
}

QSqlRecord LogicDataBase::getShiftById(int shiftId) {
    QSqlQuery query;
    query.prepare("SELECT operator_id, model_id, start_time, end_time, logs FROM shifts WHERE id = :id");
    query.bindValue(":id", shiftId);
    query.exec();
    
    if (query.next()) {
        return query.record();
    }
    return QSqlRecord();
}

QString LogicDataBase::getOperatorNameById(int operatorId) {
    QSqlQuery query;
    query.prepare("SELECT name FROM users WHERE id = :id");
    query.bindValue(":id", operatorId);
    query.exec();
    
    if (query.next()) {
        return query.value(0).toString();
    }
    return QString();
}

QString LogicDataBase::getModelNameById(int modelId) {
    QSqlQuery query;
    query.prepare("SELECT name FROM models WHERE id = :id");
    query.bindValue(":id", modelId);
    query.exec();
    
    if (query.next()) {
        return query.value(0).toString();
    }
    return QString();
}

QSqlQuery LogicDataBase::getAllShifts(bool isAdmin, int userId) {
    QSqlQuery query;

    if (isAdmin) {
        query.prepare(R"(
            SELECT shifts.id, users.name AS operator_name, models.name AS model_name, shifts.start_time, shifts.end_time, shifts.total_hours, shifts.status
            FROM shifts
            JOIN users ON shifts.operator_id = users.id
            JOIN models ON shifts.model_id = models.id
        )");
    } else {
        query.prepare(R"(
            SELECT shifts.id, users.name AS operator_name, models.name AS model_name, shifts.start_time, shifts.end_time, shifts.total_hours, shifts.status
            FROM shifts
            JOIN users ON shifts.operator_id = users.id
            JOIN models ON shifts.model_id = models.id
            WHERE shifts.operator_id = :idUser
        )");
        query.bindValue(":idUser", userId);
    }

    if (!query.exec()) {
        qDebug() << "Query execution error: " << query.lastError().text();
    }
    return query;
}

QSqlQuery LogicDataBase::getLogs(bool isAdmin, int selectedShiftId)
{
    QSqlQuery query;
    if (isAdmin) {
        query.prepare("SELECT logs FROM shifts WHERE id = :id2");
    } else {
        query.prepare("SELECT logs FROM shifts WHERE id = :id2");
        
    }
    query.bindValue(":id2", selectedShiftId);
    
    if (!query.exec()) {
        qDebug() << "Query error:" << query.lastError();
    }
    
    return query;
}

QSqlQuery LogicDataBase::getTimings(int shiftId)
{
    QSqlQuery query;
    query.prepare("SELECT start_time, end_time FROM shifts WHERE id = :id");
    query.bindValue(":id", shiftId);
    
    if (!query.exec()) {
        qDebug() << "Query error:" << query.lastError();
        return query; 
    }
    
    if (!query.next()) {
        qDebug() << "No timings found for shift ID:" << shiftId;
    }
    
    return query;  
}

QTime LogicDataBase::getTime(int shiftId)
{
    QSqlQuery query;
    query.prepare("select start_time from shifts where id = :id");
    query.bindValue(":id", shiftId);
    if (query.exec()){
        qDebug() << query.lastError();
        return QTime();
    }
    return query.value(0).toTime();

}

QString LogicDataBase::getStatus(int shiftId)
{
    QSqlQuery query;
    query.prepare("SELECT status FROM shifts WHERE id = :id");
    query.bindValue(":id", shiftId);
    qDebug() << "1";
    if (!query.exec()) {
        qDebug() << "Query execution error:" << query.lastError();
        return QString();  
    }

    if (query.next()) {
        return query.value(0).toString();  
    }

    qDebug() << "No result found for shift ID:" << shiftId;
    return QString();
}

QSqlQuery LogicDataBase::getReports()
{
    QSqlQuery query;
    query.prepare("SELECT reports.*, users.name AS operator_name, models.name AS model_name "
                  "FROM reports "
                  "JOIN shifts ON reports.shift_id = shifts.id "
                  "JOIN users ON shifts.operator_id = users.id "
                  "JOIN models ON shifts.model_id = models.id");
    
    if (!query.exec()) {
        qDebug() << "Error executing query:" << query.lastError();
    }
    
    return query;
}

QSqlQuery LogicDataBase::getShiftImages(int shiftId)
{
    QSqlQuery query;
    qDebug() << "id: " << shiftId;
    query.prepare("SELECT * FROM shift_screenshots WHERE shift_id = :id");
    query.bindValue(":id", shiftId);

    if (!query.exec()) {
        qDebug() << "Error executing query:" << query.lastError();
    }
    
    return query;  
}

QSqlQuery LogicDataBase::getReportImages(int shiftId)
{
    QSqlQuery query;
    qDebug() << shiftId;
    query.prepare("select * from report_screenshots where report_id = :id");
    query.bindValue(":id", shiftId);

    if (!query.exec()) {
        qDebug() << "Error executing query:" << query.lastError();
    }
    
    return query;
}

int LogicDataBase::getReportId(int shiftId)
{
    QSqlQuery query;
    query.prepare("SELECT id FROM reports WHERE shift_id = :id");
    query.bindValue(":id", shiftId);

    if (!query.exec()) {
        qDebug() << "Error executing query:" << query.lastError();
        return -1;  
    }

    if (query.next()) {
        int reportId = query.value(0).toInt();
        qDebug() << "Report ID found:" << reportId;
        return reportId;
    } else {
        qDebug() << "No report found for shiftId:" << shiftId;
        return -1;  
    }
}

bool LogicDataBase::createReport(int shiftId, int operatorId, const QString& report, const QString& total_earnings, bool stoppedManually, const QTime start_time)
{
    QSqlQuery query;

    query.prepare("INSERT INTO reports (shift_id, operator_id, report, total_earnings, uploaded_at, worked_hours, manually_stopped) "
                  "VALUES (:shift_id, :operator_id, :report, :total_earnings, :uploaded_at, :worked_hours, :manually_stopped);");

    int millisecondsDiff = start_time.msecsTo(QTime::currentTime()); 
    double worked_hours = millisecondsDiff / (1000.0 * 60 * 60); 

    query.bindValue(":shift_id", shiftId);
    query.bindValue(":operator_id", operatorId);
    query.bindValue(":report", report);
    query.bindValue(":total_earnings", total_earnings.toDouble());
    query.bindValue(":uploaded_at", QDateTime::currentDateTime());
    query.bindValue(":worked_hours", worked_hours);
    query.bindValue(":manually_stopped", stoppedManually);

    if (!query.exec())
    {
        qDebug() << "Error inserting shift report:" << query.lastError().text();
        return false;  
    }

    query.prepare("UPDATE shifts SET status = 'Завершен' WHERE id = :id");
    query.bindValue(":id", shiftId);

    if (!query.exec())
    {
        qDebug() << "Error updating shift status:" << query.lastError().text();
        return false;  
    }

    return true;
}

bool LogicDataBase::addUser(const QString &username, const QString &password, const QString &role)
{
    QSqlQuery query;
    if (role == "model"){
        query.prepare("INSERT INTO models (name) "
                    "VALUES (:username");
        query.bindValue("name", username);
        if (!query.exec()) {
            qDebug() << query.lastError();
            return false;
        } else {
            return true;
        }
    }
    else{
        query.prepare("INSERT INTO users (name, password, role) "
                    "VALUES (:name, :password, :role)");
        query.bindValue(":name", username);
        query.bindValue(":password", password);  
        query.bindValue(":role", role);
        
        if (!query.exec()) {
            qDebug() << query.lastError();
            return false;
        } else {
            return true;
        }
    }
    
}
bool LogicDataBase::deleteCompletedShiftsAndReorder()
{
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) {
        qDebug() << "Database is not open.";
        return false;
    }

    db.transaction();  // Start transaction

    QSqlQuery deleteScreenshotsQuery;
    deleteScreenshotsQuery.prepare(
        "DELETE FROM report_screenshots "
        "WHERE report_id IN ("
        "  SELECT id FROM reports "
        "  WHERE shift_id IN ("
        "    SELECT id FROM shifts WHERE status = 'Завершены'"
        "  )"
        ")"
    );

    if (!deleteScreenshotsQuery.exec()) {
        qDebug() << "Failed to delete report screenshots: " << deleteScreenshotsQuery.lastError();
        db.rollback();  // Roll back the transaction in case of an error
        return false;
    }

    QSqlQuery deleteReportsQuery;
    deleteReportsQuery.prepare(
        "DELETE FROM reports "
        "WHERE shift_id IN ("
        "  SELECT id FROM shifts WHERE status = 'Завершены'"
        ")"
    );

    if (!deleteReportsQuery.exec()) {
        qDebug() << "Failed to delete reports: " << deleteReportsQuery.lastError();
        db.rollback();  
        return false;
    }

    QSqlQuery deleteShiftsQuery;
    deleteShiftsQuery.prepare(
        "DELETE FROM shifts WHERE status = 'Завершены'"
    );

    if (!deleteShiftsQuery.exec()) {
        qDebug() << "Failed to delete completed shifts: " << deleteShiftsQuery.lastError();
        db.rollback();  
        return false;
    }

    if (!db.commit()) {
        qDebug() << "Failed to commit the transaction.";
        return false;
    }

    qDebug() << "Completed shifts and related records deleted successfully.";

    if (!reorderAutoIncrement("shifts") ||
        !reorderAutoIncrement("reports") ||
        !reorderAutoIncrement("report_screenshots")) {
        return false;
    }

    return true;
}

bool LogicDataBase::reorderAutoIncrement(const QString &tableName)
{
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) {
        qDebug() << "Database is not open.";
        return false;
    }

    db.transaction();  // Start transaction for reordering

    // Create a temporary table with the same structure
    QString tempTableName = tableName + "_temp";

    QSqlQuery createTempTableQuery;
    createTempTableQuery.prepare(QString(
        "CREATE TABLE %1 AS SELECT * FROM %2 WHERE 1=0"
    ).arg(tempTableName, tableName));

    if (!createTempTableQuery.exec()) {
        qDebug() << "Failed to create temporary table: " << createTempTableQuery.lastError();
        db.rollback();
        return false;
    }

    // Copy remaining rows to the temporary table, resetting the ID sequence
    QSqlQuery insertTempTableQuery;
    insertTempTableQuery.prepare(QString(
        "INSERT INTO %1 SELECT * FROM %2 ORDER BY id"
    ).arg(tempTableName, tableName));

    if (!insertTempTableQuery.exec()) {
        qDebug() << "Failed to copy rows to temporary table: " << insertTempTableQuery.lastError();
        db.rollback();
        return false;
    }

    // Drop the original table
    QSqlQuery dropTableQuery;
    dropTableQuery.prepare(QString("DROP TABLE %1").arg(tableName));

    if (!dropTableQuery.exec()) {
        qDebug() << "Failed to drop original table: " << dropTableQuery.lastError();
        db.rollback();
        return false;
    }

    // Rename the temporary table to the original table name
    QSqlQuery renameTableQuery;
    renameTableQuery.prepare(QString("ALTER TABLE %1 RENAME TO %2").arg(tempTableName, tableName));

    if (!renameTableQuery.exec()) {
        qDebug() << "Failed to rename temporary table: " << renameTableQuery.lastError();
        db.rollback();
        return false;
    }

    // Reset the auto-increment sequence to the last ID + 1
    QSqlQuery resetAutoIncrementQuery;
    resetAutoIncrementQuery.prepare(QString(
        "UPDATE sqlite_sequence SET seq = (SELECT MAX(id) FROM %1) WHERE name = '%1'"
    ).arg(tableName));

    if (!resetAutoIncrementQuery.exec()) {
        qDebug() << "Failed to reset auto-increment: " << resetAutoIncrementQuery.lastError();
        db.rollback();
        return false;
    }

    if (!db.commit()) {
        qDebug() << "Failed to commit the reordering transaction.";
        return false;
    }

    qDebug() << "Auto-increment values reordered successfully for table:" << tableName;

    return true;
}
