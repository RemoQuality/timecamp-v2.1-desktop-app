#include <QPainter>
#include <QFont>
#include <QPen>
#include <QDebug>
#include <QTimer>
#include <QTime>
#include <QStyle>
#include <QBitmap>
#include <cmath>

#include "FloatingWidget.h"
#include "src/Settings.h"

FloatingWidget::FloatingWidget(QWidget *parent)
    : QWidget(parent, Qt::Tool | Qt::X11BypassWindowManagerHint | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint) {
    this->setAcceptDrops(false); // don't let users drop stuff on our widget
//    this->setAttribute(Qt::WA_TranslucentBackground, true);
    this->background = QPixmap(MAIN_ICON);
    this->gripSize = QSize(16, 8);
    this->setMinimumSize(180, 20);
    this->setMaximumSize(600, 64);
    this->setStyleSheet("background-color:green;");
    this->hide();
    this->setMouseTracking(true); // we need this to show resize arrows


    playPixmap = style()->standardPixmap(QStyle::SP_MediaPlay);
    QBitmap playMask = playPixmap.createMaskFromColor(Qt::transparent, Qt::MaskInColor);
    playPixmap.fill(Qt::white);
    playPixmap.setMask(playMask);

    pausePixmap = style()->standardPixmap(QStyle::SP_MediaPause);
    QBitmap pauseMask = pausePixmap.createMaskFromColor(Qt::transparent, Qt::MaskInColor);
    pausePixmap.fill(Qt::white);
    pausePixmap.setMask(pauseMask);

    FloatingWidgetWasInitialised = true;

    timerTextLabel = new QLabel(this);
    taskTextLabel = new ClickableLabel(this);
    startStopLabel = new ClickableLabel(this);
    QString QLabelStyle("QLabel { color: white; }");
    timerTextLabel->setStyleSheet(QLabelStyle);
    taskTextLabel->setStyleSheet(QLabelStyle);
    startStopLabel->setStyleSheet(QLabelStyle);

    startStopLabel->setScaledContents(true); // resize play/pause icon

    QMetaObject::Connection conn1 = QObject::connect(taskTextLabel, &ClickableLabel::clicked,
                                                     this, &FloatingWidget::emitTaskNameClicked);
    this->setContextMenuPolicy(Qt::CustomContextMenu);

    QMetaObject::Connection conn2 = QObject::connect(this, &FloatingWidget::customContextMenuRequested,
                                                     this, &FloatingWidget::showContextMenu);

    QMetaObject::Connection conn3 = QObject::connect(startStopLabel, &ClickableLabel::clicked,
                                                     this, &FloatingWidget::startStopClicked);

    auto *oneSecondTimer = new QTimer();
    QObject::connect(oneSecondTimer, &QTimer::timeout, this, &FloatingWidget::oneSecTimerTimeout);
    oneSecondTimer->start(1000);
    updateWidgetStatus(false, "");
}

void FloatingWidget::oneSecTimerTimeout() {
    if (timerElapsed > 0) {
        QTime time(0,0,0);
        time = time.addSecs(timerElapsed);
        if (time.hour() > 0) {
            this->setTimerText(time.toString("H:mm:ss"));
        } else {
            this->setTimerText(time.toString("m:ss"));
        }
        timerElapsed++;
    }
}

void FloatingWidget::emitTaskNameClicked() {
    emit taskNameClicked();
}

void FloatingWidget::updateWidgetStatus(bool canBeStopped, QString timerName) {
    if (canBeStopped) {
        startStopLabel->setPixmap(pausePixmap);
        timerRunning = true;
    } else {
        startStopLabel->setPixmap(playPixmap);
        timerRunning = false;
        this->setTimerText(""); // set empty text (no 0:00 for timer when no task is running)
        timerName = "No task";
        timerElapsed = 0;
    }
    if (timerName.isEmpty()) {
        timerName = "No task";
    }
    this->setTaskText(timerName);
}

void FloatingWidget::startStopClicked() {
    if (timerRunning) {
        emit pauseButtonClicked();
        startStopLabel->setPixmap(playPixmap);
    }else{
        emit playButtonClicked();
        startStopLabel->setPixmap(pausePixmap);
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
    // TODO: this is copied from the MainWidget, not 100% sure we need it
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
        this->handleSpacingEvents();
    }
//    qDebug() << "IsResizing: " << resizing;
}

void FloatingWidget::mouseMoveEvent(QMouseEvent *event) {
    // this bit is for setting a resize cursor
    if (mouseInGrip(event->pos())) {
        if(this->cursor().shape() != Qt::SizeFDiagCursor) {
            this->setCursor(Qt::SizeFDiagCursor);
        }
    } else {
        if(this->cursor().shape() == Qt::SizeFDiagCursor) {
            this->unsetCursor();
        }
    }
    // this is for actual js-like-dragStart
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
        this->handleSpacingEvents();
    }
}

int FloatingWidget::scaleToFit(double height) {
    // magical arbitrary value, to keep widget in sane dimentions
    double newHeight = height - 2;
    if (newHeight > 62) {
        return 62;
    }
    return qRound(newHeight);
}

void FloatingWidget::paintEvent(QPaintEvent *) {
//     qDebug() << __FUNCTION__;

    QPainter painter(this);
    painter.drawPixmap(margin / 2, this->height() - scaleToFit(this->height()),
                       background.scaledToHeight(scaleToFit(this->height()) - margin / 2,
                                                 Qt::SmoothTransformation));
//    painter.setRenderHint(QPainter::Antialiasing);

    usedFont = painter.font();
    fontSize = scaleToFit((pow(this->height(), 1.0 / 3.0) * 12) - 20); // magic, should keep the values scaling nicely in a f=x^(1/3) shape, scaled
    usedFont.setPixelSize(fontSize);

    QFontMetrics metrics(usedFont);

    iconWidth = background.scaledToHeight(scaleToFit(this->height())).width();
    textStartingPoint = (this->height() - metrics.boundingRect(taskText).height()) / 2;
    textHeight = metrics.boundingRect(taskText).height();

    // special_offset is used, because PLAY and PAUSE buttons in default font are weirdly spaced
    int special_offset = 0;
    special_offset *= fontSize/12;

    taskTextLabel->setFont(usedFont);
    taskTextLabel->setText(taskText);
    taskTextLabel->setGeometry(iconWidth + margin,
                               textStartingPoint,
                               metrics.boundingRect(taskText).width(),
                               textHeight
    );

    startStopWidth = fontSize + margin;
    startStopLabel->setFont(usedFont);
    startStopLabel->setGeometry(this->width() - startStopWidth - margin,
                                textStartingPoint + special_offset,
                                startStopWidth,
                                startStopWidth
    );

    timerTextWidth = metrics.boundingRect(timerText).width() + margin;
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
    return {245, 21}; // default size; same dimentions as in the old app
}

void FloatingWidget::open() {
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
    if(text != this->timerText) { // call update only when text actually changed
        this->timerText = text;
        this->update();
    }
}

void FloatingWidget::setTaskText(QString text) {
    if(text != this->taskText) { // call update only when text actually changed
        this->taskText = text;
        this->update();
    }
}

void FloatingWidget::setMenu(QMenu *contextMenu) {
    FloatingWidget::contextMenu = contextMenu;
}

void FloatingWidget::setIcon(QString iconPath) {
    // FloatingWidget has no icon for now
    // if it had, it would be in the left corner
}

void FloatingWidget::setTimerElapsed(int timerElapsed)
{
    FloatingWidget::timerElapsed = timerElapsed;
}
