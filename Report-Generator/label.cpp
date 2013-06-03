#include "label.h"
#include "ui_mainwindow.h"

Label::Label(QWidget *parent) : QLabel(parent)
{
    setAcceptDrops(true);
    include = true;
    //ugly hack to get to the mainwindow slot
    QObject::connect(this, SIGNAL(selected(Label*)), parent->parent()->parent()->parent()->parent(), SLOT(imageSelected(Label*)));
}

void Label::startDrag()
{
    QMimeData *mimeData = new QMimeData;
    QVariant *imageVariant = new QVariant(QVariant::Image);
    *imageVariant = this->pixmap()->toImage();
    mimeData->setImageData(*imageVariant);
    mimeData->setHtml(this->description);
    mimeData->setText(this->imagePath);
    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->start();
}

void Label::dropEvent(QDropEvent *event)
{
    if(event->mimeData()->hasImage()) {
        QPixmap current = QPixmap(*(this->pixmap()));
        this->setPixmap(QPixmap::fromImage(event->mimeData()->imageData().value<QImage>()));
        QString currentHtml = this->description;
        QString currentImage = this->imagePath;
        this->description = event->mimeData()->html();
        this->imagePath = event->mimeData()->text();
        Label *from = (Label*)event->source();
        from->setPixmap(current);
        from->description = currentHtml;
        from->imagePath = currentImage;
        event->setDropAction(Qt::MoveAction);
        event->accept();
    }
}

void Label::dragEnterEvent(QDragEnterEvent *event)
{
    if(event->mimeData()->hasImage()) {
        event->setDropAction(Qt::MoveAction);
        event->accept();
    }
}

void Label::mousePressEvent(QMouseEvent *event)
    {
        if (event->button() == Qt::LeftButton)
            startPos = event->pos();
        //emit signal to change preview label
        emit selected(this);
        QLabel::mousePressEvent(event);
    }

void Label::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
            int distance = (event->pos() - startPos).manhattanLength();
            if (distance >= QApplication::startDragDistance())
                startDrag();
        }
    QLabel::mouseMoveEvent(event);
}

void Label::dragMoveEvent(QDragMoveEvent *event)
{
    if(event->mimeData()->hasImage()) {
        event->setDropAction(Qt::MoveAction);
        event->accept();
    }
}
