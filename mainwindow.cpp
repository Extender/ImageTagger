#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow *MainWindow::instance=0;

MainWindow::MainWindow(QString workingDirectory, QWidget *parent) :
    MainWindowEx(parent),
    ui(new Ui::MainWindow)
{
    instance=this;
    currentVersionStr=QString::number(CURRENT_VERSION,'f',1);
    minVersionStr=QString::number(MIN_VERSION,'f',1);

    QFile cf(QApplication::applicationDirPath()+"/DefaultTagColor.txt");
    if(cf.exists())
    {
        cf.open(QFile::ReadOnly);
        size_t s=cf.size();
        char *str=(char*)malloc(s+1);
        cf.read(str,s);
        cf.close();
        str[s]=0;
        uint32_t col;
        char *trimmed=text::trim(str);
        bool t=getColorFromHexString(trimmed,col);
        if(t)
            tagColor=QColor(col);
        else
            tagColor=QColor(DEFAULT_TAG_COLOR);
        free(str);
        free(trimmed);
    }
    else
        tagColor=QColor(DEFAULT_TAG_COLOR);

    srand(time(0));
    ui->setupUi(this);
    setWindowTitle(windowTitle()+" v"+currentVersionStr);

    // Keep this block here
    {
        scene=(GraphicsSceneEx*)ui->itemGraphicsView->scene(); // Scene already set during construction
        pixmapItem=new QGraphicsPixmapItem();
        scene->addItem(pixmapItem);
        ui->itemGraphicsView->setScene(scene);
        ui->itemGraphicsView->getCurrentTagStringFunction=&MainWindow::currentTagString;
        ui->itemGraphicsView->getCurrentTagColorFunction=&MainWindow::currentTagColor;
        connect(ui->itemGraphicsView,SIGNAL(rectItemAdded(QGraphicsRectItem*)),this,SLOT(itemGraphicsViewRectItemAdded(QGraphicsRectItem*)));
    }

    items=new vector<item_t>();
    selectedIndex=0; // Needed before first call to unselectItem()
    unselectItem();

    defaultFilters=new QStringList();
    defaultFilters->append("*.jpg");
    defaultFilters->append("*.jpeg");
    defaultFilters->append("*.png");
    defaultFilters->append("*.bmp");
    defaultFilters->append("*.gif");
    defaultFilters->append("*.tif");
    defaultFilters->append("*.tiff");

    QString initialDirPath=QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)+QString("/images/");
    QDir d(initialDirPath);
    if(!d.exists())
        QDir().mkdir(initialDirPath);

    workingDir="";
    workingDirDialog=new QFileDialog(this);
    workingDirDialog->setAcceptMode(QFileDialog::AcceptOpen);
    workingDirDialog->setFileMode(QFileDialog::Directory);
    workingDirDialog->setOption(QFileDialog::ShowDirsOnly);
    workingDirDialog->setWindowTitle("Select a directory...");
    connect(workingDirDialog,SIGNAL(fileSelected(QString)),this,SLOT(workingDirDialogFileSelected(QString)));

    colorDialog=new QColorDialog(tagColor,this);
    connect(colorDialog,SIGNAL(colorSelected(QColor)),this,SLOT(colorDialogColorSelected(QColor)));

    connect(this,SIGNAL(keyDownEx(QKeyEvent*)),this,SLOT(keyDownHandler(QKeyEvent*)));
    connect(ui->setTagColorBtn,SIGNAL(clicked(bool)),colorDialog,SLOT(exec()));
    connect(ui->browseWorkingDirectoryBtn,SIGNAL(clicked(bool)),workingDirDialog,SLOT(exec()));
    connect(ui->setWorkingDirectoryBtn,SIGNAL(clicked(bool)),this,SLOT(setWorkingDirectoryBtnClicked()));
    connect(ui->itemList,SIGNAL(itemSelectionChanged()),this,SLOT(itemListSelectionChanged()));
    connect(ui->itemNameBox,SIGNAL(editingFinished()),this,SLOT(itemNameBoxEditingFinished()));
    connect(ui->resetZoomBtn,SIGNAL(clicked(bool)),this,SLOT(resetZoomBtnClicked()));
    connect(ui->clearTagBtn,SIGNAL(clicked(bool)),ui->tagBox,SLOT(clear()));
    connect(ui->deleteItemBtn,SIGNAL(clicked(bool)),this,SLOT(deleteItemBtnClicked()));
    connect(ui->duplicateItemBtn,SIGNAL(clicked(bool)),this,SLOT(duplicateItemBtnClicked()));
    connect(ui->filterBtn,SIGNAL(clicked(bool)),this,SLOT(filterBtnClicked()));
    connect(ui->resetFilterBtn,SIGNAL(clicked(bool)),this,SLOT(resetFilterBtnClicked()));
    connect(ui->aboutLbl,SIGNAL(linkActivated(QString)),this,SLOT(linkActivated(QString)));
    connect(ui->howToUseLbl,SIGNAL(linkActivated(QString)),this,SLOT(linkActivated(QString)));

    bool hasWD=workingDirectory.length()>0;

    if(!hasWD)
    {
        QFile wdf(QApplication::applicationDirPath()+"/DefaultWorkingDirectory.txt");
        if(wdf.exists())
        {
            wdf.open(QFile::ReadOnly);
            size_t s=wdf.size();
            char *str=(char*)malloc(s+1);
            wdf.read(str,s);
            wdf.close();
            str[s]=0;
            QDir wd(QString(str).trimmed());
            if(wd.exists())
            {
                workingDirectory=wd.absolutePath();
                hasWD=true;
            }
            free(str);
        }
    }

    if(hasWD)
    {
        ui->workingDirectoryBox->setText(workingDirectory);
        setWorkingDirectoryBtnClicked();
    }

    howToUseDialog=0;
    aboutDialog=0;
}

MainWindow::~MainWindow()
{
    delete ui;
    int itemCount=items->size();
    for(int i=0;i<itemCount;i++)
        delete items->at(i);
    delete items;
    delete defaultFilters;
    delete workingDirDialog;
}

void MainWindow::removeAllItemsFromSceneExceptPixmapItem()
{
    QList<QGraphicsItem*> items=scene->items();
    vector<QGraphicsItem*> itemsToRemove;
    size_t s=items.size();
    for(size_t i=0;i<s;i++)
    {
        QGraphicsItem *item=items.at(i);
        if(item!=pixmapItem)
            itemsToRemove.push_back(item);
    }
    s=itemsToRemove.size();
    for(size_t i=0;i<s;i++)
    {
        QGraphicsItem *item=itemsToRemove.at(i);
        scene->removeItem(item);
        delete item;
    }
}

QString MainWindow::currentTagString()
{
    return instance->ui->tagBox->text();
}

QColor MainWindow::currentTagColor()
{
    return instance->tagColor;
}

QString MainWindow::reverseString(QString in)
{
    size_t l=in.length();
    QString out(l,0);
    size_t i=0;
    for(size_t r=l-1;r>=0;r--)
        out[i++]=in[r];
    return out;
}

bool MainWindow::getColorFromHexString(const char *str, uint32_t &color)
{
    int length=strlen(str);
    if(length<3)
        return false;

    if(str[0]=='#')
    {
        str++;
        length--;
    }
    else if(str[0]=='0'&&(str[1]=='x'||str[1]=='X'))
    {
        str+=2;
        length-=2;
    }

    if(length==8) // 0xAARRGGBB; A=255
    {
        char nStr[9];
        nStr[0]=str[0];
        nStr[1]=str[1];
        nStr[2]=str[2];
        nStr[3]=str[3];
        nStr[4]=str[4];
        nStr[5]=str[5];
        nStr[6]=str[6];
        nStr[7]=str[7];
        nStr[8]=0;
        text_t lengthPlaceholder;
        uint8_t *bytes=(uint8_t*)text::bytesFromHexString(nStr,lengthPlaceholder);
        uint32_t a=(uint32_t)bytes[0];
        uint32_t r=(uint32_t)bytes[1];
        uint32_t g=(uint32_t)bytes[2];
        uint32_t b=(uint32_t)bytes[3];
        color=getColor(a,r,g,b);
        free(bytes);
        return true;
    }
    else if(length==6) // 0xRRGGBB; A=255
    {
        char nStr[9];
        nStr[0]='F';
        nStr[1]='F';
        nStr[2]=str[0];
        nStr[3]=str[1];
        nStr[4]=str[2];
        nStr[5]=str[3];
        nStr[6]=str[4];
        nStr[7]=str[5];
        nStr[8]=0;
        text_t lengthPlaceholder;
        uint8_t *bytes=(uint8_t*)text::bytesFromHexString(nStr,lengthPlaceholder);
        uint32_t a=bytes[0];
        uint32_t r=bytes[1];
        uint32_t g=bytes[2];
        uint32_t b=bytes[3];
        color=getColor(a,r,g,b);
        free(bytes);
        return true;
    }
    else if(length==4) // 0xARGB
    {
        char nStr[9];
        nStr[0]=str[0];
        nStr[1]=str[0];
        nStr[2]=str[1];
        nStr[3]=str[1];
        nStr[4]=str[2];
        nStr[5]=str[2];
        nStr[6]=str[3];
        nStr[7]=str[3];
        nStr[8]=0;
        text_t lengthPlaceholder;
        uint8_t *bytes=(uint8_t*)text::bytesFromHexString(nStr,lengthPlaceholder);
        uint32_t a=bytes[0];
        uint32_t r=bytes[1];
        uint32_t g=bytes[2];
        uint32_t b=bytes[3];
        color=getColor(a,r,g,b);
        free(bytes);
        return true;
    }
    else if(length==3) // 0xRGB; A=255
    {
        char nStr[9];
        nStr[0]='F';
        nStr[1]='F';
        nStr[2]=str[0];
        nStr[3]=str[0];
        nStr[4]=str[1];
        nStr[5]=str[1];
        nStr[6]=str[2];
        nStr[7]=str[2];
        nStr[8]=0;
        text_t lengthPlaceholder;
        uint8_t *bytes=(uint8_t*)text::bytesFromHexString(nStr,lengthPlaceholder);
        uint32_t a=bytes[0];
        uint32_t r=bytes[1];
        uint32_t g=bytes[2];
        uint32_t b=bytes[3];
        color=getColor(a,r,g,b);
        free(bytes);
        return true;
    }
    else // Invalid format
        return false;
}

void MainWindow::setWorkingDirectoryBtnClicked()
{
    QString path=ui->workingDirectoryBox->text();
    if(path.length()==0)
    {
        QMessageBox::critical(this,"Error","No directory specified.");
        ui->workingDirectoryBox->setText(workingDir);
        return;
    }
    QDir d(path);
    if(!d.exists())
    {
        QMessageBox::critical(this,"Error","The specified directory does not exist.");
        ui->workingDirectoryBox->setText(workingDir);
        return;
    }

    path=path.replace("\\","/");
    if(!path.endsWith('/'))
        path+='/';

    workingDir=path;
    unselectItem();
    ui->itemList->clear();

    // Enumerate items in dir

    bool foundUnsupported=false;
    bool foundConflictingNames=false;
    QFileInfoList l=d.entryInfoList(*defaultFilters,QDir::Files);
    QStringList itemNames;
    for(QFileInfo i:l)
    {
        QString itemName=i.baseName();
        if(itemNames.contains(itemName))
        {
            foundConflictingNames=true;
            goto Skip;
        }
        {
            QString extension=i.suffix();
            QString afp=i.absoluteFilePath();
            QFile f(afp);
            f.open(QFile::ReadOnly);
            size_t s=f.size();
            QByteArray a=f.readAll();
            char *data=a.data();
            QString tagStr="";
            QFile t(afp+".taglist.json");
            if(t.exists())
            {
                t.open(QFile::ReadOnly);
                QTextStream str(&t);
                tagStr=str.readAll();
                t.close();
            }
            QStringList tagSpl=tagStr.split('\n',QString::SkipEmptyParts);
            char *extStr=strdup(extension.toUpper().toStdString().c_str());
            item_t item=new new_item_t(itemName,new QImage(QImage::fromData((uchar*)data,s,extStr)),extension);
            free(extStr);
            // item->tags already initialized during construction
            for(QString _str:tagSpl)
            {
                QString str=_str.trimmed();
                if(str.endsWith(','))
                    str.remove(str.length()-1,1);
                if(str.startsWith("\"minFileFormatVersion\""))
                {
                    int i2=str.lastIndexOf(':');
                    QString rem=str.mid(i2+1).trimmed();
                    float minVersion=rem.toFloat();
                    if(minVersion>CURRENT_VERSION)
                    {
                        foundUnsupported=true;
                        f.close();
                        goto Skip;
                    }
                }
                if(str.startsWith('[')&&str.endsWith(']'))
                {
                    // This is a tag.

                    // Remove [  ]
                    str.remove(0,1);
                    str.remove(str.length()-1,1);

                    int i=str.indexOf('"');
                    int i2=str.lastIndexOf('"');

                    if(i==-1||i2==-1)
                        continue;

                    QString name=str.mid(i+1,i2-i-1);
                    QString rem=str.mid(i2+1);

                    char *nameStr=strdup(name.toStdString().c_str());
                    QStringList spl2=rem.split(", ",QString::SkipEmptyParts);
                    QRectF rect(spl2.at(0).toFloat(),spl2.at(1).toFloat(),spl2.at(2).toFloat(),spl2.at(3).toFloat());
                    item->tags->push_back(new pair<char*,QRectF>(nameStr,rect));
                }
            }
            // Keep this here; the file may get skipped
            ui->itemList->addItem(itemName);
            itemNames.push_back(itemName);
            items->push_back(item);
            f.close();
        }
        Skip:
        continue;
    }
    if(foundUnsupported)
        QMessageBox::critical(this,"Error","Found unsupported items in working directory. Consider updating ImageTagger (http://github.com/Extender/ImageTagger/).");
    if(foundConflictingNames)
        QMessageBox::critical(this,"Error","Found conflicting files with the same base name. Some items were not added.");
}

void MainWindow::workingDirDialogFileSelected(QString newPath)
{
    ui->workingDirectoryBox->setText(newPath);
}

void MainWindow::colorDialogColorSelected(QColor newColor)
{
    // Change color of all tag rects and tag labels

    tagColor=newColor;
    QList<QGraphicsItem*> items=scene->items();
    size_t s=items.size();
    QBrush newBrush(newColor);
    QPen newPen(newBrush,1);
    for(size_t i=0;i<s;i++)
    {
        QGraphicsItem *item=items.at(i);
        int itemType=item->type();
        if(itemType==9 /*QGraphicsSimpleTextItem::type()*/)
        {
            ((QGraphicsSimpleTextItem*)item)->setBrush(newBrush);
        }
        else if(itemType==3 /*QGraphicsRectItem::type()*/)
        {
            ((QGraphicsRectItem*)item)->setPen(newPen);
        }
    }
}

void MainWindow::unselectItem()
{
    if(selectedIndex==-1)
        return;

    selectedIndex=-1;
    selectedItem=0;
    ui->itemNameBox->setText("");
    pixmapItem->setVisible(false);

    // Remove all items from scene except pixmap

    removeAllItemsFromSceneExceptPixmapItem();

    scene->update();
    list<QWidget*> frameChildren=ui->itemFrame->findChildren<QWidget*>().toStdList();
    for_each(frameChildren.begin(),frameChildren.end(),[](QWidget *w){w->setVisible(false);});
}

void MainWindow::selectItem(int index)
{
    selectedIndex=index;
    selectedItem=items->at(index);
    ui->itemNameBox->setText(selectedItem->name);
    pixmapItem->setPixmap(QPixmap::fromImage(*selectedItem->image));
    pixmapItem->setVisible(true);
    scene->setSceneRect(0,0,selectedItem->image->width(),selectedItem->image->height());
    scene->update();
    list<QWidget*> frameChildren=ui->itemFrame->findChildren<QWidget*>().toStdList();
    for_each(frameChildren.begin(),frameChildren.end(),[](QWidget *w){w->setVisible(true);});


    // Remove all current items from scene except pixmap

    removeAllItemsFromSceneExceptPixmapItem();

    // Reconstruct rect items and labels

    size_t s=selectedItem->tags->size();
    for(size_t i=0;i<s;i++)
    {
        pair<char*,QRectF> *p=selectedItem->tags->at(i);
        QGraphicsRectItem *rectItem=ui->itemGraphicsView->addItem(p->second,QString(p->first));
        rectItem->setData(Qt::UserRole+1,QVariant((qulonglong)p));
    }
}

void MainWindow::itemListSelectionChanged()
{
    QList<QListWidgetItem*> selectedItems=ui->itemList->selectedItems();
    if(selectedItems.count()==0)
    {
        unselectItem();
        return;
    }

    // Filter may be enabled; find index by name

    QString name=selectedItems.at(0)->text();
    int itemCount=items->size();
    int index=-1;
    for(int i=0;i<itemCount;i++)
    {
        if(name==items->at(i)->name)
        {
            index=i;
            break;
        }
    }
    selectItem(index);
}

void MainWindow::itemNameBoxEditingFinished()
{
    if(selectedIndex==-1)
        return;

    QString n=ui->itemNameBox->text();
    if(n==selectedItem->name)
        return;

    // Check if name already taken

    int itemCount=items->size();
    for(int i=0;i<itemCount;i++)
    {
        if(items->at(i)->name==n)
        {
            QMessageBox::critical(this,"Error","Name already taken.");
            ui->itemNameBox->setText(n);
            return;
        }
    }

    // Delete file with old name

    QFile::remove(workingDir+selectedItem->name+"."+selectedItem->extension);

    selectedItem->name=n;
    ui->itemList->selectedItems().at(0)->setText(n);
    saveCurrentItem();
}

void MainWindow::deleteItemBtnClicked()
{
    if(workingDir.length()==0)
    {
        QMessageBox::critical(this,"Error","No working directory specified.");
        return;
    }

    if(selectedIndex==-1)
        return;

    QFile f(workingDir+selectedItem->name+"."+selectedItem->extension);
    f.remove();
    items->erase(items->begin()+selectedIndex);
    ui->itemList->takeItem(ui->itemList->row(ui->itemList->selectedItems().at(0)));
    ui->itemList->repaint();
    // Make sure no item is selected
    QList <QListWidgetItem*> itemsNowSelected=ui->itemList->selectedItems();
    for_each(itemsNowSelected.begin(),itemsNowSelected.end(),[](QListWidgetItem *item){item->setSelected(false);});
    unselectItem();
}

void MainWindow::duplicateItemBtnClicked()
{
    if(workingDir.length()==0)
    {
        QMessageBox::critical(this,"Error","No working directory specified.");
        return;
    }

    addNewItem(QString("Copy of ")+selectedItem->name,new QImage(selectedItem->image->copy()));
}

void MainWindow::filterBtnClicked()
{
    filterItems(ui->filterBox->text());
}

void MainWindow::resetFilterBtnClicked()
{
    ui->filterBox->setText("");
    filterItems("");
}

void MainWindow::resetZoomBtnClicked()
{
    ui->itemGraphicsView->setZoomFactor(1.0);
}

void MainWindow::saveCurrentItem()
{
    QString path=workingDir+selectedItem->name+"."+selectedItem->extension;
    QFile f(path);
    f.open(QFile::WriteOnly|QFile::Truncate);
    QByteArray ba;
    QBuffer buff(&ba);
    buff.open(QIODevice::WriteOnly);
    char *extStr=strdup(selectedItem->extension.toUpper().toStdString().c_str());
    selectedItem->image->save(&buff,extStr,100);
    free(extStr);
    buff.close();
    size_t s=ba.size();
    f.write(ba.data(),s);
    f.close();

    saveCurrentTags();
}

void MainWindow::saveCurrentTags()
{
    QString path=workingDir+selectedItem->name+"."+selectedItem->extension+".taglist.json";
    QString tagStr="{\n\t\"note\": \"This file is auto-generated. Modifying its structure will cause ImageTagger to stop working.\",\n\t\"fileFormatVersion\": "+currentVersionStr+",\n\t\"minFileFormatVersion\": "+minVersionStr+",\n\t\"tags\":\n\t[";
    size_t tc=selectedItem->tags->count();
    for(size_t i=0;i<tc;i++)
    {
        pair<char*,QRectF> *tag=selectedItem->tags->at(i);
        QRectF r=tag->second;
        char *escStr=text::escapeDoubleQuotationMarks(tag->first);
        tagStr+=QString(i>0?",":"")+"\n\t\t[\""+escStr+"\", "+num((float)r.x())+", "+num((float)r.y())+", "+num((float)r.width())+", "+num((float)r.height())+"]";
        free(escStr);
    }

    tagStr+="\n\t]\n}";
    char *str=strdup(tagStr.toStdString().c_str());
    QFile t(path);
    t.open(QFile::WriteOnly|QFile::Truncate);
    t.write(str,strlen(str));
    t.close();
    free(str);
}

void MainWindow::addNewItem(QString name, QImage *image)
{
    item_t item=new new_item_t(name,image);

    items->push_back(item);
    QListWidgetItem *newItem=new QListWidgetItem(name,ui->itemList);
    ui->itemList->addItem(newItem);
    ui->itemList->clearSelection();
    newItem->setSelected(true);

    // Create file
    saveCurrentItem();
}

void MainWindow::filterItems(QString wildcard)
{
    unselectItem();
    ui->itemList->clear();
    int itemCount=items->size();
    if(wildcard.length()==0)
    {
        for(int i=0;i<itemCount;i++)
            ui->itemList->addItem(items->at(i)->name);
        return;
    }

    char *pattern=text::duplicateString((QString("*")+QString(wildcard)+QString("*")).toStdString().c_str()); // Must be duplicated
    for(int i=0;i<itemCount;i++)
    {
        item_t item=items->at(i);
        char *name=strdup(item->name.toStdString().c_str()); // Must be duplicated
        if(text::matchWildcard(name,pattern,true))
            ui->itemList->addItem(item->name);
        free(name);
    }
    free(pattern);
}

void MainWindow::keyDownHandler(QKeyEvent *event)
{
    if(event->key()==Qt::Key_Delete)
    {
        QGraphicsItem *item=ui->itemGraphicsView->selectedItem;
        if(item==0)
            return;

        QGraphicsSimpleTextItem *textItem;
        QGraphicsRectItem *rectItem;

        int itemType=item->type();
        if(itemType==9 /*QGraphicsSimpleTextItem::type()*/)
        {
            textItem=(QGraphicsSimpleTextItem*)item;
            rectItem=(QGraphicsRectItem*)item->data(Qt::UserRole).toULongLong();
        }
        else if(itemType==3 /*QGraphicsRectItem::type()*/)
        {
            textItem=(QGraphicsSimpleTextItem*)item->data(Qt::UserRole).toULongLong();
            rectItem=(QGraphicsRectItem*)item;
        }
        else
            return;

        selectedItem->tags->removeOne((std::pair<char*,QRectF>*)rectItem->data(Qt::UserRole+1).toULongLong()); // Do not use removeAt, as the indexes may shift

        scene->removeItem(textItem);
        scene->removeItem(rectItem);

        delete textItem;
        delete rectItem;

        saveCurrentTags();
    }
}

void MainWindow::itemGraphicsViewRectItemAdded(QGraphicsRectItem *rectItem)
{
    char *tag=strdup(ui->tagBox->text().toStdString().c_str());
    std::pair<char*,QRectF> *p=new std::pair<char*,QRectF>(tag,rectItem->rect());
    selectedItem->tags->push_back(p);
    rectItem->setData(Qt::UserRole+1,QVariant((qulonglong)p));
    saveCurrentTags();
    // Do not free "tag"
}

void MainWindow::linkActivated(QString target)
{
    if(target=="howToUse")
    {
        if(howToUseDialog==0)
        {
            QFile sourceFile(":/res/HowToUse.html");
            sourceFile.open(QFile::ReadOnly);
            QByteArray ba=sourceFile.readAll();
            QTextStream st(&ba);
            QString source=st.readAll();
            sourceFile.close();
            howToUseDialog=new WebBrowserDialog("How to use ImageTagger",source.replace("%version%",currentVersionStr),this);
        }
        howToUseDialog->show();
        howToUseDialog->activateWindow();
    }
    else if(target=="about")
    {
        if(aboutDialog==0)
        {
            QFile sourceFile(":/res/About.html");
            sourceFile.open(QFile::ReadOnly);
            QByteArray ba=sourceFile.readAll();
            QTextStream st(&ba);
            QString source=st.readAll();
            sourceFile.close();
            aboutDialog=new WebBrowserDialog("About ImageTagger",source.replace("%version%",currentVersionStr),this);
        }
        aboutDialog->show();
        aboutDialog->activateWindow();
    }
}
