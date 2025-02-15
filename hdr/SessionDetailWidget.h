#ifndef __SESSIONDETAILWIDGET_H__
#define __SESSIONDETAILWIDGET_H__

#include "CRMDatabaseManager.h"
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

class SessionDetailWidget : public QWidget {
    Q_OBJECT
    int selectedSessionId;
    int employeeId;
    QMap<QString, QImage> imageMap;
    QGridLayout* gridLayoutForImages;
    QPushButton* buttonDialogFile;
    QVBoxLayout* layout;
    QPushButton* buttonUpload;
    QScrollArea* scrollArea;
    QTextEdit* logText;
    CRMDatabaseManager* databaseManager;
    QTimer* timer;
    QLineEdit* information;
    QLineEdit* totalEarnings;
    
public:
    explicit SessionDetailWidget(QWidget* parent = nullptr, int selectedSessionId = -1, CRMDatabaseManager* database = nullptr, int employeeId = -1);
    ~SessionDetailWidget();
    void keyPressEvent(QKeyEvent *event) override;
    
public slots:
    void removeWidgetsFromLayout();
    void showImages();
    void endSession(int employeeId);
};

#endif // __SESSIONDETAILWIDGET_H__
