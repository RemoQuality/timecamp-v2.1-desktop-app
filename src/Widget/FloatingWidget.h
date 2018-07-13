#ifndef TIMECAMPDESKTOP_FLOATINGWIDGET_H
#define TIMECAMPDESKTOP_FLOATINGWIDGET_H

#include <QWidget>
#include <QCloseEvent>
#include <QResizeEvent>
#include <QSettings>
#include <QPainter>
#include "src/Overrides/ClickableLabel.h"

#include "Widget.h"
class FloatingWidget: public QWidget, public Widget
{
Q_OBJECT
    Q_DISABLE_COPY(FloatingWidget)

public:
    explicit FloatingWidget(QWidget *parent = nullptr);
    QSize sizeHint() const;

public slots:
    void open();
    void showMe();
    void hideMe();
    void setTimerText(QString text);
    void setTaskText(QString text);
    void setMenu(QMenu *);
    void setIcon(QString iconPath);
    bool isHidden();
    void showContextMenu(const QPoint &);
    void startStopClicked();
    void updateWidgetStatus(bool, QString);

signals:
    void taskNameClicked();
    void pauseButtonClicked();
    void playButtonClicked();

protected:
    void handleSpacingEvents();
    void resizeEvent(QResizeEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *) override;
    bool mouseInGrip(QPoint mousePos);

private:
    int radius;
    bool resizing;
    QPixmap background;
    QString taskText;
    QString timerText;
    QPoint dragPosition;
    QPoint oldPos;
    QSize gripSize;
    bool FloatingWidgetWasInitialised = false;
    QSettings settings;
    int scaleToFit(double height);

    ClickableLabel *taskTextLabel;
    ClickableLabel *timerTextLabel;
    ClickableLabel *startStopLabel;
    QMenu *contextMenu;
};

#endif //TIMECAMPDESKTOP_FLOATINGWIDGET_H
