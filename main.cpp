#include "mainwindow.h"
#include <QApplication>
#include <QDir>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QString workingDirectory="";
    for(int i=0;i<argc;i++)
    {
        QDir d(argv[i]);
        if(d.exists())
        {
            workingDirectory=QString(argv[i]);
            break;
        }
    }

    MainWindow w(workingDirectory);
    w.show();

    return a.exec();
}
