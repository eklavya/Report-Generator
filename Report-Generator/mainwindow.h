#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "label.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
private:
    Ui::MainWindow *ui;
    //current working image directory
    QDir *imageDir;
    //label in preview
    Label *current;
    //dummy widget to set scroll area
    QWidget *dummy;

    void generateReport(QString path);
    void importImages();

public slots:
    void imageSelected(Label*);
    void descriptionChanged();
    void generateButtonClicked();
    void openActionTriggered();
    void exitActionTriggered();
    void previewPdfActionTriggered();
    void previewHtmlActionTriggered();
    void previewOdtActionTriggered();
    void aboutActionTriggered();
    void qualitySliderChanged(int);
    void qualitySpinBoxChanged(int);
    void includeCheckChanged(bool);
};

#endif // MAINWINDOW_H
