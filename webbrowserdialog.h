#ifndef WEBBROWSERDIALOG_H
#define WEBBROWSERDIALOG_H

#include <QDialog>
#include <QtWebEngineWidgets/QtWebEngineWidgets>

#include "webenginepageex.h"

namespace Ui {
class WebBrowserDialog;
}

class WebBrowserDialog : public QDialog
{
    Q_OBJECT

public:
    QWebEngineView *webView;
    WebEnginePageEx *webPage;

    explicit WebBrowserDialog(QString title="",QString source="",QWidget *parent=0);
    ~WebBrowserDialog();

private:
    Ui::WebBrowserDialog *ui;
};

#endif // WEBBROWSERDIALOG_H
