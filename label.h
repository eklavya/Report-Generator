#ifndef LABEL_H
#define LABEL_H

#include <QLabel>
#include <QtGui>

class Label : public QLabel
{
    Q_OBJECT

public:
    Label(QWidget *parent = 0);

protected:
    void dragEnterEvent(QDragEnterEvent*);
    void dragMoveEvent(QDragMoveEvent *);
    void dropEvent(QDropEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);

private:
    void startDrag();
    QPoint startPos;

public:
    //image description
    QString description;
    //path of the image file, the label is displaying
    QString imagePath;
    //position in image grid
    int position;
    //to be included in report
    bool include;

signals:
    void selected(Label*);
};


#endif // LABEL_H
