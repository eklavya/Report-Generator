#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setCentralWidget(ui->frame);
    ui->frame->setLayout(ui->horizontalLayout);
    dummy = new QWidget(this);
    dummy->setLayout(ui->gridLayout);
    ui->scrollArea->setWidget(dummy);
    ui->imageWidth->setRange(100, 1000);
    ui->imageHeight->setRange(50, 1000);
    ui->imageWidth->setValue(840);
    ui->imageHeight->setValue(480);
    ui->qualitySlider->setValue(95);
    ui->qualitySpinBox->setValue(95);
    this->current = NULL;
    imageDir = new QDir();

    QObject::connect(ui->imageEdit, SIGNAL(textChanged()), this, SLOT(descriptionChanged()));
    QObject::connect(ui->includeCheck, SIGNAL(toggled(bool)), this, SLOT(includeCheckChanged(bool)));
    QObject::connect(ui->reportButton, SIGNAL(clicked()), this, SLOT(generateButtonClicked()));
    QObject::connect(ui->openAction, SIGNAL(triggered()), this, SLOT(openActionTriggered()));
    QObject::connect(ui->previewPdfAction, SIGNAL(triggered()), this, SLOT(previewPdfActionTriggered()));
    QObject::connect(ui->previewHtmlAction, SIGNAL(triggered()), this, SLOT(previewHtmlActionTriggered()));
    QObject::connect(ui->previewOdtAction, SIGNAL(triggered()), this, SLOT(previewOdtActionTriggered()));
    QObject::connect(ui->exitAction, SIGNAL(triggered()), this, SLOT(exitActionTriggered()));
    QObject::connect(ui->aboutAction, SIGNAL(triggered()), this, SLOT(aboutActionTriggered()));
    QObject::connect(ui->qualitySlider, SIGNAL(valueChanged(int)), this, SLOT(qualitySliderChanged(int)));
    QObject::connect(ui->qualitySpinBox, SIGNAL(valueChanged(int)), this, SLOT(qualitySpinBoxChanged(int)));
    this->show();
    importImages();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::importImages() {
    QString dirPath = QFileDialog::getExistingDirectory(this, tr("Open Directory where images are stored"),
                                                        "/home", QFileDialog::DontResolveSymlinks);
    imageDir->setPath(dirPath);
    QStringList filters;
    filters << "*.jpg" << "*.jpeg";
    imageDir->setNameFilters(filters);
    QStringList imageList = imageDir->entryList();
    QProgressDialog progressDialog("Importing images...", "Abort", 0, imageList.size(), this);
    progressDialog.setAutoClose(true);

    int num = 0;
    for(int i = 0; i < imageList.size(); i++) {
        for(int j = 0; j < 3; j++) {
            if(num >= imageList.size()) break;
            //setting parent to dummy is necessary as we are deleting items from
            //gridlayout by deleting dummy which takes care of all the childerns too
            Label *imageLabel     = new Label(dummy);
            progressDialog.setValue(num);
            imageLabel->imagePath = dirPath + "/" + imageList.at(num);
            QImage *image         = new QImage(dirPath + "/" + imageList.at(num++));
            if(image->isNull()) break;
            imageLabel->setPixmap(QPixmap::fromImage(image->scaled(200, 150 , Qt::KeepAspectRatio)));
            image->~QImage();
            ui->gridLayout->addWidget(imageLabel, i, j);
        }
        i++;
    }
    progressDialog.setValue(num);
}

void MainWindow::imageSelected(Label *label)
{
    //set surrent preview label
    current = label;
    ui->includeCheck->setChecked(current->include);
    ui->previewLabel->setPixmap(QPixmap(*(label->pixmap())));
    ui->imageEdit->setHtml(label->description);
}

void MainWindow::includeCheckChanged(bool ignore)
{
    if(current != NULL) {
        current->include = ui->includeCheck->isChecked();
    }
}

void MainWindow::descriptionChanged()
{
    current->description = ui->imageEdit->toHtml();
}

void MainWindow::openActionTriggered()
{
    ui->previewMenu->setEnabled(false);
    ui->gridLayout->~QGridLayout();
    this->dummy->~QWidget();
    ui->gridLayout = new QGridLayout(this);
    this->dummy = new QWidget(this);
    dummy->setLayout(ui->gridLayout);
    ui->scrollArea->setWidget(dummy);

    importImages();
    ui->previewMenu->setEnabled(true);
}

void MainWindow::aboutActionTriggered()
{
    QMessageBox::information(this, "About", "This is version 0.1 of report generator.\n Author: Saurabh Rawat\n\nThis software is provided under GPLv3 License.");
}

void MainWindow::generateReport(QString path)
{

    ui->openAction->setDisabled(true);

    //get rid of temp image files
    QDir delDir(QDir::tempPath() + "/compressed_images_temp");
    QStringList filters;
    filters << "*.jpg";
    delDir.setNameFilters(filters);
    QStringList delList = delDir.entryList();
    for(int i = 0; i < delList.size(); i++)
        delDir.remove(delList.at(i));
    delDir.rmdir(delDir.absolutePath());

    QDir writeDir(QDir::tempPath());
    writeDir.mkdir("compressed_images_temp");
    QString writePath = writeDir.absolutePath() + "/" + "compressed_images_temp/";
    QString html("<html><body>");
    QProgressDialog progressDialog("Generating Preview...", "Abort", 0, ((ui->gridLayout->rowCount()) * (ui->gridLayout->columnCount())), this);
    progressDialog.setAutoClose(true);

    for(int i = 0; i < ((ui->gridLayout->rowCount()) * (ui->gridLayout->columnCount())); i++) {
        QImage imageInput;
        QImage image;
        QString fileName;
        QImageWriter writer;
        writer.setFormat("jpg");

        if(ui->gridLayout->itemAt(i) == NULL) {
            break;
        }

        Label *current = (Label*)ui->gridLayout->itemAt(i)->widget();
        progressDialog.setValue(i);

        if(current->include) {
            imageInput.load(current->imagePath);
            image    =  imageInput.scaled(ui->imageWidth->value(), ui->imageHeight->value(), Qt::KeepAspectRatio);
            fileName = QString("%1.jpg").arg(QString::number(i));
            writer.setFileName(writePath + fileName);
            writer.setQuality(ui->qualitySlider->value());
            writer.write(image);
            html += QString("<div style='margin: 10%;'><div style='margin: 0 auto;'><img src=%1/></div><p>%2</p></div>").arg(writePath + QString::number(i) + ".jpg", current->description);
        }

    }
    progressDialog.setValue(((ui->gridLayout->rowCount()) * (ui->gridLayout->columnCount())));

    html += QString("</body></html>");
    QTextDocument doc;
    doc.setHtml(html);
    QPrinter printer(QPrinter::HighResolution);
    QTextDocumentWriter writer;

    if(ui->pdfCheck->isChecked()) {
        printer.setOutputFileName(path + "Report.pdf");
        printer.setOutputFormat(QPrinter::PdfFormat);
        doc.print(&printer);
    }

    if(ui->odtCheck->isChecked()) {
        writer.setFileName(path + "Report.odt");
        writer.setFormat("odt");
        writer.write(&doc);
    }

    if(ui->htmlCheck->isChecked()) {
        writer.setFileName(path + "Report.html");
        writer.setFormat("html");
        writer.write(&doc);
    }

    ui->openAction->setEnabled(true);
}

void MainWindow::generateButtonClicked()
{
    QString ignore;
    generateReport(ignore);
    if(ui->pdfCheck->isChecked()) {
        QFileInfo reportFile(QDir::tempPath() + "/Report.pdf");
        ui->pdfSizeLabel->setText("PDF Size : " + QString::number(reportFile.size()/1024) + "kB");
    }
    if(ui->odtCheck->isChecked()) {
        QFileInfo reportFile(QDir::tempPath() + "/Report.odt");
        ui->odtSizeLabel->setText("ODT Size : " + QString::number(reportFile.size()/1024) + "kB");
    }
}

void MainWindow::exitActionTriggered()
{
    qApp->quit();
}

void MainWindow::previewPdfActionTriggered()
{
    ui->previewMenu->setEnabled(false);
    ui->pdfCheck->setChecked(true);
    ui->htmlCheck->setChecked(false);
    ui->odtCheck->setChecked(false);
    QDir delDir(QDir::tempPath());
    delDir.remove("Report.pdf");
    generateReport(QDir::tempPath() + "/");
    QDesktopServices::openUrl(QUrl(QDir::tempPath() + "/" + "Report.pdf"));
    QFileInfo reportFile(QDir::tempPath() + "/Report.pdf");
    double size = reportFile.size()/1024;

    if(size >= 1024.0)
        ui->pdfSizeLabel->setText("PDF Size :  " + QString::number(size/1024.0, 'g', 3) + "MB");
    else
        ui->pdfSizeLabel->setText("PDF Size :  " + QString::number(size) + "kB");

    ui->previewMenu->setEnabled(true);
}


void MainWindow::previewHtmlActionTriggered()
{
    ui->previewMenu->setEnabled(false);
    ui->pdfCheck->setChecked(false);
    ui->htmlCheck->setChecked(true);
    ui->odtCheck->setChecked(false);
    QDir delDir(QDir::tempPath());
    delDir.remove("Report.html");
    generateReport(QDir::tempPath() + "/");
    QDesktopServices::openUrl(QUrl(QDir::tempPath() + "/" + "Report.html"));
    QFileInfo reportFile(QDir::tempPath() + "/Report.html");
    ui->previewMenu->setEnabled(true);
}


void MainWindow::previewOdtActionTriggered()
{
    ui->previewMenu->setEnabled(false);
    ui->pdfCheck->setChecked(false);
    ui->htmlCheck->setChecked(false);
    ui->odtCheck->setChecked(true);
    QDir delDir(QDir::tempPath());
    delDir.remove("Report.odt");
    generateReport(QDir::tempPath() + "/");
    QDesktopServices::openUrl(QUrl(QDir::tempPath() + "/" + "Report.odt"));
    QFileInfo reportFile(QDir::tempPath() + "/Report.odt");
    double size = reportFile.size()/1024;

    if(size >= 1024.0)
        ui->odtSizeLabel->setText("ODT Size :  " + QString::number(size/1024.0, 'g', 3) + "MB");
    else
        ui->odtSizeLabel->setText("ODT Size :  " + QString::number(size) + "kB");

    ui->previewMenu->setEnabled(true);
}

void MainWindow::qualitySliderChanged(int value)
{
    if(ui->qualitySpinBox->value() != value)
        ui->qualitySpinBox->setValue(value);
}

void MainWindow::qualitySpinBoxChanged(int value)
{
    if(ui->qualitySlider->value() != value)
        ui->qualitySlider->setValue(value);
}


