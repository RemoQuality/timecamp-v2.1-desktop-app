//
// Created by Timecamp on 12/03/2018.
//

#include "TCWebEngineView.h"
#include "TCNavigationInterceptor.h"

// Workaround to get opened URL from: https://bugreports.qt.io/browse/QTBUG-56637


TCWebEngineView::TCWebEngineView(QWidget *parent)
        : QWebEngineView(parent),
          result(nullptr)
{
}

TCWebEngineView::~TCWebEngineView()
{
    delete result;
}

QWebEngineView *TCWebEngineView::createWindow(QWebEnginePage::WebWindowType type)
{
    Q_UNUSED(type);
//    qDebug() << "createWindow called, returning NavigationRequestInterceptor";
    if (!result) {
        result = new TCWebEngineView();
        result->setPage(new TCNavigationInterceptor(this->page()));
    }
    return result;
}