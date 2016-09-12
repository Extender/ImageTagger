#ifndef WEBENGINEPAGEEX_H
#define WEBENGINEPAGEEX_H

#include <QtWebEngineWidgets/QtWebEngineWidgets>

class WebEnginePageEx : public QWebEnginePage
{
    Q_OBJECT

public:
    WebEnginePageEx();
    bool acceptNavigationRequest(const QUrl &url, NavigationType type, bool isMainFrame);
};

#endif // WEBENGINEPAGEEX_H
