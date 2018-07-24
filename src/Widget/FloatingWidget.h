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
    QSize sizeHint() const override;

public slots:
    void open();
    void showMe() override;
    void hideMe() override;
    void setTimerText(QString text) override;
    void setTaskText(QString text) override;
    void setMenu(QMenu *) override;
    void setIcon(QString iconPath) override;
    bool isHidden() override;
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
    int radius = 4;
    int margin = 4;
    bool FloatingWidgetWasInitialised = false;

    bool resizing;
    QPixmap background;
    QString taskText;
    QString timerText;
    QPoint dragPosition;
    QPoint oldPos;
    QSize gripSize;
    QSettings settings;
    int scaleToFit(double height);

    QLabel *timerTextLabel;
    ClickableLabel *taskTextLabel;
    ClickableLabel *startStopLabel;
    QMenu *contextMenu;

    QFont usedFont;
    int fontSize;
    int iconWidth;
    int textStartingPoint;
    int textHeight;
    int startStopWidth;
    int timerTextWidth;
};

#endif //TIMECAMPDESKTOP_FLOATINGWIDGET_H
