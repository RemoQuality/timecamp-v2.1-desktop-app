#include <QPainter>
#include <QFont>
#include <QPen>
#include <QDebug>
#include <cmath>

#include "FloatingWidget.h"
#include "src/Settings.h"

FloatingWidget::FloatingWidget(QWidget *parent)
    : QWidget(parent, Qt::Tool | Qt::X11BypassWindowManagerHint | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint) {
    this->setAcceptDrops(false);
//    this->setAttribute(Qt::WA_TranslucentBackground, true);
    this->background = QPixmap(MAIN_ICON);
    this->radius = 4;
    this->gripSize = QSize(15, 10);
    this->setMinimumSize(180, 20);
    this->setMaximumSize(600, 64);
    this->setStyleSheet("background-color:green;");
    this->hide();

    FloatingWidgetWasInitialised = true;

    taskTextLabel = new ClickableLabel(this);
    timerTextLabel = new ClickableLabel(this);
    startStopLabel = new ClickableLabel(this);
    startStopLabel->setText(PLAY_BUTTON);

    QMetaObject::Connection conn1 = QObject::connect(taskTextLabel, &ClickableLabel::clicked,
                                                     [&]() {
                                                         emit taskNameClicked();
                                                     });
    this->setContextMenuPolicy(Qt::CustomContextMenu);

    QMetaObject::Connection conn2 = QObject::connect(this, &FloatingWidget::customContextMenuRequested,
                                                     this, &FloatingWidget::showContextMenu);

    QMetaObject::Connection conn3 = QObject::connect(startStopLabel, &ClickableLabel::clicked,
                                                     this, &FloatingWidget::startStopClicked);
}

void FloatingWidget::updateWidgetStatus(bool canBeStopped, QString timerName) {
    if (!canBeStopped || timerName.isEmpty()) {
        timerName = "No task";
        startStopLabel->setText(PLAY_BUTTON);
    }
    if (canBeStopped) {
        startStopLabel->setText(PAUSE_BUTTON);
    }
    this->setTaskText(timerName);
}

void FloatingWidget::startStopClicked() {
    QString current = startStopLabel->text();
    if (current == PLAY_BUTTON) {
        emit playButtonClicked();
        startStopLabel->setText(PAUSE_BUTTON);
        return;
    }
    if (current == PAUSE_BUTTON) {
        emit pauseButtonClicked();
        startStopLabel->setText(PLAY_BUTTON);
        return;
    }
}

void FloatingWidget::showContextMenu(const QPoint &pos) {
    contextMenu->exec(mapToGlobal(pos));
}

void FloatingWidget::handleSpacingEvents() {
//    qInfo("Size: %d x %d", size().width(), size().height());
    if (FloatingWidgetWasInitialised) {
        this->setUpdatesEnabled(false);
        settings.setValue("floatingWidgetGeometry", saveGeometry()); // save window position
        settings.sync();
        this->setUpdatesEnabled(true);
    }
}

void FloatingWidget::closeEvent(QCloseEvent *event) {
    hide(); // hide our window when X was pressed
    event->ignore(); // don't do the default action (which usually is app exit)
}

void FloatingWidget::resizeEvent(QResizeEvent *event) {
    QPainterPath path;
    path.addRoundedRect(this->rect(), radius, radius);
    QRegion maskedRegion(path.toFillPolygon().toPolygon());
    setMask(maskedRegion);
    this->handleSpacingEvents();
}

void FloatingWidget::mousePressEvent(QMouseEvent *event) {
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
//    qDebug() << "IsResizing: " << resizing;
}

void FloatingWidget::mouseMoveEvent(QMouseEvent *event) {
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

int FloatingWidget::scaleToFit(double height) {
    double newHeight = height - 2;
    if (newHeight > 62) {
        return 62;
    }
    return qRound(newHeight);
}

void FloatingWidget::paintEvent(QPaintEvent *) {
    int margin = 4;
    QPainter painter(this);
    painter.drawPixmap(margin / 2, this->height() - scaleToFit(this->height()),
                       background.scaledToHeight(scaleToFit(this->height()) - margin / 2,
                                                 Qt::SmoothTransformation));
//    painter.setRenderHint(QPainter::Antialiasing);

    QFont usedFont = painter.font();
    int fontSize = scaleToFit((pow(this->height(), 1.0 / 3.0) * 12) - 20);
    usedFont.setPixelSize(fontSize);

    QFontMetrics metrics(usedFont);

    int iconWidth = background.scaledToHeight(scaleToFit(this->height())).width();
    int textStartingPoint = (this->height() - metrics.boundingRect(taskText).height()) / 2;
    int textHeight = metrics.boundingRect(taskText).height();

    int special_offset = 0;
    if (startStopLabel->text() == PLAY_BUTTON) {
        special_offset = -2;
    }
    if (startStopLabel->text() == PAUSE_BUTTON) {
        special_offset = 2;
    }

    taskTextLabel->setFont(usedFont);
    taskTextLabel->setText(taskText);
    taskTextLabel->setGeometry(iconWidth + margin,
                               textStartingPoint,
                               metrics.boundingRect(taskText).width(),
                               textHeight
    );

    int startStopWidth = fontSize + margin;
    startStopLabel->setFont(usedFont);
    startStopLabel->setGeometry(this->width() - startStopWidth - margin,
                                textStartingPoint + special_offset,
                                startStopWidth,
                                startStopWidth
    );

    int timerTextWidth = metrics.boundingRect(timerText).width() + margin;
    timerTextLabel->setFont(usedFont);
    timerTextLabel->setText(timerText);
    timerTextLabel->setGeometry(this->width() - startStopWidth - timerTextWidth - margin * 2,
                                textStartingPoint,
                                timerTextWidth,
                                textHeight
    );
}

bool FloatingWidget::mouseInGrip(QPoint mousePos) {
//    qDebug() << "Mouse: (" << mousePos.x() << ", " << mousePos.y() << ")";
//    qDebug() << "Widget: (" << width() << ", " << height() << ")";
//    qDebug() << "Activation point: (" << width() - gripSize.width() << ", " << height() - gripSize.height() << ")";

    // "handle" is in the lower right hand corner
    return ((mousePos.x() > (width() - gripSize.width()))
            && (mousePos.y() > (height() - gripSize.height())));
}

QSize FloatingWidget::sizeHint() const {
    return QSize(245, 21);
}

void FloatingWidget::open() {
    qDebug(__FUNCTION__);
    settings.sync();
    restoreGeometry(settings.value("floatingWidgetGeometry").toByteArray());
    show();
    raise();
    setWindowState((windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
}

void FloatingWidget::showMe() {
    this->open();
}

void FloatingWidget::hideMe() {
    this->hide();
}

bool FloatingWidget::isHidden() {
    return !this->isVisible();
}

void FloatingWidget::setTimerText(QString text) {
    this->timerText = text;
    this->update();
}

void FloatingWidget::setTaskText(QString text) {
    this->taskText = text;
    this->update();
}

void FloatingWidget::setMenu(QMenu *contextMenu) {
    FloatingWidget::contextMenu = contextMenu;
}

void FloatingWidget::setIcon(QString iconPath) {

}
