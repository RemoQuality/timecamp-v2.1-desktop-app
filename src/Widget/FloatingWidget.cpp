#include <QPainter>
#include <QFont>
#include <QPen>
#include <QDebug>
#include <cmath>

#include "FloatingWidget.h"
#include "src/Settings.h"

FloatingWidget::FloatingWidget(QWidget *parent)
    : QWidget(parent, Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint)
{
    this->setAcceptDrops(false);
//    this->setAttribute(Qt::WA_TranslucentBackground, true);
    this->background = QPixmap(MAIN_ICON);
    this->radius = 4;
    this->gripSize = QSize(15, 15);
    this->setMinimumSize(150, 20);
    this->setMaximumSize(600, 64);
    this->setStyleSheet("background-color:green;");
    this->hide();

    FloatingWidgetWasInitialised = true;
}

void FloatingWidget::handleSpacingEvents()
{
    qInfo("Size: %d x %d", size().width(), size().height());
    if (FloatingWidgetWasInitialised) {
        this->setUpdatesEnabled(false);
        settings.setValue("floatingWidgetGeometry", saveGeometry()); // save window position
        settings.sync();
        this->setUpdatesEnabled(true);
    }
}

void FloatingWidget::closeEvent(QCloseEvent *event)
{
    hide(); // hide our window when X was pressed
    event->ignore(); // don't do the default action (which usually is app exit)
}

//void FloatingWidget::moveEvent(QMoveEvent *event)
//{
//    this->handleSpacingEvents();
//    QWidget::moveEvent(event); // do the default "whatever happens on move"
//}
//
//void FloatingWidget::changeEvent(QEvent *event)
//{
//    if (event->type() == QEvent::WindowStateChange) {
//        this->handleSpacingEvents();
//    }
//    QWidget::changeEvent(event);
//}
//
//void FloatingWidget::resizeEvent(QResizeEvent *event)
//{
//    this->handleSpacingEvents();
//    QWidget::resizeEvent(event); // do the default "whatever happens on resize"
//}
void FloatingWidget::resizeEvent(QResizeEvent *event)
{
    QPainterPath path;
    path.addRoundedRect(this->rect(), radius, radius);
    QRegion maskedRegion(path.toFillPolygon().toPolygon());
    setMask(maskedRegion);
    this->handleSpacingEvents();
}

void FloatingWidget::mousePressEvent(QMouseEvent *event)
{
    // Check if we hit the grip handle
    if (event->button() == Qt::LeftButton) {
        if (mouseInGrip(event->pos())) {
            resizing = true;
            oldPos = event->pos();
            event->accept();
        } else {
            resizing = false;
            dragPosition = event->globalPos() - frameGeometry().topLeft();
            event->accept();
        }
    }
    qDebug() << "IsResizing: " << resizing;
}

void FloatingWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        if (resizing) {
            // adapt the widget size based on mouse movement
            QPoint delta = event->pos() - oldPos;
            oldPos = event->pos();
            resize(width() + delta.x(), height() + delta.y());
            updateGeometry();
        } else {
            move(event->globalPos() - dragPosition);
            event->accept();
        }
    }
    this->handleSpacingEvents();
}

int FloatingWidget::scaleToFit(double height)
{
    double newHeight = height - 2;
    if (newHeight > 62) {
        return 62;
    }
    return qRound(newHeight);
}

void FloatingWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.drawPixmap(1, this->height() - scaleToFit(this->height()),
                       background.scaledToHeight(scaleToFit(this->height()) - 2,
                       Qt::SmoothTransformation));
    painter.setRenderHint(QPainter::Antialiasing);

    QFont usedFont = painter.font();
    int fontSize = scaleToFit((pow(this->height(), 1.0/3.0) * 12) - 20);
    usedFont.setPixelSize(fontSize);
    painter.setFont(usedFont);

    QPen usedPen = painter.pen();
    usedPen.setColor(QColor(255, 255, 255)); // white font color
    painter.setPen(usedPen);

    QFontMetrics metrics(usedFont);
    painter.drawText(this->width() - metrics.boundingRect(timerText).width() - 4,
                     (this->height() + fontSize) / 2 - (fontSize / 12),
                     timerText);
}

bool FloatingWidget::mouseInGrip(QPoint mousePos)
{
    qDebug() << "Mouse: (" << mousePos.x() << ", " << mousePos.y() << ")";
    qDebug() << "Widget: (" << width() << ", " << height() << ")";
    qDebug() << "Activation point: (" << width() - gripSize.width() << ", " << height() - gripSize.height() << ")";

    // "handle" is in the lower right hand corner
    return ((mousePos.x() > (width() - gripSize.width()))
        && (mousePos.y() > (height() - gripSize.height())));
}

QSize FloatingWidget::sizeHint() const
{
    return QSize(245, 21);
}

void FloatingWidget::open()
{
    qDebug(__FUNCTION__);
    settings.sync();
//    restoreGeometry(settings.value("floatingWidgetGeometry").toByteArray());
    show();
    raise();
    setWindowState((windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
}

void FloatingWidget::showMe()
{
    qDebug(__FUNCTION__);
    this->open();
}
void FloatingWidget::hideMe()
{
    qDebug(__FUNCTION__);
    this->hide();
}
bool FloatingWidget::isHidden()
{
    qDebug(__FUNCTION__);
    return !this->isVisible();
}
void FloatingWidget::setTimerText(QString text)
{
    this->timerText = text;
    this->update();
}
void FloatingWidget::setMenu(QMenu *)
{

}
void FloatingWidget::setIcon(QString iconPath)
{

}
