#ifndef TIMECAMPDESKTOP_TCWEBENGINEVIEW_H
#define TIMECAMPDESKTOP_TCWEBENGINEVIEW_H

#include <QObject>
#include <QWebEngineView>

class TCWebEngineView : public QWebEngineView
{
    Q_OBJECT

public:
    explicit TCWebEngineView(QWidget *parent = nullptr);
    ~TCWebEngineView() override;

protected:
    QWebEngineView *createWindow(QWebEnginePage::WebWindowType type) override;

private:
    TCWebEngineView *result;
};


#endif //TIMECAMPDESKTOP_TCWEBENGINEVIEW_H
