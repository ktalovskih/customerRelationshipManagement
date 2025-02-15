#ifndef __CRM_DATABASE_MANAGER_H__
#define __CRM_DATABASE_MANAGER_H__

#include <QtSql>
#include <QString>
#pragma once

class CRMDatabaseManager {
    QSqlDatabase db;
    QString role;
    int employeeId;

public:
    CRMDatabaseManager();
    ~CRMDatabaseManager();

    int getEmployeeId();
    QString getRole();
    QSqlQueryModel* getClients();
    QSqlQueryModel* getEmployees();
    QSqlRecord getSessionById(int sessionId);
    QString getEmployeeNameById(int employeeId);
    QString getClientNameById(int clientId);
    QSqlQuery fetchAllSessions(bool isAdmin, int employeeId);
    QSqlQuery fetchSessionNotes(bool isAdmin, int sessionId);
    QSqlQuery fetchSessionTimings(int sessionId);
    QTime getSessionStartTime(int sessionId);
    QString getSessionStatus(int sessionId);
    QSqlQuery fetchReports();
    QSqlQuery fetchSessionImages(int sessionId);
    QSqlQuery fetchReportImages(int sessionId);
    
    int getReportIdForSession(int sessionId);
    
    bool login(const QString& username, const QString& password);
    void createSessionRecord(const QString& employeeName, const QString& clientName, const QTime startTime, const QTime endTime, const QString& notes);
    void deleteSessionRecord(int sessionId);
    void uploadDocumentToDB(const QPixmap& pixmap, int index, bool isSessionDocument);
    void updateSessionRecord(const QString& employeeName, const QString& clientName, const QTime startTime, const QTime endTime, const QString& notes, int sessionId);
    bool startSession(int sessionId);
    bool createReport(int sessionId, int employeeId, const QString& report, const QString& totalEarnings, bool stoppedManually, const QTime startTime);
    bool addEmployee(const QString& username, const QString& password, const QString& role);
    bool reorderAutoIncrement(const QString& tableName);
    bool deleteCompletedSessionsAndReorder();
};

#endif // __CRM_DATABASE_MANAGER_H__
