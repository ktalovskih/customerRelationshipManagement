#include "SessionDetailWidget.h"

SessionDetailWidget::SessionDetailWidget(QWidget* parent, int _selectedSessionId, CRMDatabaseManager* _database, int _employeeId) 
    : QWidget(parent),
      databaseManager(_database),
      employeeId(_employeeId),
      selectedSessionId(_selectedSessionId), 
      gridLayoutForImages(new QGridLayout()),
      buttonDialogFile(new QPushButton("Select File", this)),
      buttonUpload(new QPushButton("Upload", this)),
      logText(new QTextEdit(this)),
      layout(new QVBoxLayout(this)),
      scrollArea(new QScrollArea(this)),
      timer(new QTimer(this)),
      information(new QLineEdit(this)),
      totalEarnings(new QLineEdit(this))
{
    information->hide();
    totalEarnings->hide();
    
    QObject::connect(timer, &QTimer::timeout, [this]() {
        QString text = "Session ";
        text += QString::number(selectedSessionId);
        text += " completed";
        QMessageBox::information(this, "Timer", text);
        timer->stop();
    });
    
    this->setFocusPolicy(Qt::StrongFocus);
    QSqlQuery logsQuery = databaseManager->fetchSessionNotes(0, selectedSessionId);
    QString buffer;
    while (logsQuery.next()) {
        buffer += logsQuery.value(0).toString();  
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
                        imageMap.insert(fileName, image);
                    }
                }
            }
            showImages();
        }
    });
    
    QObject::connect(buttonUpload, &QPushButton::clicked, [this]() {
        // Replace startShift with startSession
        if (databaseManager->startSession(selectedSessionId)) {
            // Replace uploadImageToDB with uploadDocumentToDB
            for (const auto& m : imageMap) {
                databaseManager->uploadDocumentToDB(QPixmap::fromImage(m), selectedSessionId, true);  
            }
            imageMap.clear();
            QMessageBox::information(this, "Success", "Session started and images uploaded successfully.");
        } else {
            QMessageBox::critical(this, "Error", "Something went wrong while starting the session.");
            return;
        }
        removeWidgetsFromLayout();
        this->hide();
        // Replace getTimings with fetchSessionTimings
        QSqlQuery query = databaseManager->fetchSessionTimings(selectedSessionId);
        QTime start_time = query.value("start_time").toTime();
        QTime end_time = query.value("end_time").toTime();
        int temp = start_time.msecsTo(end_time);
        qDebug() << temp;
        if (start_time < end_time) {
            timer->start(temp);
        }
    });
}

void SessionDetailWidget::endSession(int employeeId)
{
    bool stoppedManually = false; 
    layout->addWidget(totalEarnings);   
    layout->addWidget(information);  
    information->show();
    totalEarnings->show();
    totalEarnings->setPlaceholderText("Enter total earnings");
    this->show();  

    if (timer->isActive()) {
        timer->stop();
        stoppedManually = true;
    }
    
    disconnect(buttonUpload, nullptr, nullptr, nullptr);
    
    QObject::connect(buttonUpload, &QPushButton::clicked, [this, employeeId, stoppedManually]() {
        QString reportText = information->text();
        QString totalEarningsText = totalEarnings->text();  
        // Replace getTime with getSessionStartTime and pass selectedSessionId
        QTime startTime = databaseManager->getSessionStartTime(selectedSessionId);     
        
        if (databaseManager->createReport(selectedSessionId, employeeId, reportText, totalEarningsText, stoppedManually, startTime)) {
            // Replace getReportId with getReportIdForSession
            int reportId = databaseManager->getReportIdForSession(selectedSessionId);
            if (reportId == -1) {
                QMessageBox::critical(this, "Error", "Failed to retrieve a valid report ID.");
                return;
            }
            for (const auto& m : imageMap) {
                if (m.isNull()) {
                    continue;
                }
                // Replace uploadImageToDB with uploadDocumentToDB
                databaseManager->uploadDocumentToDB(QPixmap::fromImage(m), reportId, false);  
            }
            imageMap.clear();
            QMessageBox::information(this, "Success", "Session ended and images uploaded successfully.");
            close();
        } else {
            QMessageBox::critical(this, "Error", "Something went wrong while ending the session.");
            return;
        }
    });
}

SessionDetailWidget::~SessionDetailWidget()
{
}

void SessionDetailWidget::showImages()
{
    QLayoutItem* child;
    while ((child = gridLayoutForImages->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }
    int row = 0;
    int col = 0;
    foreach (const QImage &image, imageMap) {
        if (!image.isNull()) {
            QLabel* imageLabel = new QLabel(this);
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

void SessionDetailWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->matches(QKeySequence::Paste)) {
        QClipboard* clipboard = QApplication::clipboard();
        const QMimeData* mimeData = clipboard->mimeData();
        if (mimeData->hasImage()) {
            QPixmap pixmap = qvariant_cast<QPixmap>(mimeData->imageData());
            QImage image = pixmap.toImage();
            QString key = QString::number(QRandomGenerator::global()->bounded(0, 10000));
            imageMap.insert(key, image);
            showImages();
        }
    }
}

void SessionDetailWidget::removeWidgetsFromLayout()
{
    QLayoutItem* item;
    while ((item = gridLayoutForImages->takeAt(0)) != nullptr)
    {
        if (item->widget()) {
            QWidget* widget = item->widget();
            gridLayoutForImages->removeWidget(widget);
            widget->deleteLater();  
        }
        delete item;
    }
}

#include <moc_SessionDetailWidget.cpp>
