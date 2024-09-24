#ifndef A3EA14AA_8C2A_401A_8517_AD677F77D054
#define A3EA14AA_8C2A_401A_8517_AD677F77D054

#include <QMainWindow>
#include "logicDb.h"  
#include "ui_mainWindow.h" 

class MainWindow : public QMainWindow {
    Q_OBJECT 

    int selectedShiftId;
    LogicDataBase* db;  
    std::unique_ptr<Ui::MainWindow> ui;  

public:
    explicit MainWindow(QWidget* parent, LogicDataBase* database);
    ~MainWindow();
    void createShiftForm();
public slots:
    void addTask();
    void showAllShifts();
    void onShiftSelected(QListWidgetItem *item);
    void editShift();
    void createShiftForm(int indexOfShift);
};

#endif /* A3EA14AA_8C2A_401A_8517_AD677F77D054 */
