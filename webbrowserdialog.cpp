#include "webbrowserdialog.h"
#include "ui_webbrowserdialog.h"

WebBrowserDialog::WebBrowserDialog(QString title, QString source, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::WebBrowserDialog)
{
    Qt::WindowFlags newWindowFlags=(windowFlags()&(~(Qt::WindowContextHelpButtonHint)))|Qt::WindowMaximizeButtonHint;
    if(parent==0) // If taskbar entry present
        newWindowFlags|=Qt::WindowMinimizeButtonHint;
    setWindowFlags(newWindowFlags);
    ui->setupUi(this);
    setWindowTitle(title);
    webView=new QWebEngineView();
    webPage=new WebEnginePageEx();
    webPage->setHtml(source);
    webView->setPage(webPage);
    ui->horizontalLayout->addWidget(webView);
}

WebBrowserDialog::~WebBrowserDialog()
{
    delete ui;
}
