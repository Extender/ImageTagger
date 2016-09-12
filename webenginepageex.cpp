#include "webenginepageex.h"

WebEnginePageEx::WebEnginePageEx()
{

}

bool WebEnginePageEx::acceptNavigationRequest(const QUrl &url, QWebEnginePage::NavigationType type, bool isMainFrame)
{
    if(type==QWebEnginePage::NavigationTypeLinkClicked)
    {
        QDesktopServices::openUrl(url);
        return false;
    }
    return true;
}
