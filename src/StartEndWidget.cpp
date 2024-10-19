#include "StartEndWidget.h"

StartEndWidget::StartEndWidget(QWidget* parent, int _selectedShiftId, LogicDataBase* _database, int _idUser) 
    : QWidget(parent), 
      db(_database),
      idUser(_idUser),
      selectedShiftId(_selectedShiftId), 
      gridLayoutForImages(new QGridLayout()),
      buttonDialogFile(new QPushButton("Select File", this)),
      buttonUpload(new QPushButton("Upload", this)),
      logText(new QTextEdit(this)),
      layout(new QVBoxLayout(this)),
      scrollArea(new QScrollArea(this)),
      timer(new QTimer(this)),
      information(new QLineEdit(this)),
      totalMoney(new QLineEdit(this))
{
    information->hide();
    totalMoney->hide();
    QObject::connect(timer, &QTimer::timeout, [this]() {
        QString text = "Сессия ";
        text += QString::number(selectedShiftId);
        text += " завершена";
        QMessageBox::information(this, "Timer", text);
        timer->stop();
    });

    this->setFocusPolicy(Qt::StrongFocus);
    QSqlQuery logs = db->getLogs(0, selectedShiftId);
    QString buffer;
    while (logs.next()) {
        buffer += logs.value(0).toString();  
        buffer += "\n";
    }
    logText->setReadOnly(true);
    logText->setText(buffer);
    

    QWidget* containerWidget = new QWidget(scrollArea);
    containerWidget->setLayout(gridLayoutForImages);
    scrollArea->setWidget(containerWidget);
    scrollArea->setWidgetResizable(true);

    layout->addWidget(buttonDialogFile);
    layout->addWidget(scrollArea);
    layout->addWidget(buttonUpload);
    layout->addWidget(logText);

    this->setLayout(layout);
    this->resize(600,400);

    QObject::connect(buttonDialogFile, &QPushButton::clicked, [this]()
    {
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
                        mapOfTheImages.insert(fileName, image);
                    }
                }
            }
            showImages();
        }
    });

    QObject::connect(buttonUpload, &QPushButton::clicked, [this]()
    {
        if (db->startShift(selectedShiftId))
        {
            for (const auto& m : mapOfTheImages) {
                db->uploadImageToDB(QPixmap::fromImage(m), selectedShiftId, true);  
            }

            mapOfTheImages.clear();  

            QMessageBox::information(this, "Success", "Shift started and images uploaded successfully.");
            
        }
        else
        {
            QMessageBox::critical(this, "Error", "Something went wrong while starting the shift.");
            return;
        }
        removeWidgetsFromLayout();
        this->hide(); 
        QSqlQuery query = db->getTimings(selectedShiftId);
        QTime start_time = query.value("start_time").toTime();
        QTime end_time = query.value("end_time").toTime();
        int temp = start_time.msecsTo(end_time);
        qDebug() << temp;
        if (start_time < end_time){
            timer->start(temp);
        }
        else{
            
        }
        

    });


}

void StartEndWidget::endShift(int operatorId)
{
    bool stoppedManually = false; 
    layout->addWidget(totalMoney);   
    layout->addWidget(information);  
    information->show();
    totalMoney->show();
    totalMoney->setPlaceholderText("Введите общую сумму заработка");
    this->show();  

    if (timer->isActive()) {
        timer->stop();
        stoppedManually = true;
    }

    disconnect(buttonUpload, nullptr, nullptr, nullptr);

    QObject::connect(buttonUpload, &QPushButton::clicked, [this, operatorId, stoppedManually]()
    {
        QString reportText = information->text();
        QString totalEarnings = totalMoney->text();  
        QTime startTime = db->getTime(operatorId);     
        
        if (db->createReport(selectedShiftId, operatorId, reportText, totalEarnings, stoppedManually, startTime))
        {
            int reportId = db->getReportId(selectedShiftId);
            if (reportId == -1) {
                QMessageBox::critical(this, "Error", "Failed to retrieve a valid report ID.");
                return;
            }
            for (const auto& m : mapOfTheImages) {
                if (m.isNull()) {
                    continue;
                }
                db->uploadImageToDB(QPixmap::fromImage(m), reportId, false);  
            }

            mapOfTheImages.clear();  

            QMessageBox::information(this, "Success", "Shift ended and images uploaded successfully.");
            close();
        }
        else
        {
            QMessageBox::critical(this, "Error", "Something went wrong while ending the shift.");
            return;
        }
    });
}


StartEndWidget::~StartEndWidget()
{

}

void StartEndWidget::showImages()
{
    QLayoutItem *child;
    while ((child = gridLayoutForImages->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }

    int row = 0;
    int col = 0;
    foreach (const QImage &image, mapOfTheImages) {
        if (!image.isNull()) {
            QLabel *imageLabel = new QLabel(this);
            imageLabel->setPixmap(QPixmap::fromImage(image).scaled(100, 100, Qt::KeepAspectRatio));
            gridLayoutForImages->addWidget(imageLabel, row, col);

            col++;
            if (col >= 3) {
                col = 0;
                row++;
            }
        }
    }
}

void StartEndWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->matches(QKeySequence::Paste)) {
        QClipboard *clipboard = QApplication::clipboard();
        const QMimeData *mimeData = clipboard->mimeData();

        if (mimeData->hasImage()) {
            QPixmap pixmap = qvariant_cast<QPixmap>(mimeData->imageData());
            QImage image = pixmap.toImage();
            QString key = QString::number(QRandomGenerator::global()->bounded(0, 10000));
            mapOfTheImages.insert(key, image);
            showImages();
        }
    }
}

void StartEndWidget::removeWidgetsFromLayout()
{
    QLayoutItem* item;
    while ((item = gridLayoutForImages->takeAt(0)) != nullptr)
    {
        if (item->widget())
        {
            QWidget* widget = item->widget();
            gridLayoutForImages->removeWidget(widget);
            widget->deleteLater();  
        }
        delete item;
    }
}

#include <moc_StartEndWidget.cpp>
