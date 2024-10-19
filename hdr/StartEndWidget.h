#ifndef __STARTENDWIDGET_H__
#define __STARTENDWIDGET_H__
#include "LogicDataBase.h"
#include <QWidget>
#include <QKeyEvent>
#include <QGridLayout>
#include <QImage>
#include <QMap>
#include <QList>
#include <QClipboard>
#include <QMimeData>
#include <QLabel>
#include <QRandomGenerator>
#include <QDebug>
#include <QtSql>
#include <QPushButton>
#include <QApplication>
#include <QTextEdit>
#include <QFileDialog>
#include <QTimer>
#include <QScrollArea>
#include <QLineEdit>
#include <QMessageBox>
#pragma once

class StartEndWidget : public QWidget{

    Q_OBJECT
    int selectedShiftId;
    int idUser;

    //QList<QImage> screenshots;
    QMap<QString, QImage> mapOfTheImages;
    QGridLayout* gridLayoutForImages;
    QPushButton *buttonDialogFile;
    QVBoxLayout *layout;
    QPushButton *buttonUpload;
    QScrollArea *scrollArea;
    QTextEdit *logText;
    LogicDataBase* db;
    QTimer* timer;
    QLineEdit* information;
    QLineEdit* totalMoney;
    
public:

    explicit StartEndWidget(QWidget* parent = nullptr, int selectedShiftId = -1,LogicDataBase* database = nullptr, int idUser = -1);
    ~StartEndWidget();
    void keyPressEvent(QKeyEvent *event) override;
    
public slots:
    void removeWidgetsFromLayout();
    void showImages();
    void endShift(int operatorId);

};

#endif // __STARTENDWIDGET_H__