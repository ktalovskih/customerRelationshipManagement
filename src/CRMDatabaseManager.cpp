#include "CRMDatabaseManager.h"
#include "DashboardWindow.h"
#include <QTime>
#include <QPixmap>
#include <QLabel>
#include <iostream>
#include <filesystem>
#include <QMessageBox>
#include <QBuffer>
#include <QDateTime>
#include <QSqlError>
#include <QDebug>


CRMDatabaseManager::CRMDatabaseManager()
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("/home/zxc/xdxd/crm.db");

    if (!db.open())
        qDebug() << "Database connection failed:" << db.lastError().text();
}

CRMDatabaseManager::~CRMDatabaseManager()
{
    db.close();
}

bool CRMDatabaseManager::login(const QString &username, const QString &password) {
    QSqlQuery query;
    query.prepare("SELECT hashed_password FROM employees WHERE username = :username");
    query.bindValue(":username", username);
    
    if (!query.exec() || !query.next()) {
        return false;  
    }

    std::string storedHash = query.value(0).toString().toStdString();

    if (crypto_pwhash_str_verify(storedHash.c_str(), password.toStdString().c_str(), password.length()) == 0) {
        role = [&](){
            QSqlQuery query;
            query.prepare("SELECT role from employees WHERE username = :username");
            query.bindValue(":username", username);
            if (!query.exec() || !query.next()) {
                qDebug() << "!"; 
            }
            return query.value(0).toString();
        }();
        employeeId = [&](){
            QSqlQuery query;
            query.prepare("SELECT employee_id from employees WHERE username = :username");
            query.bindValue(":username", username);
            if (!query.exec() || !query.next()) {
                qDebug() << "!"; 
            }
            return query.value(0).toInt();
        }();
        return true;  
    }

    return false;  
}



void CRMDatabaseManager::createSessionRecord(const QString& employeeName, const QString& clientName, const QTime startTime, const QTime endTime, const QString& notes)
{
    if (startTime >= endTime)
    {
        QMessageBox::critical(nullptr, "Error", "Start time must be earlier than end time.");
        return;
    }

    QSqlQuery query;
    int empId = [=]() {
        QSqlQuery idx;
        idx.prepare("SELECT employee_id FROM employees WHERE full_name = :name");
        idx.bindValue(":name", employeeName);
        idx.exec();
        if (idx.next()) return idx.value(0).toInt();
        return -1;
    }();

    int cliId = [=]() {
        QSqlQuery idx;
        idx.prepare("SELECT client_id FROM clients WHERE full_name = :name");
        idx.bindValue(":name", clientName);
        idx.exec();
        if (idx.next()) return idx.value(0).toInt();
        return -1;
    }();

    if (empId == -1 || cliId == -1)
    {
        QMessageBox::critical(nullptr, "Error", "Employee or client was not found!");
        return;
    }

    int totalSeconds = startTime.secsTo(endTime);
    float totalHours = totalSeconds / 3600.0;

    query.prepare("INSERT INTO work_sessions (employee_id, client_id, start_time, end_time, total_hours, session_status, session_notes) "
                  "VALUES (:employee_id, :client_id, :start_time, :end_time, :total_hours, 'Created', :notes)");
    query.bindValue(":employee_id", empId);
    query.bindValue(":client_id", cliId);
    query.bindValue(":start_time", startTime.toString("HH:mm"));
    query.bindValue(":end_time", endTime.toString("HH:mm"));
    query.bindValue(":notes", notes);
    query.bindValue(":total_hours", totalHours);

    if (!query.exec())
    {
        QMessageBox::critical(nullptr, "Database Error", query.lastError().text());
    }
}

void CRMDatabaseManager::deleteSessionRecord(int sessionId)
{
    QSqlQuery query;
    query.prepare("DELETE FROM work_sessions WHERE session_id = :id");
    query.bindValue(":id", sessionId);
    if (!query.exec())
    {
        qDebug() << "Failed to delete session:" << query.lastError().text();
    }
}

bool CRMDatabaseManager::startSession(int sessionId)
{
    QSqlQuery query;
    qDebug() << sessionId;
    query.prepare("UPDATE work_sessions SET start_time = :start_time, session_status = 'In Progress' WHERE session_id = :id");
    query.bindValue(":id", sessionId);
    query.bindValue(":start_time", QTime::currentTime().toString("HH:mm"));
    if (!query.exec())
    {
        qDebug() << "Error starting session:" << query.lastError().text();
        return false;
    }
    return true;
}

int CRMDatabaseManager::getEmployeeId()
{
    return employeeId;
}

QString CRMDatabaseManager::getRole()
{
    return role;
}

QSqlQueryModel* CRMDatabaseManager::getClients() {
    QSqlQueryModel* model = new QSqlQueryModel();
    model->setQuery("SELECT full_name FROM clients");
    return model;
}

QSqlQueryModel* CRMDatabaseManager::getEmployees() {
    QSqlQueryModel* model = new QSqlQueryModel();
    model->setQuery("SELECT full_name FROM employees WHERE role = 'employee'");
    return model;
}

QSqlRecord CRMDatabaseManager::getSessionById(int sessionId) {
    QSqlQuery query;
    query.prepare("SELECT employee_id, client_id, start_time, end_time, session_notes FROM work_sessions WHERE session_id = :id");
    query.bindValue(":id", sessionId);
    query.exec();
    if (query.next()) {
        return query.record();
    }
    return QSqlRecord();
}

QString CRMDatabaseManager::getEmployeeNameById(int empId) {
    QSqlQuery query;
    query.prepare("SELECT full_name FROM employees WHERE employee_id = :id");
    query.bindValue(":id", empId);
    query.exec();
    if (query.next()) {
        return query.value(0).toString();
    }
    return QString();
}

QString CRMDatabaseManager::getClientNameById(int cliId) {
    QSqlQuery query;
    query.prepare("SELECT full_name FROM clients WHERE client_id = :id");
    query.bindValue(":id", cliId);
    query.exec();
    if (query.next()) {
        return query.value(0).toString();
    }
    return QString();
}
// Part 2 of CRMDatabaseManager.cpp

QSqlQuery CRMDatabaseManager::fetchAllSessions(bool isAdmin, int userId) {
    QSqlQuery query;
    if (isAdmin) {
        query.prepare(R"(
            SELECT work_sessions.session_id, employees.full_name AS employee_name, clients.full_name AS client_name,
                   work_sessions.start_time, work_sessions.end_time, work_sessions.total_hours, work_sessions.session_status
            FROM work_sessions
            JOIN employees ON work_sessions.employee_id = employees.employee_id
            JOIN clients ON work_sessions.client_id = clients.client_id
        )");
    } else {
        query.prepare(R"(
            SELECT work_sessions.session_id, employees.full_name AS employee_name, clients.full_name AS client_name,
                   work_sessions.start_time, work_sessions.end_time, work_sessions.total_hours, work_sessions.session_status
            FROM work_sessions
            JOIN employees ON work_sessions.employee_id = employees.employee_id
            JOIN clients ON work_sessions.client_id = clients.client_id
            WHERE work_sessions.employee_id = :idUser
        )");
        query.bindValue(":idUser", userId);
        qDebug() << "id" << userId;
    }
    if (!query.exec())
        qDebug() << "Query execution error:" << query.lastError().text();
    return query;
}

QSqlQuery CRMDatabaseManager::fetchSessionNotes(bool isAdmin, int sessionId) {
    QSqlQuery query;
    query.prepare("SELECT session_notes FROM work_sessions WHERE session_id = :id");
    query.bindValue(":id", sessionId);
    if (!query.exec())
        qDebug() << "Query error:" << query.lastError();
    return query;
}

QSqlQuery CRMDatabaseManager::fetchSessionTimings(int sessionId) {
    QSqlQuery query;
    query.prepare("SELECT start_time, end_time FROM work_sessions WHERE session_id = :id");
    query.bindValue(":id", sessionId);
    if (!query.exec()) {
        qDebug() << "Query error:" << query.lastError();
        return query;
    }
    if (!query.next())
        qDebug() << "No timings found for session ID:" << sessionId;
    return query;
}

QTime CRMDatabaseManager::getSessionStartTime(int sessionId) {
    QSqlQuery query;
    query.prepare("SELECT start_time FROM work_sessions WHERE session_id = :id");
    query.bindValue(":id", sessionId);
    if (!query.exec()) {
        qDebug() << "Error fetching start time:" << query.lastError();
        return QTime();
    }
    if (query.next())
        return query.value(0).toTime();
    return QTime();
}

QString CRMDatabaseManager::getSessionStatus(int sessionId) {
    QSqlQuery query;
    query.prepare("SELECT session_status FROM work_sessions WHERE session_id = :id");
    query.bindValue(":id", sessionId);
    if (!query.exec()) {
        qDebug() << "Query execution error:" << query.lastError();
        return QString();
    }
    if (query.next())
        return query.value(0).toString();
    qDebug() << "No result found for session ID:" << sessionId;
    return QString();
}

QSqlQuery CRMDatabaseManager::fetchReports() {
    QSqlQuery query;
    query.prepare("SELECT reports.*, employees.full_name AS employee_name, clients.full_name AS client_name "
                  "FROM reports "
                  "JOIN work_sessions ON reports.session_id = work_sessions.session_id "
                  "JOIN employees ON work_sessions.employee_id = employees.employee_id "
                  "JOIN clients ON work_sessions.client_id = clients.client_id");
    if (!query.exec())
        qDebug() << "Error executing query:" << query.lastError();
    return query;
}

QSqlQuery CRMDatabaseManager::fetchSessionImages(int sessionId) {
    QSqlQuery query;
    qDebug() << "sessionId:" << sessionId;
    query.prepare("SELECT * FROM session_documents WHERE session_id = :id");
    query.bindValue(":id", sessionId);
    if (!query.exec())
        qDebug() << "Error executing query:" << query.lastError();
    return query;
}

QSqlQuery CRMDatabaseManager::fetchReportImages(int sessionId) {
    QSqlQuery query;
    qDebug() << sessionId;
    query.prepare("SELECT * FROM report_documents WHERE report_id = :id");
    query.bindValue(":id", sessionId);
    if (!query.exec())
        qDebug() << "Error executing query:" << query.lastError();
    return query;
}

int CRMDatabaseManager::getReportIdForSession(int sessionId) {
    QSqlQuery query;
    query.prepare("SELECT report_id FROM reports WHERE session_id = :id");
    query.bindValue(":id", sessionId);
    if (!query.exec()) {
        qDebug() << "Error executing query:" << query.lastError();
        return -1;
    }
    if (query.next()) {
        int reportId = query.value(0).toInt();
        qDebug() << "Report ID found:" << reportId;
        return reportId;
    } else {
        qDebug() << "No report found for sessionId:" << sessionId;
        return -1;
    }
}

bool CRMDatabaseManager::createReport(int sessionId, int empId, const QString& report, const QString& totalEarnings, bool stoppedManually, const QTime start_time) {
    QSqlQuery query;
    query.prepare("INSERT INTO reports (session_id, employee_id, report, total_earnings, uploaded_at, worked_hours, manually_stopped) "
                  "VALUES (:session_id, :employee_id, :report, :total_earnings, :uploaded_at, :worked_hours, :manually_stopped)");
    int millisecondsDiff = start_time.msecsTo(QTime::currentTime());
    double worked_hours = millisecondsDiff / (1000.0 * 60 * 60);
    query.bindValue(":session_id", sessionId);
    query.bindValue(":employee_id", empId);
    query.bindValue(":report", report);
    query.bindValue(":total_earnings", totalEarnings.toDouble());
    query.bindValue(":uploaded_at", QDateTime::currentDateTime());
    query.bindValue(":worked_hours", worked_hours);
    query.bindValue(":manually_stopped", stoppedManually);
    if (!query.exec()) {
        qDebug() << "Error inserting session report:" << query.lastError().text();
        return false;
    }
    query.prepare("UPDATE work_sessions SET session_status = 'Completed' WHERE session_id = :id");
    query.bindValue(":id", sessionId);
    if (!query.exec()) {
        qDebug() << "Error updating session status:" << query.lastError().text();
        return false;
    }
    return true;
}
// Part 3 of CRMDatabaseManager.cpp

void CRMDatabaseManager::uploadDocumentToDB(const QPixmap &pixmap, int index, bool isSessionDocument)
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
    if (isSessionDocument) {
        query.prepare("INSERT INTO session_documents (session_id, document, uploaded_at) "
                      "VALUES (:id, :document, :uploaded_at)");
    } else {
        query.prepare("INSERT INTO report_documents (report_id, document, uploaded_at) "
                      "VALUES (:id, :document, :uploaded_at)");
    }
    query.bindValue(":id", index);
    query.bindValue(":document", byteArray);
    query.bindValue(":uploaded_at", QDateTime::currentDateTime());
    if (!query.exec()) {
        qDebug() << "Failed to upload document:" << query.lastError().text();
        qDebug() << "Executed Query:" << query.lastQuery();
    } else {
        qDebug() << "Document uploaded successfully!";
    }
}

void CRMDatabaseManager::updateSessionRecord(const QString& employeeName, const QString& clientName, const QTime startTime, const QTime endTime, const QString& notes, int sessionId)
{
    QSqlQuery query;
    query.prepare("UPDATE work_sessions SET employee_id = :employeeId, client_id = :clientId, "
                  "start_time = :startTime, end_time = :endTime, session_notes = :notes WHERE session_id = :id");

    int empId = [=]() {
        QSqlQuery idx;
        idx.prepare("SELECT employee_id FROM employees WHERE full_name = :name");
        idx.bindValue(":name", employeeName);
        idx.exec();
        if (idx.next())
            return idx.value(0).toInt();
        return -1;
    }();

    int cliId = [=]() {
        QSqlQuery idx;
        idx.prepare("SELECT client_id FROM clients WHERE full_name = :name");
        idx.bindValue(":name", clientName);
        idx.exec();
        if (idx.next())
            return idx.value(0).toInt();
        return -1;
    }();

    query.bindValue(":employeeId", empId);
    query.bindValue(":clientId", cliId);
    query.bindValue(":startTime", startTime.toString("HH:mm"));
    query.bindValue(":endTime", endTime.toString("HH:mm"));
    query.bindValue(":notes", notes);
    query.bindValue(":id", sessionId);

    if (query.exec()) {
        QMessageBox::information(nullptr, "Session Updated", "Session has been successfully updated.");
    } else {
        QMessageBox::warning(nullptr, "Error", "Failed to update session: " + query.lastError().text());
        qDebug() << query.lastError().text();
    }
}

bool CRMDatabaseManager::addEmployee(const QString &username, const QString &password, const QString &role) {
    QSqlQuery query;
    if (role == "client") {
        query.prepare("INSERT INTO clients (full_name) VALUES (:username)");
        query.bindValue(":username", username);
        if (!query.exec()) {
            qDebug() << query.lastError();
            return false;
        } else {
            return true;
        }
    }
    else {
        query.prepare("INSERT INTO employees (full_name, username, hashed_password, role) VALUES (:full_name, :username, :hashed_password, :role)");
        query.bindValue(":full_name", username);
        query.bindValue(":username", username);
        query.bindValue(":hashed_password", ([&](){
            char hashedPassword[crypto_pwhash_STRBYTES];
            QByteArray passwordBytes = password.toUtf8();
        
            if (crypto_pwhash_str(hashedPassword, passwordBytes.constData(), passwordBytes.size(),
                                  crypto_pwhash_OPSLIMIT_INTERACTIVE, 
                                  crypto_pwhash_MEMLIMIT_INTERACTIVE) != 0) {
                throw std::runtime_error("Password hashing failed");
            }
            return QString::fromStdString(std::string(hashedPassword));
        })());
               
        query.bindValue(":role", role);
        if (!query.exec()) {
            qDebug() << query.lastError();
            return false;
        } else {
            return true;
        }
    }
}

std::string hashPassword(const std::string &password) {
    char hashedPassword[crypto_pwhash_STRBYTES];

    if (crypto_pwhash_str(hashedPassword, password.c_str(), password.length(),
                          crypto_pwhash_OPSLIMIT_INTERACTIVE, 
                          crypto_pwhash_MEMLIMIT_INTERACTIVE) != 0) {
        throw std::runtime_error("Password hashing failed");
    }

    return std::string(hashedPassword);
}

bool CRMDatabaseManager::deleteCompletedSessionsAndReorder() {
    QSqlDatabase database = QSqlDatabase::database();
    if (!database.isOpen()) {
        qDebug() << "Database is not open.";
        return false;
    }
    database.transaction();

    QSqlQuery deleteDocumentsQuery;
    deleteDocumentsQuery.prepare(
        "DELETE FROM report_documents "
        "WHERE report_id IN ("
        "  SELECT report_id FROM reports "
        "  WHERE session_id IN ("
        "    SELECT session_id FROM work_sessions WHERE session_status = 'Completed'"
        "  )"
        ")"
    );
    if (!deleteDocumentsQuery.exec()) {
        qDebug() << "Failed to delete report documents:" << deleteDocumentsQuery.lastError();
        database.rollback();
        return false;
    }

    QSqlQuery deleteReportsQuery;
    deleteReportsQuery.prepare(
        "DELETE FROM reports "
        "WHERE session_id IN ("
        "  SELECT session_id FROM work_sessions WHERE session_status = 'Completed'"
        ")"
    );
    if (!deleteReportsQuery.exec()) {
        qDebug() << "Failed to delete reports:" << deleteReportsQuery.lastError();
        database.rollback();
        return false;
    }

    QSqlQuery deleteSessionsQuery;
    deleteSessionsQuery.prepare("DELETE FROM work_sessions WHERE session_status = 'Completed'");
    if (!deleteSessionsQuery.exec()) {
        qDebug() << "Failed to delete completed sessions:" << deleteSessionsQuery.lastError();
        database.rollback();
        return false;
    }

    if (!database.commit()) {
        qDebug() << "Failed to commit the transaction.";
        return false;
    }

    qDebug() << "Completed sessions and related records deleted successfully.";

    if (!reorderAutoIncrement("work_sessions") ||
        !reorderAutoIncrement("reports") ||
        !reorderAutoIncrement("report_documents")) {
        return false;
    }
    return true;
}

bool CRMDatabaseManager::reorderAutoIncrement(const QString &tableName) {
    QSqlDatabase database = QSqlDatabase::database();
    if (!database.isOpen()) {
        qDebug() << "Database is not open.";
        return false;
    }
    database.transaction();
    QString tempTableName = tableName + "_temp";
    QSqlQuery createTempTableQuery;
    createTempTableQuery.prepare(QString("CREATE TABLE %1 AS SELECT * FROM %2 WHERE 1=0").arg(tempTableName, tableName));
    if (!createTempTableQuery.exec()) {
        qDebug() << "Failed to create temporary table:" << createTempTableQuery.lastError();
        database.rollback();
        return false;
    }
    QSqlQuery insertTempTableQuery;
    insertTempTableQuery.prepare(QString("INSERT INTO %1 SELECT * FROM %2 ORDER BY id").arg(tempTableName, tableName));
    if (!insertTempTableQuery.exec()) {
        qDebug() << "Failed to copy rows to temporary table:" << insertTempTableQuery.lastError();
        database.rollback();
        return false;
    }
    QSqlQuery dropTableQuery;
    dropTableQuery.prepare(QString("DROP TABLE %1").arg(tableName));
    if (!dropTableQuery.exec()) {
        qDebug() << "Failed to drop original table:" << dropTableQuery.lastError();
        database.rollback();
        return false;
    }
    QSqlQuery renameTableQuery;
    renameTableQuery.prepare(QString("ALTER TABLE %1 RENAME TO %2").arg(tempTableName, tableName));
    if (!renameTableQuery.exec()) {
        qDebug() << "Failed to rename temporary table:" << renameTableQuery.lastError();
        database.rollback();
        return false;
    }
    QSqlQuery resetAutoIncrementQuery;
    resetAutoIncrementQuery.prepare(QString("UPDATE sqlite_sequence SET seq = (SELECT MAX(id) FROM %1) WHERE name = '%1'").arg(tableName));
    if (!resetAutoIncrementQuery.exec()) {
        qDebug() << "Failed to reset auto-increment:" << resetAutoIncrementQuery.lastError();
        database.rollback();
        return false;
    }
    if (!database.commit()) {
        qDebug() << "Failed to commit the reordering transaction.";
        return false;
    }
    qDebug() << "Auto-increment values reordered successfully for table:" << tableName;
    return true;
}
