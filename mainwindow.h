#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QDir>
#include <QMessageBox>
#include <QByteArray>
#include <QBuffer>
#include <QStandardPaths>
#include <QColorDialog>
#include <vector>
#include <list>
#include <ctime>

#include "io.h"
#include "text.h"

#include "defaulttagcolor.h"
#include "mainwindowex.h"
#include "extcolordefs.h"
#include "graphicssceneex.h"
#include "graphicsviewex.h"
#include "webbrowserdialog.h"

// WARNING: Must have exactly one decimal place!
// The format "1.11" is illegal!

#define CURRENT_VERSION 1.1f
#define MIN_VERSION 1.1f

#define num(n) (QString::number((n)))

using namespace std;

namespace Ui {
class MainWindow;
}

class GraphicsSceneEx;
class GraphicsViewEx;

class TagPath
{
public:
    bool isRect;
    QRectF rect;
    QPainterPath path;

    TagPath(QRectF _rect)
    {
        isRect=true;
        rect=QRectF(_rect.topLeft(),_rect.size()); // Do not use QRectF(_rect)! No copy constructor available!
    }

    TagPath(QPainterPath _path)
    {
        isRect=false;
        path=_path.toReversed().toReversed(); // Do not use QPainterPath(_path)! No copy constructor available!
    }
};

class ItemClass
{
public:
    QString name;
    QList<pair<char*,TagPath>*> *tags;
    QImage *image;
    QString extension;

    ItemClass()
    {
        name="";
        tags=new QList<pair<char*,TagPath>*>();
        image=0;
        extension="";
    }

    ItemClass(QString _name)
    {
        name=_name;
        tags=new QList<pair<char*,TagPath>*>();
        image=0;
        extension="";
    }

    ItemClass(QString _name,QString _extension)
    {
        name=_name;
        tags=new QList<pair<char*,TagPath>*>();
        image=0;
        extension=_extension;
    }

    ItemClass(QString _name,QImage *_image)
    {
        name=_name;
        tags=new QList<pair<char*,TagPath>*>();
        image=_image;
        extension="";
    }

    ItemClass(QString _name,QImage *_image,QString _extension)
    {
        name=_name;
        tags=new QList<pair<char*,TagPath>*>();
        image=_image;
        extension=_extension;
    }

    ~ItemClass()
    {
        size_t s=tags->count();
        for(size_t i=0;i<s;i++)
        {
            auto p=tags->at(i);
            free(p->first);
            delete p;
        }
        delete tags;
        delete image;
    }
};

// Define item type here:
typedef ItemClass new_item_t;

typedef new_item_t* item_t;

class MainWindow : public MainWindowEx
{
    Q_OBJECT

public:
    static MainWindow *instance;

    QString workingDir;
    QFileDialog *workingDirDialog;
    QColorDialog *colorDialog;
    QStringList *defaultFilters;
    int selectedIndex;
    item_t selectedItem;
    vector<item_t> *items;
    GraphicsSceneEx *scene;
    QGraphicsPixmapItem *pixmapItem;
    QString currentVersionStr;
    QString minVersionStr;
    QColor tagColor;
    WebBrowserDialog *howToUseDialog;
    WebBrowserDialog *aboutDialog;

    explicit MainWindow(QString workingDirectory="",QWidget *parent = 0);
    ~MainWindow();
    void removeAllItemsFromSceneExceptPixmapItem();

    static QString currentTagString();
    static QColor currentTagColor();
    static bool isRectangularSelection();
    static bool isPolygonalSelection();
    static QString reverseString(QString in);
    static void trimAllStrings(QStringList &list);
    static bool getColorFromHexString(const char *str, uint32_t &color);

public slots:
    void setWorkingDirectoryBtnClicked();
    void workingDirDialogFileSelected(QString newPath);
    void colorDialogColorSelected(QColor newColor);
    void unselectItem();
    void selectItem(int index);
    void itemListSelectionChanged();
    void itemNameBoxEditingFinished();
    void deleteItemBtnClicked();
    void duplicateItemBtnClicked();
    void filterBtnClicked();
    void resetFilterBtnClicked();
    void resetZoomBtnClicked();
    void saveCurrentItem();
    void saveCurrentTags();
    void addNewItem(QString name,QImage *image);
    void filterItems(QString wildcard);
    void keyDownHandler(QKeyEvent *event);
    void itemGraphicsViewRectItemAdded(QGraphicsRectItem *rectItem);
    void itemGraphicsViewPathItemAdded(QGraphicsPathItem *pathItem);
    void linkActivated(QString target);

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
