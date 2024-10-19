#include "MainWindow.h"
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
#include <set>
#include <QTextEdit>

MainWindow::MainWindow(QWidget *parent, LogicDataBase *database, bool isAdmin, int idUser)
    : QMainWindow(parent), db(database), isAdmin(isAdmin), idUser(idUser), updateTimer(new QTimer())
{
    this->setFocusPolicy(Qt::StrongFocus);
    updateTimer->start(5000);
    QObject::connect(updateTimer, &QTimer::timeout, [this](){
        showAllShifts();
        updateTimer->start(5000);
    });
    if (isAdmin)
    {
        uiAdmin = std::make_unique<Ui::mainWindowAdmin>();
        uiAdmin->setupUi(this);
        QObject::connect(uiAdmin->pushButton, &QPushButton::pressed, this, &MainWindow::addTask);

        QObject::connect(uiAdmin->listWidget, &QListWidget::itemClicked, this, &MainWindow::onShiftSelected);
        QObject::connect(uiAdmin->listWidget_2, &QListWidget::itemClicked, this, &MainWindow::onShiftSelected);

        QObject::connect(uiAdmin->listWidget, &QListWidget::doubleClicked, this, &MainWindow::showWholeInformation);
        QObject::connect(uiAdmin->listWidget_2, &QListWidget::doubleClicked, this, &MainWindow::showWholeInformation);

        QObject::connect(uiAdmin->pushButton_2, &QPushButton::clicked, this, &MainWindow::showAllShifts);
        QObject::connect(uiAdmin->pushButton_3, &QPushButton::clicked,this, &MainWindow::addNewUser);
        QObject::connect(uiAdmin->pushButton_4, &QPushButton::clicked, [this](){
            db->deleteCompletedShiftsAndReorder();
            showAllShifts();
        });

        
    }
    else
    {
        uiOperator = std::make_unique<Ui::mainWindowOperator>();
        uiOperator->setupUi(this);
        QObject::connect(uiOperator->pushButton_3, &QPushButton::pressed, this, &MainWindow::showAllShifts);


        QObject::connect(uiOperator->listWidget, &QListWidget::itemClicked, this, &MainWindow::onShiftSelected);
        QObject::connect(uiOperator->listWidget_2, &QListWidget::itemClicked, this, &MainWindow::onShiftSelected);
        
        QObject::connect(uiOperator->pushButton, &QPushButton::pressed, [this]() {
            auto* startEndWidget = new StartEndWidget(nullptr,selectedShiftId, this->db);
            startEndWidgets.insert(std::make_pair(selectedShiftId, startEndWidget));
            startEndWidget->show();
            this->showAllShifts();
            QObject::connect(startEndWidget, &QWidget::isVisible, [this](){
                this->showAllShifts();
            });
        });
        
        QObject::connect(uiOperator->pushButton_2, &QPushButton::pressed, this, [this](){
            auto temp = startEndWidgets.find(selectedShiftId);
            auto* widget = temp->second;
            widget->endShift(temp->first);
            QObject::connect(widget, &QWidget::destroyed, [this,temp](){
                startEndWidgets.erase(temp->first);
            });
            showAllShifts(); 
        });

        QObject::connect(uiOperator->listWidget_2, &QListWidget::doubleClicked, this, &MainWindow::showWholeInformation);
        QObject::connect(uiOperator->listWidget, &QListWidget::doubleClicked, this, &MainWindow::showWholeInformation);

        showAllShifts();

    }
    this->resize(1320,700);
}

MainWindow::~MainWindow()
{
    delete db;
}

void MainWindow::onShiftSelected(QListWidgetItem *item)
{
    selectedShiftId = item->data(Qt::UserRole).toInt();
}

void MainWindow::editShift()
{
    createShiftForm(selectedShiftId);
}

void MainWindow::addTask()
{
    createShiftForm(-1);
}

void MainWindow::createShiftForm(int indexOfShift)
{
    QSqlQueryModel *model1 = db->getModels();
    QSqlQueryModel *model2 = db->getOperators();

    QWidget *widget = new QWidget;
    QGridLayout *layout = new QGridLayout;

    auto *startTime = new QTimeEdit();
    auto *endTime = new QTimeEdit();
    startTime->setTime(QTime::currentTime());
    endTime->setTime(QTime::currentTime());

    startTime->setDisplayFormat("HH:mm");
    endTime->setDisplayFormat("HH:mm");
    auto *logs = new QTextEdit();

    auto button = new QPushButton(indexOfShift == -1 ? "Create Shift" : "Edit Shift");

    QComboBox *comboBoxModel = new QComboBox;
    QComboBox *comboBoxOperator = new QComboBox;

    comboBoxModel->setModel(model1);
    comboBoxModel->setModelColumn(0);

    comboBoxOperator->setModel(model2);
    comboBoxOperator->setModelColumn(0);

    QCompleter *completer1 = new QCompleter(model1, this);
    completer1->setCompletionColumn(0);
    completer1->setCompletionMode(QCompleter::PopupCompletion);
    completer1->setCaseSensitivity(Qt::CaseInsensitive);

    comboBoxModel->setCompleter(completer1);

    QCompleter *completer2 = new QCompleter(model2, this);
    completer2->setCompletionColumn(0);
    completer2->setCompletionMode(QCompleter::PopupCompletion);
    completer2->setCaseSensitivity(Qt::CaseInsensitive);

    comboBoxOperator->setCompleter(completer2);

    comboBoxModel->setEditable(true);
    comboBoxOperator->setEditable(true);

    comboBoxOperator->setPlaceholderText("Select Operator");
    comboBoxModel->setPlaceholderText("Select Model");

    startTime->setKeyboardTracking(true);
    endTime->setKeyboardTracking(true);

    if (indexOfShift != -1)
    {
        QSqlRecord shiftRecord = db->getShiftById(indexOfShift);
        if (!shiftRecord.isEmpty()) {
            int operatorId = shiftRecord.value("operator_id").toInt();
            int modelId = shiftRecord.value("model_id").toInt();
            QTime start = QTime::fromString(shiftRecord.value("start_time").toString(), "HH:mm");
            QTime end = QTime::fromString(shiftRecord.value("end_time").toString(), "HH:mm");
            QString logsText = shiftRecord.value("logs").toString();

            QString operatorName = db->getOperatorNameById(operatorId);
            QString modelName = db->getModelNameById(modelId);

            comboBoxOperator->setCurrentText(operatorName);
            comboBoxModel->setCurrentText(modelName);
            startTime->setTime(start);
            endTime->setTime(end);
            logs->setText(logsText);
        }
    }

    auto *deleteButton = new QPushButton("delete");
    widget->setLayout(layout);
    layout->addWidget(comboBoxOperator,0 , 0);
    layout->addWidget(comboBoxModel, 0 ,1);
    layout->addWidget(startTime, 1, 0);
    layout->addWidget(endTime, 1 ,1);
    layout->addWidget(logs,2,0,2,3);
    layout->setRowStretch(3, 1);
    layout->addWidget(button,4, 0, 1, 2);
    if (indexOfShift != -1)
         layout->addWidget(deleteButton, 5, 0, 1, 2);
    widget->show();

    QObject::connect(deleteButton, &QPushButton::clicked, [=]()
                     {
        db->deleteShift(indexOfShift);
        uiAdmin->listWidget->clear();
        showAllShifts();
        widget->deleteLater(); 
    });

    QObject::connect(button, &QPushButton::pressed, [=]()
                     {
        QString operatorName = comboBoxOperator->currentText();
        QString modelName = comboBoxModel->currentText();
        QTime start = startTime->time();
        QTime end = endTime->time();
        QString logsText = logs->toPlainText();
        if (indexOfShift == -1) {
            db->addShift(operatorName, modelName, start, end, logsText);
            QMessageBox::information(widget, "Shift Created", "Shift has been successfully created.");
        } else {
            db->updateShift(operatorName, modelName, start, end, logsText, indexOfShift);
        }
        uiAdmin->listWidget->clear();
        showAllShifts();
        widget->deleteLater(); 
    });
}

void MainWindow::showReport()
{
    
}

void MainWindow::showWholeInformation()
{
    auto* editButton = new QPushButton("Редактировать");
    QObject::connect(editButton, &QPushButton::clicked, this, &MainWindow::editShift);
    auto *widget = new QWidget;
    auto *layout = new QVBoxLayout(widget);

    QString status = db->getStatus(selectedShiftId);
    qDebug() << status;

    if (status == "В работе" || status == "Завершен" || status == "в работе") {
        
        auto *widgetForImages = new QWidget(widget);
        QListWidget *listWidget = new QListWidget(widgetForImages);

        QSqlQuery query = db->getShiftImages(selectedShiftId);

        while (query.next()) {
            QListWidgetItem *item = new QListWidgetItem;
            QByteArray imageData = query.value("screenshot").toByteArray();
            QPixmap map;

            if (!map.loadFromData(imageData)) {
                continue;
            }

            item->setIcon(QIcon(map));
            item->setData(Qt::UserRole, map); 

            listWidget->addItem(item);
        }
        if (status == "Завершен"){
            QSqlQuery query = db->getReportImages(selectedShiftId);
            while (query.next()) {
                QListWidgetItem *item = new QListWidgetItem;
                QByteArray imageData = query.value("screenshot").toByteArray();
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
            QLabel* label= new QLabel();
            label->setPixmap(pixmap);
            label->show();
        });
    }

    auto *text = new QTextEdit(widget);
    text->setReadOnly(true);

    QSqlQuery logQuery = db->getLogs(isAdmin, selectedShiftId);
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
    layout->addWidget(editButton);

    widget->show();
}

void MainWindow::addNewUser()
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
    comboBoxRoles->addItems({"admin", "operator", "model"});  

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
            result = db->addUser(username, QString(), role);  
        } else {
            result = db->addUser(username, password, role);  
        }

        if (result) {
            QMessageBox::information(widget, "Success", "User added successfully.");
            widget->close();
        } else {
            QMessageBox::critical(widget, "Error", "Failed to add user. Please try again.");
        }
    });
}

void MainWindow::showAllShifts()
{
    if (!isAdmin) {
        uiOperator->listWidget_2->clear();
        uiOperator->listWidget->clear();
    } else {
        uiAdmin->listWidget->clear();
        uiAdmin->listWidget_2->clear();
    }

    QSqlQuery shiftQuery = db->getAllShifts(isAdmin, idUser);

    while (shiftQuery.next()) {
        int id = shiftQuery.value("id").toInt();
        QString operatorName = shiftQuery.value("operator_name").toString();
        QString modelName = shiftQuery.value("model_name").toString();
        QString startTime = shiftQuery.value("start_time").toString();
        QString endTime = shiftQuery.value("end_time").toString();
        QString totalHours = shiftQuery.value("total_hours").toString();
        QString status = shiftQuery.value("status").toString();

        QString buffer = QString("Оператор: %1 | Модель: %2 | Начало: %3 | Конец: %4 | Кл-во Часов: %5 | Статус: %6")
                             .arg(operatorName)
                             .arg(modelName)
                             .arg(startTime)
                             .arg(endTime)
                             .arg(totalHours.isEmpty() ? "N/A" : totalHours)
                             .arg(status);

        QPixmap map(16, 16);
        QListWidget *targetListWidget = nullptr;

        if (isAdmin) {
            if (status == "Создан") {
                map.fill(Qt::red);
                targetListWidget = uiAdmin->listWidget;
            } else if (status == "Завершен") {
                map.fill(Qt::green);
                targetListWidget = uiAdmin->listWidget;
            } else {
                map.fill(Qt::yellow);
                targetListWidget = uiAdmin->listWidget;
            }
        } else {
            if (status == "Создан") {
                map.fill(Qt::red);
                targetListWidget = uiOperator->listWidget;
            } else if (status == "Завершен") {
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
        QSqlQuery reportQuery = db->getReports();  

        while (reportQuery.next()) {
            int id = reportQuery.value("id").toInt();
            int shiftId = reportQuery.value("shift_id").toInt();
            double earnings = reportQuery.value("total_earnings").toDouble();
            QString uploadedAt = reportQuery.value("uploaded_at").toString();
            QString operatorName = reportQuery.value("operator_name").toString();  
            QString modelName = reportQuery.value("model_name").toString();  

            QString reportBuffer = QString("Сессия: %1 | Общая сумма: %2 | Загруженно В: %3 | Оператор: %4 | Модель: %5")
                                    .arg(shiftId)
                                    .arg(earnings)
                                    .arg(uploadedAt)
                                    .arg(operatorName)
                                    .arg(modelName);

            QPixmap reportMap(16, 16);
            reportMap.fill(Qt::blue);

            QListWidgetItem *reportItem = new QListWidgetItem(reportBuffer);
            reportItem->setIcon(QIcon(reportMap));
            reportItem->setData(Qt::UserRole, id);
            uiAdmin->listWidget_2->addItem(reportItem);
        }

    }
}

#include <moc_MainWindow.cpp>