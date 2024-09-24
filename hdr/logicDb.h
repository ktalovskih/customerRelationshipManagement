#ifndef FD2C3853_FC07_4A74_99A9_B22B1E448B61
#define FD2C3853_FC07_4A74_99A9_B22B1E448B61
#include <QtSql>
#include <QString>
class LogicDataBase{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
public:
    LogicDataBase();
    ~LogicDataBase();
    void addShift(const QString _operator, const QString _model,const QTime _timeStart,const QTime _timeEnd);
    void deleteTask();
    void addDiscription();
    bool login(const QString name,const QString password);
};

#endif /* FD2C3853_FC07_4A74_99A9_B22B1E448B61 */
