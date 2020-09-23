#include "legenditemmover.h"
#include <qwt_plot.h>
#include <qwt_plot_canvas.h>
#include <qwt_scale_map.h>
#include <qwt_plot_legenditem.h>
#include <qevent.h>

// The overlay shows the content of the legend item while it is
// is being dragged across the screen
class Overlay: public QwtWidgetOverlay
{
public:
    Overlay( QWidget *parent, LegendItemMover *editor ):
        QwtWidgetOverlay( parent ),
        d_mover( editor )
    {
        setMaskMode( QwtWidgetOverlay::MaskHint );
    }

protected:
    virtual void drawOverlay( QPainter *painter ) const
    {
        d_mover->drawOverlay( painter );
    }

    virtual QRegion maskHint() const
    {
        return d_mover->maskHint();
    }

private:
    LegendItemMover *d_mover;
};


// *** LegendItemMover ***

LegendItemMover::LegendItemMover( QwtPlot* plot ):
    QObject( plot ),
    d_overlay( NULL )
{
    plot->canvas()->installEventFilter( this );
}


LegendItemMover::~LegendItemMover()
{
    delete d_overlay;
}

QwtPlot *LegendItemMover::plot()
{
    return qobject_cast<QwtPlot *>( parent() );
}

const QwtPlot *LegendItemMover::plot() const
{
    return qobject_cast<const QwtPlot *>( parent() );
}

bool LegendItemMover::eventFilter( QObject* object, QEvent* event )
{
    QwtPlot *plot = qobject_cast<QwtPlot *>( parent() );
    if ( plot && object == plot->canvas() )
    {
        switch( event->type() )
        {
            case QEvent::MouseButtonPress:
            {
                const QMouseEvent* mouseEvent =
                    dynamic_cast<QMouseEvent* >( event );

                if ( d_overlay == NULL &&
                    mouseEvent->button() == Qt::LeftButton  )
                {
                    const bool accepted = pressed( mouseEvent->pos() );
                    if ( accepted )
                    {
                        d_overlay = new Overlay( plot->canvas(), this );

                        d_overlay->updateOverlay();
                        d_overlay->show();
                        return true; // prevent event to fall through to PlotZoomer and start a zoom drag-rect
                    }
                }

                break;
            }

            case QEvent::MouseMove:
            {
                if ( d_overlay )
                {
                    const QMouseEvent* mouseEvent =
                        dynamic_cast< QMouseEvent* >( event );

                    const bool accepted = moved( mouseEvent->pos() );
                    if ( accepted )
                        d_overlay->updateOverlay();
                }

                break;
            }

            case QEvent::MouseButtonRelease:
            {
                const QMouseEvent* mouseEvent =
                    static_cast<QMouseEvent* >( event );

                if ( d_overlay && mouseEvent->button() == Qt::LeftButton )
                {
                    released( mouseEvent->pos() );

                    delete d_overlay;
                    d_overlay = NULL;
                }

                break;
            }
            default:
                break;
        }

        return false; // go on with next eventfilter in queue
    }

    // event not for canvas, do the regular stuff
    return QObject::eventFilter( object, event );
}

bool LegendItemMover::pressed( const QPoint& pos )
{
    d_legendItem = itemAt( pos );
    if ( d_legendItem )
    {
        d_currentPos = pos;

        // during dragging, the legend item anchor is always top left
        QRect itemR = d_legendItem->geometry( canvasRect() );
        d_legendItem->setAlignmentInCanvas(Qt::AlignLeft | Qt::AlignTop);
        // recalculate offset to top-left
        d_legendItem->setOffsetInCanvas(Qt::Horizontal, itemR.left()-canvasRect().left());
        d_legendItem->setOffsetInCanvas(Qt::Vertical, itemR.top()-canvasRect().top());
        setItemVisible( false );

        return true;
    }

    return false; // don't accept the position
}

bool LegendItemMover::moved( const QPoint& pos )
{
    if ( plot() == NULL )
        return false;

    // moved distance
    const double dx = pos.x() - d_currentPos.x();
    const double dy = pos.y() - d_currentPos.y();

    // get new coordinate with respect to top/left
    // -1 is for the frame width
    double newX = std::max(-1.0, dx + d_legendItem->offsetInCanvas( Qt::Horizontal ));
    double newY = std::max(-1.0, dy + d_legendItem->offsetInCanvas( Qt::Vertical ));

    QRectF cr = canvasRect();
    QRect itemGeometry = d_legendItem->geometry(cr);
    newX = std::min(newX, cr.right() - itemGeometry.width());
    newY = std::min(newY, cr.bottom() - itemGeometry.height());

    d_legendItem->setOffsetInCanvas( Qt::Horizontal, newX);
    d_legendItem->setOffsetInCanvas( Qt::Vertical, newY);

    d_currentPos = pos;

    return true;
}

void LegendItemMover::released( const QPoint& pos )
{
    Q_UNUSED( pos );

    if ( d_legendItem  )
    {

        // get canvas geometry
        QRectF canvasR = canvasRect();
        int cw = canvasR.width();
        int ch = canvasR.height();

        // compute position rectangle of legend
        int xpos = d_legendItem->offsetInCanvas( Qt::Horizontal );
        int ypos = d_legendItem->offsetInCanvas( Qt::Vertical );
        QRect itemR = d_legendItem->geometry( canvasR );
        // move itemR so that is is aligned with offset (mind: canvas has 1,1 shift)
        itemR.setLeft(xpos);
        itemR.setTop(ypos);

        // determine suitable alignment in canvas, based on currently given offset
        // and legendItem geometry, compute distance to all corners and remember
        // which one is closest

        // calc offsets of legend rect to all sides, clipped to ensure legend remains visible
        int offsetL = std::min(xpos, cw-itemR.width());
        int offsetR = std::max(0, cw-itemR.right());
        int offsetT = std::min(ypos, ch-itemR.height());
        int offsetB = std::max(0, ch-itemR.bottom());

        int distances[4];
        int minDistIdx = 0; // store index to closest

        distances[0] =  offsetL*offsetL + offsetT*offsetT;
        if ( (distances[1] = offsetR*offsetR + offsetT*offsetT)    < distances[minDistIdx])	minDistIdx = 1;
        if ( (distances[2] = offsetR*offsetR + offsetB*offsetB)    < distances[minDistIdx])	minDistIdx = 2;
        if ( (distances[3] = offsetL*offsetL + offsetB*offsetB)    < distances[minDistIdx])	minDistIdx = 3;

        // set the alignment and position in the legend item
        switch (minDistIdx) {
          case 0 :
            d_legendItem->setAlignmentInCanvas(Qt::AlignLeft | Qt::AlignTop);
            d_legendItem->setOffsetInCanvas( Qt::Horizontal, offsetL);
            d_legendItem->setOffsetInCanvas( Qt::Vertical, offsetT);
            break;
          case 1 :
            d_legendItem->setAlignmentInCanvas(Qt::AlignRight | Qt::AlignTop);
            d_legendItem->setOffsetInCanvas( Qt::Horizontal, offsetR);
            d_legendItem->setOffsetInCanvas( Qt::Vertical, offsetT);
            break;
          case 2 :
            d_legendItem->setAlignmentInCanvas(Qt::AlignRight | Qt::AlignBottom);
            d_legendItem->setOffsetInCanvas( Qt::Horizontal, offsetR);
            d_legendItem->setOffsetInCanvas( Qt::Vertical, offsetB);
            break;
          case 3 :
            d_legendItem->setAlignmentInCanvas(Qt::AlignLeft | Qt::AlignBottom);
            d_legendItem->setOffsetInCanvas( Qt::Horizontal, offsetL);
            d_legendItem->setOffsetInCanvas( Qt::Vertical, offsetB);
            break;
        }

        setItemVisible( true );
        d_legendItem = NULL;
    }
}

QwtPlotLegendItem* LegendItemMover::itemAt( const QPoint& pos ) const
{
    const QwtPlot *plot = this->plot();
    if ( plot == NULL )
        return NULL;

    QwtPlotItemList items = plot->itemList();
    for ( int i = items.size() - 1; i >= 0; i-- )
    {
        QwtPlotItem *item = items[ i ];
        if ( item->isVisible() &&
            item->rtti() == QwtPlotItem::Rtti_PlotLegend )
        {
            QwtPlotLegendItem *legendItem = static_cast<QwtPlotLegendItem *>( item );

            const QRectF legendRect = legendItem->geometry( canvasRect() );
            if ( legendRect.contains( pos ) )
                return legendItem;
        }
    }

    return NULL;
}

QRectF LegendItemMover::canvasRect() const
{
    const QwtPlot *plot = this->plot();
    if ( plot )
        return plot->canvas()->contentsRect();

    return QRectF();
}

QRegion LegendItemMover::maskHint() const
{
    if ( d_legendItem ) {
        QRect r = d_legendItem->geometry( canvasRect() );
        // increase clip range by 1 to include bottom/right part of rectangle
        // Mind that a legend item with geometry 20x10 is in fact painted
        // using 21x11 pixels!
        r.setWidth(r.width()+1);
        r.setHeight(r.height()+1);
        return r;
    }

    return QRegion();
}

void LegendItemMover::drawOverlay( QPainter* painter ) const
{
    const QwtPlot *plot = this->plot();
    if ( plot == NULL || d_legendItem == NULL )
        return;

    const QwtScaleMap xMap = plot->canvasMap( d_legendItem->xAxis() );
    const QwtScaleMap yMap = plot->canvasMap( d_legendItem->yAxis() );

    painter->setRenderHint( QPainter::Antialiasing,
        d_legendItem->testRenderHint( QwtPlotItem::RenderAntialiased ) );

    qDebug() << d_legendItem->geometry( canvasRect() );
    d_legendItem->draw( painter, xMap, yMap, canvasRect() );
}

void LegendItemMover::setItemVisible( bool on )
{
    if ( plot() == NULL || d_legendItem == NULL || d_legendItem->isVisible() == on )
        return;

    const bool doAutoReplot = plot()->autoReplot();
    plot()->setAutoReplot( false );

    d_legendItem->setVisible( on );

    plot()->setAutoReplot( doAutoReplot );

    /*
      Avoid replot with a full repaint of the canvas.
      For special combinations - f.e. using the
      raster paint engine on a remote display -
      this makes a difference.
     */

    QwtPlotCanvas *canvas =
        qobject_cast<QwtPlotCanvas *>( plot()->canvas() );
    if ( canvas )
        canvas->invalidateBackingStore();

    plot()->canvas()->update( maskHint() );
}

