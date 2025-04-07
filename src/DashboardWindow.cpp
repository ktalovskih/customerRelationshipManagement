#include "DashboardWindow.h"
#include <QPushButton>
#include <QLineEdit>
#include <QCompleter>
#include <QTimeEdit>
#include <QTime>
#include <QLabel>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextEdit>
#include <QClipboard>
#include <QPixmap>
#include <QScrollArea>
#include <QComboBox>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QSqlQueryModel>
#include <QSqlRecord>
#include <QSqlQuery>
#include <QDebug>

DashboardWindow::DashboardWindow(QWidget *parent, CRMDatabaseManager *database, bool isAdmin, int employeeId)
    : QMainWindow(parent), databaseManager(database), isAdmin(isAdmin), employeeId(employeeId), updateTimer(new QTimer())
{
    this->setFocusPolicy(Qt::StrongFocus);
    updateTimer->start(5000);
    QObject::connect(updateTimer, &QTimer::timeout, [this](){
        showAllSessions();
        updateTimer->start(5000);
    });
    if (isAdmin)
    {
        uiAdmin = std::make_unique<Ui::mainWindowAdmin>();
        uiAdmin->setupUi(this);
        showAllSessions();
        QObject::connect(uiAdmin->pushButton, &QPushButton::pressed, this, &DashboardWindow::addTask);

        QObject::connect(uiAdmin->listWidget, &QListWidget::itemClicked, this, &DashboardWindow::onSessionSelected);
        QObject::connect(uiAdmin->listWidget_2, &QListWidget::itemClicked, this, &DashboardWindow::onSessionSelected);

        QObject::connect(uiAdmin->listWidget, &QListWidget::doubleClicked, this, &DashboardWindow::showWholeInformation);
        QObject::connect(uiAdmin->listWidget_2, &QListWidget::doubleClicked, this, &DashboardWindow::showWholeInformation);

        QObject::connect(uiAdmin->pushButton_2, &QPushButton::clicked, this, &DashboardWindow::showAllSessions);
        QObject::connect(uiAdmin->pushButton_3, &QPushButton::clicked, this, &DashboardWindow::addNewUser);
        QObject::connect(uiAdmin->pushButton_4, &QPushButton::clicked, [this](){
            databaseManager->deleteCompletedSessionsAndReorder();
            showAllSessions();
        });
    }
    else
    {
        uiOperator = std::make_unique<Ui::mainWindowOperator>();
        uiOperator->setupUi(this);
        QObject::connect(uiOperator->pushButton_3, &QPushButton::pressed, this, &DashboardWindow::showAllSessions);

        QObject::connect(uiOperator->listWidget, &QListWidget::itemClicked, this, &DashboardWindow::onSessionSelected);
        QObject::connect(uiOperator->listWidget_2, &QListWidget::itemClicked, this, &DashboardWindow::onSessionSelected);
        
        QObject::connect(uiOperator->pushButton, &QPushButton::pressed, [this]() {
            auto* sessionDetailWidget = new SessionDetailWidget(nullptr, selectedSessionId, this->databaseManager);
            sessionDetailWidgets.insert(std::make_pair(selectedSessionId, sessionDetailWidget));
            sessionDetailWidget->show();
            this->showAllSessions();
            QObject::connect(sessionDetailWidget, &QWidget::isVisible, [this](){
                this->showAllSessions();
            });
        });
        
        QObject::connect(uiOperator->pushButton_2, &QPushButton::pressed, this, [this](){
            auto temp = sessionDetailWidgets.find(selectedSessionId);
            auto* widget = temp->second;
            widget->endSession(temp->first);
            QObject::connect(widget, &QWidget::destroyed, [this, temp](){
                sessionDetailWidgets.erase(temp->first);
            });
            showAllSessions(); 
        });

        QObject::connect(uiOperator->listWidget_2, &QListWidget::doubleClicked, this, &DashboardWindow::showWholeInformation);
        QObject::connect(uiOperator->listWidget, &QListWidget::doubleClicked, this, &DashboardWindow::showWholeInformation);

        showAllSessions();
    }
    this->resize(1320,700);
}

DashboardWindow::~DashboardWindow()
{
    delete databaseManager;
}

void DashboardWindow::onSessionSelected(QListWidgetItem *item)
{
    selectedSessionId = item->data(Qt::UserRole).toInt();
}

void DashboardWindow::editSession()
{
    createSessionForm(selectedSessionId);
}

void DashboardWindow::addTask()
{
    createSessionForm(-1);
}

void DashboardWindow::createSessionForm(int indexOfSession)
{
    QSqlQueryModel *model1 = databaseManager->getClients();
    QSqlQueryModel *model2 = databaseManager->getEmployees();

    QWidget *widget = new QWidget;
    QGridLayout *layout = new QGridLayout;

    auto *startTime = new QTimeEdit();
    auto *endTime = new QTimeEdit();
    startTime->setTime(QTime::currentTime());
    endTime->setTime(QTime::currentTime());

    startTime->setDisplayFormat("HH:mm");
    endTime->setDisplayFormat("HH:mm");
    auto *notes = new QTextEdit();

    auto button = new QPushButton(indexOfSession == -1 ? "Create Session" : "Edit Session");

    QComboBox *comboBoxClient = new QComboBox;
    QComboBox *comboBoxEmployee = new QComboBox;

    comboBoxClient->setModel(model1);
    comboBoxClient->setModelColumn(0);

    comboBoxEmployee->setModel(model2);
    comboBoxEmployee->setModelColumn(0);

    QCompleter *completer1 = new QCompleter(model1, this);
    completer1->setCompletionColumn(0);
    completer1->setCompletionMode(QCompleter::PopupCompletion);
    completer1->setCaseSensitivity(Qt::CaseInsensitive);
    comboBoxClient->setCompleter(completer1);

    QCompleter *completer2 = new QCompleter(model2, this);
    completer2->setCompletionColumn(0);
    completer2->setCompletionMode(QCompleter::PopupCompletion);
    completer2->setCaseSensitivity(Qt::CaseInsensitive);
    comboBoxEmployee->setCompleter(completer2);

    comboBoxClient->setEditable(true);
    comboBoxEmployee->setEditable(true);

    comboBoxEmployee->setPlaceholderText("Select Employee");
    comboBoxClient->setPlaceholderText("Select Client");

    startTime->setKeyboardTracking(true);
    endTime->setKeyboardTracking(true);

    if (indexOfSession != -1)
    {
        QSqlRecord sessionRecord = databaseManager->getSessionById(indexOfSession);
        if (!sessionRecord.isEmpty()) {
            int empId = sessionRecord.value("employee_id").toInt();
            int cliId = sessionRecord.value("client_id").toInt();
            QTime start = QTime::fromString(sessionRecord.value("start_time").toString(), "HH:mm");
            QTime end = QTime::fromString(sessionRecord.value("end_time").toString(), "HH:mm");
            QString notesText = sessionRecord.value("session_notes").toString();

            QString employeeName = databaseManager->getEmployeeNameById(empId);
            QString clientName = databaseManager->getClientNameById(cliId);

            comboBoxEmployee->setCurrentText(employeeName);
            comboBoxClient->setCurrentText(clientName);
            startTime->setTime(start);
            endTime->setTime(end);
            notes->setText(notesText);
        }
    }

    auto *deleteButton = new QPushButton("Delete");
    widget->setLayout(layout);
    layout->addWidget(comboBoxEmployee, 0, 0);
    layout->addWidget(comboBoxClient, 0, 1);
    layout->addWidget(startTime, 1, 0);
    layout->addWidget(endTime, 1, 1);
    layout->addWidget(notes, 2, 0, 2, 3);
    layout->setRowStretch(3, 1);
    layout->addWidget(button, 4, 0, 1, 2);
    if (indexOfSession != -1)
        layout->addWidget(deleteButton, 5, 0, 1, 2);
    widget->show();

    QObject::connect(deleteButton, &QPushButton::clicked, [=]() {
        databaseManager->deleteSessionRecord(indexOfSession);
        uiAdmin->listWidget->clear();
        showAllSessions();
        widget->deleteLater();
    });

    QObject::connect(button, &QPushButton::pressed, [=]() {
        QString employeeName = comboBoxEmployee->currentText();
        QString clientName = comboBoxClient->currentText();
        QTime start = startTime->time();
        QTime end = endTime->time();
        QString notesText = notes->toPlainText();
        if (indexOfSession == -1) {
            databaseManager->createSessionRecord(employeeName, clientName, start, end, notesText);
            QMessageBox::information(widget, "Session Created", "Session has been successfully created.");
        } else {
            databaseManager->updateSessionRecord(employeeName, clientName, start, end, notesText, indexOfSession);
        }
        uiAdmin->listWidget->clear();
        showAllSessions();
        widget->deleteLater();
    });
}

void DashboardWindow::showReport()
{
    
}

void DashboardWindow::showWholeInformation()
{
    auto* editButton = new QPushButton("Edit Session");
    QObject::connect(editButton, &QPushButton::clicked, this, &DashboardWindow::editSession);
    auto *widget = new QWidget;
    auto *layout = new QVBoxLayout(widget);

    QString status = databaseManager->getSessionStatus(selectedSessionId);
    qDebug() << status;

    if (status == "In Progress" || status == "Completed" || status == "in progress") {
        auto *widgetForImages = new QWidget(widget);
        QListWidget *listWidget = new QListWidget(widgetForImages);

        QSqlQuery query = databaseManager->fetchSessionImages(selectedSessionId);
        while (query.next()) {
            QListWidgetItem *item = new QListWidgetItem;
            QByteArray imageData = query.value("document").toByteArray();
            QPixmap map;
            if (!map.loadFromData(imageData)) {
                continue;
            }
            item->setIcon(QIcon(map));
            item->setData(Qt::UserRole, map);
            listWidget->addItem(item);
        }
        if (status == "Completed") {
            QSqlQuery query = databaseManager->fetchReportImages(selectedSessionId);
            while (query.next()) {
                QListWidgetItem *item = new QListWidgetItem;
                QByteArray imageData = query.value("document").toByteArray();
                QPixmap map;
                if (!map.loadFromData(imageData)) {
                    qDebug() << "Failed to load image data";
                    continue;
                }
                item->setIcon(QIcon(map));
                item->setData(Qt::UserRole, map);
                listWidget->addItem(item);
            }
        }
        auto *imagesLayout = new QVBoxLayout(widgetForImages);
        imagesLayout->addWidget(listWidget);
        widgetForImages->setLayout(imagesLayout);
        widgetForImages->setMinimumSize(400, 200);
        layout->addWidget(widgetForImages);
        connect(listWidget, &QListWidget::itemClicked, [=](QListWidgetItem *item) {
            QPixmap pixmap = item->data(Qt::UserRole).value<QPixmap>();
            QLabel* label = new QLabel();
            label->setPixmap(pixmap);
            label->show();
        });
    }

    auto *text = new QTextEdit(widget);
    text->setReadOnly(true);
    QSqlQuery logQuery = databaseManager->fetchSessionNotes(isAdmin, selectedSessionId);
    QString buffer;
    while (logQuery.next()) {
        buffer += logQuery.value(0).toString();
        buffer += "\n";
    }
    text->setText(buffer);
    layout->addWidget(text);
    widget->setLayout(layout);
    widget->setMinimumSize(800, 600);
    widget->setAttribute(Qt::WA_DeleteOnClose);
    if(this->isAdmin)
        layout->addWidget(editButton);
    widget->show();
}

void DashboardWindow::addNewUser()
{
    auto *widget = new QWidget();
    auto *layout = new QGridLayout(widget);

    auto *labelUsername = new QLabel("Username:", widget);
    auto *editUsername = new QLineEdit(widget);

    auto *labelPassword = new QLabel("Password:", widget);
    auto *editPassword = new QLineEdit(widget);
    editPassword->setEchoMode(QLineEdit::Password);

    auto *labelRole = new QLabel("Role:", widget);
    auto *comboBoxRoles = new QComboBox(widget);
    comboBoxRoles->addItems({"admin", "employee", "client"});

    auto *buttonOk = new QPushButton("OK", widget);

    layout->addWidget(labelUsername, 0, 0);
    layout->addWidget(editUsername, 0, 1);
    layout->addWidget(labelPassword, 1, 0);
    layout->addWidget(editPassword, 1, 1);
    layout->addWidget(labelRole, 2, 0);
    layout->addWidget(comboBoxRoles, 2, 1);
    layout->addWidget(buttonOk, 3, 0);

    widget->setLayout(layout);
    widget->setWindowTitle("Add New User");
    widget->setMinimumSize(300, 200);
    widget->show();

    connect(comboBoxRoles, &QComboBox::currentIndexChanged, [=]() {
        if (comboBoxRoles->currentText() == "model") {
            editPassword->hide();
            labelPassword->hide();
        } else {
            editPassword->show();
            labelPassword->show();
        }
    });

    connect(buttonOk, &QPushButton::clicked, this, [=]() {
        QString username = editUsername->text();
        QString password = editPassword->text();
        QString role = comboBoxRoles->currentText();

        if (username.isEmpty()) {
            QMessageBox::warning(widget, "Input Error", "Username cannot be empty.");
            return;
        }

        if (role != "model" && password.isEmpty()) {
            QMessageBox::warning(widget, "Input Error", "Password cannot be empty for non-model roles.");
            return;
        }

        bool result;
        if (role == "model") {
            result = databaseManager->addEmployee(username, QString(), role);  
        } else {
            result = databaseManager->addEmployee(username, password, role);  
        }
        

        if (result) {
            QMessageBox::information(widget, "Success", "User added successfully.");
            widget->close();
        } else {
            QMessageBox::critical(widget, "Error", "Failed to add user. Please try again.");
        }
    });
}

void DashboardWindow::showAllSessions()
{
    if (!isAdmin) {
        uiOperator->listWidget_2->clear();
        uiOperator->listWidget->clear();
    } else {
        uiAdmin->listWidget->clear();
        uiAdmin->listWidget_2->clear();
    }

    QSqlQuery sessionQuery = databaseManager->fetchAllSessions(isAdmin, employeeId);

    while (sessionQuery.next()) {
        int id = sessionQuery.value("session_id").toInt();
        QString employeeName = sessionQuery.value("employee_name").toString();
        QString clientName = sessionQuery.value("client_name").toString();
        QString startTime = sessionQuery.value("start_time").toString();
        QString endTime = sessionQuery.value("end_time").toString();
        QString totalHours = sessionQuery.value("total_hours").toString();
        QString status = sessionQuery.value("session_status").toString();

        QString buffer = QString("Employee: %1 | Client: %2 | Start: %3 | End: %4 | Hours: %5 | Status: %6")
                             .arg(employeeName)
                             .arg(clientName)
                             .arg(startTime)
                             .arg(endTime)
                             .arg(totalHours.isEmpty() ? "N/A" : totalHours)
                             .arg(status);

        QPixmap map(16, 16);
        QListWidget *targetListWidget = nullptr;

        if (isAdmin) {
            if (status == "Created") {
                map.fill(Qt::red);
                targetListWidget = uiAdmin->listWidget;
            } else if (status == "Completed") {
                map.fill(Qt::green);
                targetListWidget = uiAdmin->listWidget;
            } else {
                map.fill(Qt::yellow);
                targetListWidget = uiAdmin->listWidget;
            }
        } else {
            if (status == "Created") {
                map.fill(Qt::red);
                targetListWidget = uiOperator->listWidget;
            } else if (status == "Completed") {
                map.fill(Qt::green);
                targetListWidget = uiOperator->listWidget;
            } else {
                map.fill(Qt::yellow);
                targetListWidget = uiOperator->listWidget_2;
            }
        }

        QListWidgetItem *item = new QListWidgetItem(buffer);
        item->setIcon(QIcon(map));
        item->setData(Qt::UserRole, id);
        if (targetListWidget) {
            targetListWidget->addItem(item);
        }
    }

    if (isAdmin) {
        QSqlQuery reportQuery = databaseManager->fetchReports();
        while (reportQuery.next()) {
            int id = reportQuery.value("report_id").toInt();
            int sessionId = reportQuery.value("session_id").toInt();
            double earnings = reportQuery.value("total_earnings").toDouble();
            QString uploadedAt = reportQuery.value("uploaded_at").toString();
            QString employeeName = reportQuery.value("employee_name").toString();
            QString clientName = reportQuery.value("client_name").toString();

            QString reportBuffer = QString("Session: %1 | Earnings: %2 | Uploaded At: %3 | Employee: %4 | Client: %5")
                                    .arg(sessionId)
                                    .arg(earnings)
                                    .arg(uploadedAt)
                                    .arg(employeeName)
                                    .arg(clientName);

            QPixmap reportMap(16, 16);
            reportMap.fill(Qt::blue);

            QListWidgetItem *reportItem = new QListWidgetItem(reportBuffer);
            reportItem->setIcon(QIcon(reportMap));
            reportItem->setData(Qt::UserRole, id);
            uiAdmin->listWidget_2->addItem(reportItem);
        }
    }
}

#include <moc_DashboardWindow.cpp>
