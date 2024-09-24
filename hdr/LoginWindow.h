#ifndef C5E36286_7F69_4C42_9A47_DEAA86B7E45B
#define C5E36286_7F69_4C42_9A47_DEAA86B7E45B
#include <ui_loginWindow.h>
#include <QMainWindow>
#include <logicDb.h>
#include "MainWindow.h"

class LoginWindow : public QWidget{
    Q_OBJECT
    LogicDataBase* db;   
    std::unique_ptr<Ui::Form> ui; 
public:
    explicit LoginWindow(QWidget* parent = nullptr);
    ~LoginWindow();
signals:
    void loginSuccessful(LogicDataBase* db); 
private slots:
    void attemptLogin();
    
};

#endif /* C5E36286_7F69_4C42_9A47_DEAA86B7E45B */
