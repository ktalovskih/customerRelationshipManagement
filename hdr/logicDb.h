#ifndef __LOGICDB_H__
#define __LOGICDB_H__

#include <QtSql>
#include <QString>

class LogicDataBase{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    QString role;
    int idUser;
public:
    LogicDataBase();
    ~LogicDataBase();
    void addShift(const QString _operator, const QString _model,const QTime _timeStart,const QTime _timeEnd, const QString logs);
    void deleteShift(const int indexOfShift);
    void addDiscription();
    QString getRole();
    bool login(const QString name,const QString password);
    int getId();
    void uploadImageToDB(const QPixmap &pixmap, const QString &fileName);
    void updateShift(const QString _operator, const QString _model,const QTime _timeStart,const QTime _timeEnd, const QString logs, int  indexOfShift);
};


#endif // __LOGICDB_H__