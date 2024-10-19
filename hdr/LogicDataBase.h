#ifndef __LOGICDB_H__
#define __LOGICDB_H__

#include <QtSql>
#include <QString>
#pragma once

class LogicDataBase{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    QString role;
    int idUser;
public:
    LogicDataBase();
    ~LogicDataBase();
    //getters
    int getId();
    QString getRole();
    QSqlQueryModel* getModels();
    QSqlQueryModel* getOperators();
    QSqlRecord getShiftById(int shiftId);
    QString getOperatorNameById(int operatorId);
    QString getModelNameById(int modelId);
    QSqlQuery getAllShifts(bool isAdmin, int userId);
    QSqlQuery getLogs(bool isAdmin,int selectedShiftId);
    QSqlQuery getTimings(int shiftId);
    QTime getTime(int shiftId);
    QString getStatus(int shiftId);
    QSqlQuery getReports();
    QSqlQuery getShiftImages(int shiftId);
    QSqlQuery getReportImages(int shiftId);
    
    int getReportId(int shiftId);
    //
    bool login(const QString& name,const QString& password);

    void addShift(const QString& _operator, const QString& _model,const QTime _timeStart,const QTime _timeEnd, const QString& logs);
    void deleteShift(const int indexOfShift);
    void uploadImageToDB(const QPixmap &pixmap,int index, bool isShift);
    void updateShift(const QString& _operator, const QString& _model,const QTime _timeStart,const QTime _timeEnd, const QString& logs, int indexOfShift);
    bool startShift(int indexOfShift);
    bool createReport(int shiftId, int operatorId, const QString& report, const QString& total_earnings, bool stoppedManually, const QTime start_time);
    void addDiscription(); // i do not know for that it exists 
    bool addUser(const QString &username, const QString &password, const QString &role);
    bool reorderAutoIncrement(const QString &tableName);
    bool deleteCompletedShiftsAndReorder();
};


#endif // __LOGICDB_H__