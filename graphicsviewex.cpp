#include "graphicsviewex.h"

GraphicsViewEx::GraphicsViewEx(QWidget *parent)
    : QGraphicsView(parent)
{
    setMouseTracking(true);
    setScene(new GraphicsSceneEx(this)); // We need to pass a parent!
    posMap=new QMap<QGraphicsItem*,QPoint>();
    defaultHandlers=true;
    zoomFactor=1.0;
    dragging=false;
    startPoint=QPoint();
    newItem=false;
    newRectItem=0;
    tagToUse="";
}

void GraphicsViewEx::wheelEvent(QWheelEvent *e)
{
    if(!defaultHandlers||newItem)
    {
        wheelEx(e);
        return;
    }
    if(dragging)
        dragging=false;
    if(e->delta()>0)
    {
        double newZoomFactor=zoomFactor+(zoomFactor/10);
        scale(newZoomFactor/zoomFactor,newZoomFactor/zoomFactor);
        zoomFactor=newZoomFactor;
    }
    else
    {
        double newZoomFactor=zoomFactor-(zoomFactor/10);
        scale(newZoomFactor/zoomFactor,newZoomFactor/zoomFactor);
        zoomFactor=newZoomFactor;
    }
    wheelEx(e);
}

void GraphicsViewEx::enterEvent(QEvent *e)
{
    QGraphicsView::enterEvent(e);
    mouseEnterEx(e);
}

void GraphicsViewEx::leaveEvent(QEvent *e)
{
    QGraphicsView::leaveEvent(e);
    if(dragging)
        dragging=false;
    if(newItem)
    {
        toggleNewItem();
    }
    mouseLeaveEx(e);
}

void GraphicsViewEx::mouseMoveEvent(QMouseEvent *e)
{
    QGraphicsView::mouseMoveEvent(e);
    if(!defaultHandlers)
        goto EmitEvent;
    if(newItem&&newRectItem!=nullptr)
    {
        // !!!!! No setPos here !!!!!
        int x=newRectItem->rect().x();
        int y=newRectItem->rect().y();
        QPoint scenePoint=QPointF(mapToScene(e->pos())).toPoint();
        int w=scenePoint.x()-x;
        int h=scenePoint.y()-y;
        if(w<0)
            w=0;
        if(h<0)
            h=0;
        newRectItem->setRect(x,y,w,h);
        goto EmitEvent;
    }
    if(dragging)
    {
        int x=offset.x()+(startPoint.x()-e->x());
        int y=offset.y()+(startPoint.y()-e->y());
        int xDelta=abs(x-offset.x());
        int yDelta=abs(y-offset.y());
        if(x<0)
            x=0;
        if(x>horizontalScrollBar()->maximum())
            x=horizontalScrollBar()->maximum();
        if(y<0)
            y=0;
        if(y>verticalScrollBar()->maximum())
            y=verticalScrollBar()->maximum();

        if(xDelta<10&&yDelta<10)
            goto EmitEvent;
        // Hack.
        horizontalScrollBar()->setValue(x);
        verticalScrollBar()->setValue(y);
    }
    EmitEvent:
    mouseMoveEx(e);
}

void GraphicsViewEx::mousePressEvent(QMouseEvent *e)
{
    QGraphicsItem *nSI=itemAt(e->pos());
    if(selectedItem!=nSI)
    {
        selectedItem=nSI;
        selectedItemChanged(selectedItem);
    }
    QGraphicsView::mousePressEvent(e);
    if(e->button()&Qt::RightButton)
    {
        tagToUse=getCurrentTagStringFunction();
        if(tagToUse.length()>0)
        {
            newItem=true;
        }
    }
    if(newItem)
    {
        // !!!!! No setPos here !!!!!
        newRectItem=new QGraphicsRectItem();
        newRectItem->setPen(QPen(QBrush(getCurrentTagColorFunction()),1));
        QPointF scenePoint=mapToScene(e->pos());
        scene()->addItem(newRectItem);
        newRectItem->setRect(scenePoint.x(),scenePoint.y(),0,0);
        goto SkipDragActions;
    }
    if(zoomFactor*scene()->width()<geometry().width()&&zoomFactor*scene()->height()<geometry().height()) // geometry: boundaries
        goto SkipDragActions;
    dragging=true;
    startPoint=e->pos();
    posMap->clear();
    offset=QPoint(horizontalScrollBar()->value(),verticalScrollBar()->value());
    SkipDragActions:
    mouseDownEx(e);
}

void GraphicsViewEx::mouseReleaseEvent(QMouseEvent *e)
{
    QGraphicsView::mouseReleaseEvent(e);
    if(newItem)
    {
        toggleNewItem();
        scene()->removeItem(newRectItem);

        QRectF nIR=newRectItem->rect();

        QGraphicsRectItem *substitute=addItem(nIR,tagToUse);

        rectItemAdded(substitute);
        delete newRectItem;
        newRectItem=0; // Tested, works. Do not use new QGraphicsRectItem(newRectItem), this item wouldn't be in the scene's items.
    }
    dragging=false;
    mouseUpEx(e);
}

void GraphicsViewEx::mouseDoubleClickEvent(QMouseEvent *e)
{
    QGraphicsView::mouseDoubleClickEvent(e);
    mouseDoubleClickEx(e);
}

void GraphicsViewEx::dropEvent(QDropEvent *e)
{
    dropEx(e);
}

void GraphicsViewEx::toggleNewItem()
{
    if(newItem)
    {
        newItem=false;
        setCursor(QCursor(Qt::ArrowCursor));
        if(newRectItem!=0)
            scene()->removeItem(newRectItem);
    }
    else
    {
        newItem=true;
        if(dragging)
            dragging=false;
        setCursor(QCursor(Qt::CrossCursor));
    }
}

void GraphicsViewEx::setZoomFactor(double newZoomFactor)
{
    scale(newZoomFactor/zoomFactor,newZoomFactor/zoomFactor);
    zoomFactor=newZoomFactor;
}

void GraphicsViewEx::resetZoom()
{
    setZoomFactor(1.0);
}

QGraphicsRectItem *GraphicsViewEx::addItem(QRectF rect, QString tag)
{
    QGraphicsRectItem *rectItem=new QGraphicsRectItem();
    QBrush brush(getCurrentTagColorFunction());

    QGraphicsSimpleTextItem *textItem=new QGraphicsSimpleTextItem(tag);
    textItem->setData(Qt::UserRole,QVariant((qulonglong)rectItem));
    textItem->setPos(QPoint(rect.x()+5,rect.y()+5));
    textItem->setFont(QFont(textItem->font().family(),12));
    textItem->setBrush(brush);
    textItem->setParentItem(rectItem);
    scene()->addItem(textItem);

    rectItem->setData(Qt::UserRole,QVariant((qulonglong)textItem));
    rectItem->setRect(rect);
    rectItem->setPen(QPen(brush,1));
    scene()->addItem(rectItem);
    return rectItem;
}
