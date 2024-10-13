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
#include <set>
MainWindow::MainWindow(QWidget *parent, LogicDataBase *database, bool isAdmin, int idUser)
    : QMainWindow(parent), db(database), isAdmin(isAdmin), idUser(idUser), labelForImage(new QLabel())
{
    if (isAdmin)
    {
        uiAdmin = std::make_unique<Ui::mainWindowAdmin>();
        uiAdmin->setupUi(this);
        QObject::connect(uiAdmin->listWidget, &QListWidget::doubleClicked, this, &MainWindow::editShift);
        QObject::connect(uiAdmin->pushButton, &QPushButton::pressed, this, &MainWindow::addTask);
        showAllShifts();
    }
    else
    {
        uiOperator = std::make_unique<Ui::mainWindowOperator>();
        uiOperator->setupUi(this);

        QObject::connect(uiOperator->listWidget, &QListWidget::itemClicked, this, &MainWindow::onShiftSelected);
        QObject::connect(uiOperator->listWidget_2, &QListWidget::itemClicked, this, &MainWindow::onShiftSelected);

        QObject::connect(uiOperator->pushButton, &QPushButton::pressed, this, &MainWindow::startShift);
        QObject::connect(uiOperator->pushButton_2, &QPushButton::pressed, this, &MainWindow::endShift);
        QObject::connect(uiOperator->listWidget_2, &QListWidget::doubleClicked, this, &MainWindow::showWholeInformation);
        QObject::connect(uiOperator->listWidget, &QListWidget::doubleClicked, this, &MainWindow::showWholeInformation);

        showAllShifts();
    }
}

MainWindow::~MainWindow()
{
    delete db;
};
void MainWindow::onShiftSelected(QListWidgetItem *item)
{
    selectedShiftId = item->data(Qt::UserRole).toInt();
    qDebug() << "Selected Shift ID:" << selectedShiftId;
}
void MainWindow::editShift()
{
    QListWidgetItem *selectedItem = uiAdmin->listWidget->currentItem();
    if (!selectedItem)
    {
        QMessageBox::warning(this, "No Selection", "Please select a shift to edit.");
        return;
    }

    int shiftId = selectedItem->data(Qt::UserRole).toInt();

    createShiftForm(shiftId);
}

void MainWindow::addTask()
{
    createShiftForm(-1);
}

void MainWindow::createShiftForm(int indexOfShift)
{
    QSqlQueryModel *model1 = new QSqlQueryModel(this);
    QSqlQueryModel *model2 = new QSqlQueryModel(this);

    QWidget *widget = new QWidget;
    QHBoxLayout *layout = new QHBoxLayout;

    auto *startTime = new QTimeEdit();
    auto *endTime = new QTimeEdit();
    startTime->setTime(QTime::currentTime());
    endTime->setTime(QTime::currentTime());

    startTime->setDisplayFormat("HH:mm");
    endTime->setDisplayFormat("HH:mm");
    auto *logs = new QLineEdit();

    auto button = new QPushButton(indexOfShift == -1 ? "Create Shift" : "Edit Shift");

    model1->setQuery("SELECT name FROM models");
    model2->setQuery("SELECT name FROM users WHERE role = 'operator'");

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
        QSqlQuery query;
        query.prepare("SELECT operator_id, model_id, start_time, end_time FROM shifts WHERE id = :id");
        query.bindValue(":id", indexOfShift);
        query.exec();

        if (query.next())
        {
            int operatorId = query.value(0).toInt();
            int modelId = query.value(1).toInt();
            QTime start = QTime::fromString(query.value(2).toString(), "HH:mm");
            QTime end = QTime::fromString(query.value(3).toString(), "HH:mm");
            QString logsText = query.value(8).toString();
            QSqlQuery operatorQuery;
            operatorQuery.prepare("SELECT name FROM users WHERE id = :id");
            operatorQuery.bindValue(":id", operatorId);
            operatorQuery.exec();
            QString operatorName;
            if (operatorQuery.next())
            {
                operatorName = operatorQuery.value(0).toString();
            }

            QSqlQuery modelQuery;
            modelQuery.prepare("SELECT name FROM models WHERE id = :id");
            modelQuery.bindValue(":id", modelId);
            modelQuery.exec();
            QString modelName;
            if (modelQuery.next())
            {
                modelName = modelQuery.value(0).toString();
            }

            comboBoxOperator->setCurrentText(operatorName);
            comboBoxModel->setCurrentText(modelName);
            startTime->setTime(start);
            endTime->setTime(end);
            logs->setText(logsText);
        }
    }

    auto *deleteButton = new QPushButton("delete");
    widget->setLayout(layout);
    layout->addWidget(comboBoxOperator);
    layout->addWidget(comboBoxModel);
    layout->addWidget(startTime);
    layout->addWidget(endTime);
    layout->addWidget(logs);
    layout->addWidget(button);
    if (indexOfShift != -1)
        layout->addWidget(deleteButton);
    widget->show();

    QObject::connect(deleteButton, &QPushButton::clicked, [=]()
                     {
        db->deleteShift(indexOfShift);
        uiAdmin->listWidget->clear();
        showAllShifts();
        widget->deleteLater(); });

    QObject::connect(button, &QPushButton::pressed, [=](){
        QString operatorName = comboBoxOperator->currentText();
        QString modelName = comboBoxModel->currentText();
        QTime start = startTime->time();
        QTime end = endTime->time();
        QString logsText = logs->text();
        if (indexOfShift == -1) {
            qDebug() << indexOfShift;
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

void MainWindow::keyPressEvent(QKeyEvent *event) //not working
{
    if (event->matches(QKeySequence::Paste)) {
        QClipboard *clipboard = QApplication::clipboard();
        const QMimeData *mimeData = clipboard->mimeData();

        if (mimeData->hasImage()) {
            QPixmap image = qvariant_cast<QPixmap>(mimeData->imageData());
            labelForImage->setPixmap(image);
            labelForImage->setScaledContents(true);
            db->uploadImageToDB(image);
        } else if (mimeData->hasUrls()) {
            QList<QUrl> urls = mimeData->urls();
            foreach (const QUrl &url, urls) {
                QString fileName = url.toLocalFile();
                QPixmap image(fileName);
                if (!image.isNull()) {
                    labelForImage->setPixmap(image);
                    labelForImage->setScaledContents(true);
                    db->uploadImageToDB(image);
                }
            }
        }
    }
}

void MainWindow::endShift() 
{
    
    auto *label = new QLabel();

    QSqlQuery query;
    query.prepare("SELECT screenshot FROM shift_screenshots WHERE id = :id");
    query.bindValue(":id", 1);

    if (query.exec() && query.next())
    {
        QByteArray byteArray = query.value(0).toByteArray(); // Get the BLOB
        QPixmap pixmap;
        pixmap.loadFromData(byteArray); // Convert BLOB back to QPixmap

        if (!pixmap.isNull())
        {
            label->setPixmap(pixmap.scaled(label->size(), Qt::KeepAspectRatio)); // Display the image
        }
        else
        {
            qDebug() << "Failed to load image from DB";
        }
    }
    label->show();
}

void MainWindow::showWholeInformation()
{
    auto *widget = new QWidget;
    auto *layout = new QVBoxLayout(widget);
    
    auto *text = new QTextEdit(widget);
    text->setReadOnly(true);  

    QSqlQuery query;
    query.prepare("SELECT logs FROM shifts WHERE operator_id = :id and id = :id2");  
    query.bindValue(":id", idUser);  
    query.bindValue(":id2", selectedShiftId);  
    if (!query.exec())
        qDebug() << query.lastError();
    QString buffer;
    while (query.next())
    {
        buffer += query.value(0).toString();  
        buffer += "\n";  
    }

    text->setText(buffer);

    layout->addWidget(text);
    widget->setLayout(layout);

    widget->setMinimumSize(600, 400);  

    widget->setAttribute(Qt::WA_DeleteOnClose);

    widget->show();
}

void MainWindow::startShift()
{
    
    QSqlQuery query;
    query.prepare("SELECT status FROM shifts WHERE id = :id");
    query.bindValue(":id", selectedShiftId);

    if (query.exec() && query.next()) {  
        if (query.value(0).toString() == "в работе") {  
            return;  
        }
    }

    auto* widget = new QWidget();
    
    auto* buttonDialogFile = new QPushButton("Select File");
    labelForImage = new QLabel();
    auto* layout = new QVBoxLayout(widget);
    auto* buttonUpload = new QPushButton("Upload");

    auto* logText = new QTextEdit();
    logText->setReadOnly(true);
    QSqlQuery logQuery;
    logQuery.prepare("SELECT logs FROM shifts WHERE id = :id");
    logQuery.bindValue(":id", selectedShiftId);
    if (logQuery.exec()) {
        QString buffer;
        while (logQuery.next()) {
            buffer += logQuery.value(0).toString();
            buffer += "\n";
        }
        logText->setText(buffer);
    } else {
        qDebug() << "Failed to retrieve logs for shift ID:" << selectedShiftId;
    }

    QObject::connect(buttonDialogFile, &QPushButton::clicked, [this]() {
    QFileDialog dialog;
    
    dialog.setFileMode(QFileDialog::ExistingFiles);
    
    dialog.setNameFilter("Images (*.png *.jpg *.jpeg);;All files (*.*)");
    
    dialog.setDirectory(QDir::currentPath());

    if (dialog.exec()) {
        QStringList fileNames = dialog.selectedFiles();  
        
        foreach (const QString &fileName, fileNames) {
            if (!fileName.isEmpty()) {
                QImage image(fileName);  

                if (!image.isNull()) {
                    labelForImage->setPixmap(QPixmap::fromImage(image));
                    labelForImage->setScaledContents(true);

                    mapOfTheImages.insert(fileName, image);
                } else {
                    qDebug() << "Failed to load image from file:" << fileName;
                }
            }
        }
    }
});


    QObject::connect(buttonUpload, &QPushButton::clicked, [this, widget]() {
        QSqlQuery query;
        query.prepare("UPDATE shifts SET start_time = :start_time, status = 'в работе' WHERE id = :id");
        query.bindValue(":id", selectedShiftId);
        query.bindValue(":start_time", QTime::currentTime().toString("HH:mm"));
        if (!query.exec()) {
            QMessageBox::critical(this, "Error", "Something went wrong while starting the shift.");
            return;
        }

        for (const auto& m : mapOfTheImages) {
            db->uploadImageToDB(QPixmap::fromImage(m));
        }
        
        widget->deleteLater();  
        mapOfTheImages.clear();
        showAllShifts();  
    });

    layout->addWidget(logText);
    layout->addWidget(labelForImage);
    layout->addWidget(buttonDialogFile);
    layout->addWidget(buttonUpload);

    connect(widget, &QWidget::destroyed, [this]() {
        mapOfTheImages.clear();  
    });

    widget->setLayout(layout);
    widget->setMinimumSize(600, 400);
    widget->setWindowTitle("Shift Logs and Image Upload");
    widget->show();
}


void MainWindow::showAllShifts()
{
    QSqlQuery query;

    if (!isAdmin)
    {
        query.prepare(R"(
            SELECT shifts.id, users.name AS operator_name, models.name AS model_name, shifts.start_time, shifts.end_time, shifts.total_hours, shifts.status
            FROM shifts
            JOIN users ON shifts.operator_id = users.id
            JOIN models ON shifts.model_id = models.id
            WHERE shifts.operator_id = :idUser
        )");
        query.bindValue(":idUser", idUser);
    }
    else
    {
        query.prepare(R"(
            SELECT shifts.id, users.name AS operator_name, models.name AS model_name, shifts.start_time, shifts.end_time, shifts.total_hours, shifts.status
            FROM shifts
            JOIN users ON shifts.operator_id = users.id
            JOIN models ON shifts.model_id = models.id
            )");
    }

    if (!query.exec())
    {
        qDebug() << "Query execution error: " << query.lastError().text();
        return;
    }
    if (!isAdmin)
    {
        uiOperator->listWidget_2->clear();
        uiOperator->listWidget->clear();
    }
    else
    {
        uiAdmin->listWidget->clear();
    }
    while (query.next())
    {
        int id = query.value("id").toInt();
        QString operatorName = query.value("operator_name").toString();
        QString modelName = query.value("model_name").toString();
        QString startTime = query.value("start_time").toString();
        QString endTime = query.value("end_time").toString();
        QString totalHours = query.value("total_hours").toString();
        QString status = query.value("status").toString();

        QString buffer = QString("Operator: %1 | Model: %2 | Start: %3 | End: %4 | Hours: %5 | Status: %6")
                             .arg(operatorName)
                             .arg(modelName)
                             .arg(startTime)
                             .arg(endTime)
                             .arg(totalHours.isEmpty() ? "N/A" : totalHours)
                             .arg(status);

        qDebug() << "Adding item: " << id;
        QListWidgetItem *item = new QListWidgetItem(buffer);
        item->setData(Qt::UserRole, id);

        if (!isAdmin)
        {
            if (status == "в работе")
            {
                uiOperator->listWidget->addItem(item);
            }
            else
            {
                uiOperator->listWidget_2->addItem(item);
            }
        }
        else
        {
            uiAdmin->listWidget->addItem(item);
        }
    }
}

#include <moc_MainWindow.cpp>