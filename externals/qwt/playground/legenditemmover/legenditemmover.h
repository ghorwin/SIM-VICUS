#ifndef LEGENDITEMMOVER_H
#define LEGENDITEMMOVER_H

#include <qobject.h>
#include <qregion.h>
#include <qpointer.h>
#include <qwt_widget_overlay.h>

class QwtPlot;
class QwtPlotLegendItem;
class QPainter;
class QPoint;

class LegendItemMover: public QObject
{
    Q_OBJECT

public:
    LegendItemMover( QwtPlot * );
    virtual ~LegendItemMover();

    const QwtPlot *plot() const;
    QwtPlot *plot();

    void drawOverlay( QPainter * ) const;
    QRegion maskHint() const;

    virtual bool eventFilter( QObject *, QEvent *);

private:
    bool pressed( const QPoint & );
    bool moved( const QPoint & );
    void released( const QPoint & );

    QRectF canvasRect() const;
    QwtPlotLegendItem* itemAt( const QPoint& ) const;

    void setItemVisible( bool on );

    QPointer<QwtWidgetOverlay> d_overlay;

    // Mouse positions
    QPointF d_currentPos;
    QwtPlotLegendItem* d_legendItem;
};

#endif
