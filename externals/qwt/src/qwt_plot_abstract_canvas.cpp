/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_plot_abstract_canvas.h"
#include "qwt_plot.h"
#include "qwt_painter.h"
#include <qpainter.h>
#include <qdrawutil.h>
#include <qstyle.h>
#include <qstyleoption.h>

class QwtStyleSheetRecorder: public QwtNullPaintDevice
{
public:
    explicit QwtStyleSheetRecorder( const QSize &size ):
        d_size( size )
    {
    }

    virtual void updateState( const QPaintEngineState &state )
    {
        if ( state.state() & QPaintEngine::DirtyPen )
        {
            d_pen = state.pen();
        }
        if ( state.state() & QPaintEngine::DirtyBrush )
        {
            d_brush = state.brush();
        }
        if ( state.state() & QPaintEngine::DirtyBrushOrigin )
        {
            d_origin = state.brushOrigin();
        }
    }

    virtual void drawRects(const QRectF *rects, int count )
    {
        for ( int i = 0; i < count; i++ )
            border.rectList += rects[i];
    }

    virtual void drawRects(const QRect *rects, int count )
    {
        for ( int i = 0; i < count; i++ )
            border.rectList += rects[i];
    }

    virtual void drawPath( const QPainterPath &path )
    {
        const QRectF rect( QPointF( 0.0, 0.0 ), d_size );
        if ( path.controlPointRect().contains( rect.center() ) )
        {
            setCornerRects( path );
            alignCornerRects( rect );

            background.path = path;
            background.brush = d_brush;
            background.origin = d_origin;
        }
        else
        {
            border.pathList += path;
        }
    }

    void setCornerRects( const QPainterPath &path )
    {
        QPointF pos( 0.0, 0.0 );

        for ( int i = 0; i < path.elementCount(); i++ )
        {
            QPainterPath::Element el = path.elementAt(i); 
            switch( el.type )
            {
                case QPainterPath::MoveToElement:
                case QPainterPath::LineToElement:
                {
                    pos.setX( el.x );
                    pos.setY( el.y );
                    break;
                }
                case QPainterPath::CurveToElement:
                {
                    QRectF r( pos, QPointF( el.x, el.y ) );
                    clipRects += r.normalized();

                    pos.setX( el.x );
                    pos.setY( el.y );

                    break;
                }
                case QPainterPath::CurveToDataElement:
                {
                    if ( clipRects.size() > 0 )
                    {
                        QRectF r = clipRects.last();
                        r.setCoords( 
                            qMin( r.left(), el.x ),
                            qMin( r.top(), el.y ),
                            qMax( r.right(), el.x ),
                            qMax( r.bottom(), el.y )
                        );
                        clipRects.last() = r.normalized();
                    }
                    break;
                }
            }
        }
    }

protected:
    virtual QSize sizeMetrics() const
    {
        return d_size;
    }

private:
    void alignCornerRects( const QRectF &rect )
    {
        for ( int i = 0; i < clipRects.size(); i++ )
        {
            QRectF &r = clipRects[i];
            if ( r.center().x() < rect.center().x() )
                r.setLeft( rect.left() );
            else
                r.setRight( rect.right() );

            if ( r.center().y() < rect.center().y() )
                r.setTop( rect.top() );
            else
                r.setBottom( rect.bottom() );
        }
    }


public:
    QVector<QRectF> clipRects;

    struct Border
    {
        QList<QPainterPath> pathList;
        QList<QRectF> rectList;
        QRegion clipRegion;
    } border;

    struct Background
    {
        QPainterPath path;
        QBrush brush;
        QPointF origin;
    } background;

private:
    const QSize d_size;

    QPen d_pen;
    QBrush d_brush;
    QPointF d_origin;
};

static void qwtUpdateContentsRect( int fw, QWidget *canvas )
{
    canvas->setContentsMargins( fw, fw, fw, fw );
}

static void qwtDrawBackground( QPainter *painter, QWidget *canvas )
{
    painter->save();

    QPainterPath borderClip;
    
    ( void )QMetaObject::invokeMethod(
        canvas, "borderPath", Qt::DirectConnection,
        Q_RETURN_ARG( QPainterPath, borderClip ), Q_ARG( QRect, canvas->rect() ) );

    if ( !borderClip.isEmpty() )
        painter->setClipPath( borderClip, Qt::IntersectClip );

    const QBrush &brush = 
        canvas->palette().brush( canvas->backgroundRole() );

    if ( brush.style() == Qt::TexturePattern )
    {
        QPixmap pm( canvas->size() );
        QwtPainter::fillPixmap( canvas, pm );
        painter->drawPixmap( 0, 0, pm );
    }
    else if ( brush.gradient() )
    {
        QVector<QRect> rects;

        if ( brush.gradient()->coordinateMode() == QGradient::ObjectBoundingMode )
        {
            rects += canvas->rect();
        } 
        else 
        {
            rects = painter->clipRegion().rects();
        }

#if 1
        bool useRaster = false;

        if ( painter->paintEngine()->type() == QPaintEngine::X11 )
        {
            // Qt 4.7.1: gradients on X11 are broken ( subrects + 
            // QGradient::StretchToDeviceMode ) and horrible slow.
            // As workaround we have to use the raster paintengine.
            // Even if the QImage -> QPixmap translation is slow
            // it is three times faster, than using X11 directly

            useRaster = true;
        }
#endif
        if ( useRaster )
        {
            QImage::Format format = QImage::Format_RGB32;

            const QGradientStops stops = brush.gradient()->stops();
            for ( int i = 0; i < stops.size(); i++ )
            {
                if ( stops[i].second.alpha() != 255 )
                {
                    // don't use Format_ARGB32_Premultiplied. It's
                    // recommended by the Qt docs, but QPainter::drawImage()
                    // is horrible slow on X11.

                    format = QImage::Format_ARGB32;
                    break;
                }
            }
            
            QImage image( canvas->size(), format );

            QPainter p( &image );
            p.setPen( Qt::NoPen );
            p.setBrush( brush );

            p.drawRects( rects );

            p.end();

            painter->drawImage( 0, 0, image );
        }
        else
        {
            painter->setPen( Qt::NoPen );
            painter->setBrush( brush );

            painter->drawRects( rects );
        }
    }
    else
    {
        painter->setPen( Qt::NoPen );
        painter->setBrush( brush );

        painter->drawRects( painter->clipRegion().rects() );

    }

    painter->restore();
}

static inline void qwtDrawStyledBackground(
    QWidget *w, QPainter *painter )
{
    QStyleOption opt;
    opt.initFrom(w);
    w->style()->drawPrimitive( QStyle::PE_Widget, &opt, painter, w);
}

static QWidget *qwtBackgroundWidget( QWidget *w )
{
    if ( w->parentWidget() == NULL )
        return w;

    if ( w->autoFillBackground() )
    {
        const QBrush brush = w->palette().brush( w->backgroundRole() );
        if ( brush.color().alpha() > 0 )
            return w;
    }

    if ( w->testAttribute( Qt::WA_StyledBackground ) )
    {
        QImage image( 1, 1, QImage::Format_ARGB32 );
        image.fill( Qt::transparent );

        QPainter painter( &image );
        painter.translate( -w->rect().center() );
        qwtDrawStyledBackground( w, &painter );
        painter.end();

        if ( qAlpha( image.pixel( 0, 0 ) ) != 0 )
            return w;
    }

    return qwtBackgroundWidget( w->parentWidget() );
}

static void qwtFillBackground( QPainter *painter,
    QWidget *widget, const QVector<QRectF> &fillRects )
{
    if ( fillRects.isEmpty() )
        return;

    QRegion clipRegion;
    if ( painter->hasClipping() )
        clipRegion = painter->transform().map( painter->clipRegion() );
    else
        clipRegion = widget->contentsRect();

    // Try to find out which widget fills
    // the unfilled areas of the styled background

    QWidget *bgWidget = qwtBackgroundWidget( widget->parentWidget() );

    for ( int i = 0; i < fillRects.size(); i++ )
    {
        const QRect rect = fillRects[i].toAlignedRect();
        if ( clipRegion.intersects( rect ) )
        {
            QPixmap pm( rect.size() );
            QwtPainter::fillPixmap( bgWidget, pm, widget->mapTo( bgWidget, rect.topLeft() ) );
            painter->drawPixmap( rect, pm );
        }
    }
}

static void qwtFillBackground( QPainter *painter, QWidget *canvas )
{
    QVector<QRectF> rects;

    if ( canvas->testAttribute( Qt::WA_StyledBackground ) )
    {
        QwtStyleSheetRecorder recorder( canvas->size() );

        QPainter p( &recorder );
        qwtDrawStyledBackground( canvas, &p );
        p.end();

        if ( recorder.background.brush.isOpaque() )
            rects = recorder.clipRects;
        else
            rects += canvas->rect();
    }
    else
    {
        const double borderRadius = canvas->property( "borderRadius" ).toDouble();
        if ( borderRadius > 0.0 )
        {
            QSizeF sz( borderRadius, borderRadius );

            const QRectF r = canvas->rect();
            rects += QRectF( r.topLeft(), sz );
            rects += QRectF( r.topRight() - QPointF( borderRadius, 0 ), sz );
            rects += QRectF( r.bottomRight() - QPointF( borderRadius, borderRadius ), sz );
            rects += QRectF( r.bottomLeft() - QPointF( 0, borderRadius ), sz );
        }
    }

    qwtFillBackground( painter, canvas, rects);
}

static inline void qwtRevertPath( QPainterPath &path )
{
    if ( path.elementCount() == 4 )
    {
        QPainterPath::Element el0 = path.elementAt(0);
        QPainterPath::Element el3 = path.elementAt(3);

        path.setElementPositionAt( 0, el3.x, el3.y );
        path.setElementPositionAt( 3, el0.x, el0.y );
    }
}

static QPainterPath qwtCombinePathList( const QRectF &rect, 
    const QList<QPainterPath> &pathList )
{
    if ( pathList.isEmpty() )
        return QPainterPath();

    QPainterPath ordered[8]; // starting top left

    for ( int i = 0; i < pathList.size(); i++ )
    {
        int index = -1;
        QPainterPath subPath = pathList[i];

        const QRectF br = pathList[i].controlPointRect();
        if ( br.center().x() < rect.center().x() )
        {
            if ( br.center().y() < rect.center().y() )
            {
                if ( qAbs( br.top() - rect.top() ) < 
                    qAbs( br.left() - rect.left() ) )
                {
                    index = 1;
                }
                else
                {
                    index = 0;
                }
            }
            else
            {
                if ( qAbs( br.bottom() - rect.bottom() ) < 
                    qAbs( br.left() - rect.left() ) )
                {
                    index = 6;
                }
                else
                {
                    index = 7;
                }
            }

            if ( subPath.currentPosition().y() > br.center().y() )
                qwtRevertPath( subPath );
        }
        else
        {
            if ( br.center().y() < rect.center().y() )
            {
                if ( qAbs( br.top() - rect.top() ) < 
                    qAbs( br.right() - rect.right() ) )
                {
                    index = 2;
                }
                else
                {
                    index = 3;
                }
            }
            else
            {
                if ( qAbs( br.bottom() - rect.bottom() ) < 
                    qAbs( br.right() - rect.right() ) )
                {
                    index = 5;
                }
                else
                {
                    index = 4;
                }
            }
            if ( subPath.currentPosition().y() < br.center().y() )
                qwtRevertPath( subPath );
        }   
        ordered[index] = subPath;
    }

    for ( int i = 0; i < 4; i++ )
    {
        if ( ordered[ 2 * i].isEmpty() != ordered[2 * i + 1].isEmpty() )
        {
            // we don't accept incomplete rounded borders
            return QPainterPath();
        }
    }


    const QPolygonF corners( rect );

    QPainterPath path;
    //path.moveTo( rect.topLeft() );

    for ( int i = 0; i < 4; i++ )
    {
        if ( ordered[2 * i].isEmpty() )
        {
            path.lineTo( corners[i] );
        }
        else
        {
            path.connectPath( ordered[2 * i] );
            path.connectPath( ordered[2 * i + 1] );
        }
    }

    path.closeSubpath();

#if 0
    return path.simplified();
#else
    return path;
#endif
}

static QPainterPath qwtBorderPath( const QWidget *canvas, const QRect &rect ) 
{
    if ( canvas->testAttribute(Qt::WA_StyledBackground ) )
    {
        QwtStyleSheetRecorder recorder( rect.size() );

        QPainter painter( &recorder );

        QStyleOption opt;
        opt.initFrom( canvas );
        opt.rect = rect;
        canvas->style()->drawPrimitive( QStyle::PE_Widget, &opt, &painter, canvas );

        painter.end();

        if ( !recorder.background.path.isEmpty() )
            return recorder.background.path;

        if ( !recorder.border.rectList.isEmpty() )
            return qwtCombinePathList( rect, recorder.border.pathList );
    }
    else 
    {
        const double borderRadius = canvas->property( "borderRadius" ).toDouble();

        if ( borderRadius > 0.0 )
        {
            double fw2 = canvas->property( "frameWidth" ).toInt() * 0.5;
            QRectF r = QRectF(rect).adjusted( fw2, fw2, -fw2, -fw2 );

            QPainterPath path;
            path.addRoundedRect( r, borderRadius, borderRadius );
            return path;
        }
    }

    return QPainterPath();
}

class QwtPlotAbstractCanvas::PrivateData
{
public:
    PrivateData():
        focusIndicator( NoFocusIndicator ),
        borderRadius( 0 )
    {
        styleSheet.hasBorder = false;
    }

    FocusIndicator focusIndicator;
    double borderRadius;

    struct StyleSheet
    {
        bool hasBorder;
        QPainterPath borderPath;
        QVector<QRectF> cornerRects;
        
        struct StyleSheetBackground
        {
            QBrush brush;
            QPointF origin;
        } background;
        
    } styleSheet;

    QWidget *canvasWidget;
};

QwtPlotAbstractCanvas::QwtPlotAbstractCanvas( QWidget *canvasWidget )
{
    d_data = new PrivateData;
    d_data->canvasWidget = canvasWidget;

#ifndef QT_NO_CURSOR
    canvasWidget->setCursor( Qt::CrossCursor );
#endif

    canvasWidget->setAutoFillBackground( true );

    canvasWidget->setProperty( "lineWidth", 2 );
    canvasWidget->setProperty( "frameShadow", QFrame::Sunken );
    canvasWidget->setProperty( "frameShape", QFrame::Panel );
}

QwtPlotAbstractCanvas::~QwtPlotAbstractCanvas()
{
    delete d_data;
}

//! Return parent plot widget
QwtPlot *QwtPlotAbstractCanvas::plot()
{
    return qobject_cast<QwtPlot *>( d_data->canvasWidget->parent() );
}

//! Return parent plot widget
const QwtPlot *QwtPlotAbstractCanvas::plot() const
{
    return qobject_cast<const QwtPlot *>( d_data->canvasWidget->parent() );
}

/*!
  Set the focus indicator

  \sa FocusIndicator, focusIndicator()
*/
void QwtPlotAbstractCanvas::setFocusIndicator( FocusIndicator focusIndicator )
{
    d_data->focusIndicator = focusIndicator;
}

/*!
  \return Focus indicator

  \sa FocusIndicator, setFocusIndicator()
*/
QwtPlotAbstractCanvas::FocusIndicator QwtPlotAbstractCanvas::focusIndicator() const
{
    return d_data->focusIndicator;
}

/*!
  Draw the focus indication
  \param painter Painter
*/
void QwtPlotAbstractCanvas::drawFocusIndicator( QPainter *painter )
{
    const int margin = 1;

    QRect focusRect = d_data->canvasWidget->contentsRect();
    focusRect.setRect( focusRect.x() + margin, focusRect.y() + margin,
        focusRect.width() - 2 * margin, focusRect.height() - 2 * margin );

    QwtPainter::drawFocusRect( painter, d_data->canvasWidget, focusRect );
}

/*!
  Set the radius for the corners of the border frame

  \param radius Radius of a rounded corner
  \sa borderRadius()
*/
void QwtPlotAbstractCanvas::setBorderRadius( double radius )
{
    d_data->borderRadius = qMax( 0.0, radius );
}

/*!
  \return Radius for the corners of the border frame
  \sa setBorderRadius()
*/
double QwtPlotAbstractCanvas::borderRadius() const
{
    return d_data->borderRadius;
}

QPainterPath QwtPlotAbstractCanvas::borderPath2( const QRect &rect ) const
{
    return qwtBorderPath( canvasWidget(), rect );
}

/*!
  Draw the border of the canvas
  \param painter Painter
*/
void QwtPlotAbstractCanvas::drawBorder( QPainter *painter )
{
    const QWidget *w = canvasWidget();

    if ( d_data->borderRadius > 0 )
    {
        const int frameWidth = w->property( "frameWidth" ).toInt();
        if ( frameWidth > 0 )
        {
            const int frameShape = w->property( "frameShape" ).toInt();
            const int frameShadow = w->property( "frameShadow" ).toInt();

            const QRectF frameRect = w->property( "frameRect" ).toRect();

            QwtPainter::drawRoundedFrame( painter, frameRect,
                d_data->borderRadius, d_data->borderRadius,
                w->palette(), frameWidth, frameShape | frameShadow );
        }
    }
    else
    {
#if QT_VERSION >= 0x040500
        const int frameShape = w->property( "frameShape" ).toInt();
        const int frameShadow = w->property( "frameShadow" ).toInt();

#if QT_VERSION < 0x050000
        QStyleOptionFrameV3 opt;
#else
        QStyleOptionFrame opt;
#endif
        opt.init( w );

        opt.frameShape = QFrame::Shape( int( opt.frameShape ) | frameShape );

        switch (frameShape)
        {
            case QFrame::Box:
            case QFrame::HLine:
            case QFrame::VLine:
            case QFrame::StyledPanel:
            case QFrame::Panel:
            {
                opt.lineWidth = w->property( "lineWidth" ).toInt();
                opt.midLineWidth = w->property( "midLineWidth" ).toInt();
                break;
            }
            default:
            {
                opt.lineWidth = w->property( "frameWidth" ).toInt();
                break;
            }
        }

        if ( frameShadow == QFrame::Sunken )
            opt.state |= QStyle::State_Sunken;
        else if ( frameShadow == QFrame::Raised )
            opt.state |= QStyle::State_Raised;

        w->style()->drawControl(QStyle::CE_ShapedFrame, &opt, painter, w );
#else
        // TODO: do we really need Qt 4.4 ?
#endif
    }
}

void QwtPlotAbstractCanvas::drawBackground( QPainter *painter )
{
    qwtDrawBackground( painter, canvasWidget() );
}

void QwtPlotAbstractCanvas::fillBackground( QPainter *painter )
{
    qwtFillBackground( painter, canvasWidget() );
}

void QwtPlotAbstractCanvas::drawUnstyled( QPainter *painter )
{
    fillBackground( painter );

    QWidget *w = canvasWidget();

    if ( w->autoFillBackground() )
    {
        const QRect canvasRect = w->rect();

        painter->save();

        painter->setPen( Qt::NoPen );
        painter->setBrush( w->palette().brush( w->backgroundRole() ) );

        const QRect frameRect = w->property( "frameRect" ).toRect();
        if ( borderRadius() > 0.0 && ( canvasRect == frameRect ) )
        {
            const int frameWidth = w->property( "frameWidth" ).toInt();
            if ( frameWidth > 0 )
            {
                painter->setClipPath( borderPath2( canvasRect ) );
                painter->drawRect( canvasRect );
            }
            else
            {
                painter->setRenderHint( QPainter::Antialiasing, true );
                painter->drawPath( borderPath2( canvasRect ) );
            }
        }
        else
        {
            painter->drawRect( canvasRect );
        }

        painter->restore();
    }

    drawCanvas( painter );
}

void QwtPlotAbstractCanvas::drawStyled( QPainter *painter, bool hackStyledBackground )
{
    fillBackground( painter );

    if ( hackStyledBackground )
    {
        // Antialiasing rounded borders is done by
        // inserting pixels with colors between the 
        // border color and the color on the canvas,
        // When the border is painted before the plot items
        // these colors are interpolated for the canvas
        // and the plot items need to be clipped excluding
        // the anialiased pixels. In situations, where
        // the plot items fill the area at the rounded
        // borders this is noticeable.
        // The only way to avoid these annoying "artefacts"
        // is to paint the border on top of the plot items.

        if ( !d_data->styleSheet.hasBorder ||
            d_data->styleSheet.borderPath.isEmpty() )
        {
            // We have no border with at least one rounded corner
            hackStyledBackground = false;
        }
    }
    
    QWidget *w = canvasWidget();

    if ( hackStyledBackground )
    {
        painter->save();
        
        // paint background without border
        painter->setPen( Qt::NoPen );
        painter->setBrush( d_data->styleSheet.background.brush );
        painter->setBrushOrigin( d_data->styleSheet.background.origin );
        painter->setClipPath( d_data->styleSheet.borderPath );
        painter->drawRect( w->contentsRect() );

        painter->restore();

        drawCanvas( painter );

        // Now paint the border on top
        QStyleOptionFrame opt;
        opt.initFrom( w );
        w->style()->drawPrimitive( QStyle::PE_Frame, &opt, painter, w);
    }   
    else
    {
        QStyleOption opt;
        opt.initFrom( w );
        w->style()->drawPrimitive( QStyle::PE_Widget, &opt, painter, w );
    
        drawCanvas( painter );
    }   
}   

void QwtPlotAbstractCanvas::drawCanvas( QPainter *painter )
{
    QWidget *w = canvasWidget();

    painter->save();

    if ( !d_data->styleSheet.borderPath.isEmpty() )
    {
        painter->setClipPath(
            d_data->styleSheet.borderPath, Qt::IntersectClip );
    }
    else
    {
        if ( borderRadius() > 0.0 )
        {
            const QRect frameRect = w->property( "frameRect" ).toRect();
            painter->setClipPath( borderPath2( frameRect ), Qt::IntersectClip );
        }
        else
        {
            painter->setClipRect( w->contentsRect(), Qt::IntersectClip );
        }
    }

    QwtPlot *plot = qobject_cast< QwtPlot *>( w->parent() );
    if ( plot )
        plot->drawCanvas( painter );

    painter->restore();
}

//! Update the cached information about the current style sheet
void QwtPlotAbstractCanvas::updateStyleSheetInfo()
{
    QWidget *w = canvasWidget();

    if ( !w->testAttribute( Qt::WA_StyledBackground ) )
        return;

    QwtStyleSheetRecorder recorder( w->size() );

    QPainter painter( &recorder );

    QStyleOption opt;
    opt.initFrom(w);
    w->style()->drawPrimitive( QStyle::PE_Widget, &opt, &painter, w);

    painter.end();

    d_data->styleSheet.hasBorder = !recorder.border.rectList.isEmpty();
    d_data->styleSheet.cornerRects = recorder.clipRects;

    if ( recorder.background.path.isEmpty() )
    {
        if ( !recorder.border.rectList.isEmpty() )
        {
            d_data->styleSheet.borderPath =
                qwtCombinePathList( w->rect(), recorder.border.pathList );
        }
    }
    else
    {
        d_data->styleSheet.borderPath = recorder.background.path;
        d_data->styleSheet.background.brush = recorder.background.brush;
        d_data->styleSheet.background.origin = recorder.background.origin;
    }
}

QWidget* QwtPlotAbstractCanvas::canvasWidget()
{
    return d_data->canvasWidget;
}

const QWidget* QwtPlotAbstractCanvas::canvasWidget() const
{
    return d_data->canvasWidget;
}

class QwtPlotAbstractGLCanvas::PrivateData
{
public:
    PrivateData():
        frameStyle( QFrame::Panel | QFrame::Sunken),
        lineWidth( 2 ),
        midLineWidth( 0 )
    {
    }

    ~PrivateData()
    {
    }

    QwtPlotAbstractGLCanvas::PaintAttributes paintAttributes;

    int frameStyle;
    int lineWidth;
    int midLineWidth;
};

QwtPlotAbstractGLCanvas::QwtPlotAbstractGLCanvas( QWidget *canvasWidget ):
    QwtPlotAbstractCanvas( canvasWidget )
{
    d_data = new PrivateData;

    qwtUpdateContentsRect( frameWidth(), canvasWidget );
    d_data->paintAttributes = QwtPlotAbstractGLCanvas::BackingStore;
}

QwtPlotAbstractGLCanvas::~QwtPlotAbstractGLCanvas()
{
    delete d_data;
}

/*!
  \brief Changing the paint attributes

  \param attribute Paint attribute
  \param on On/Off

  \sa testPaintAttribute()
*/
void QwtPlotAbstractGLCanvas::setPaintAttribute( PaintAttribute attribute, bool on )
{   
    if ( bool( d_data->paintAttributes & attribute ) == on )
        return;
    
    if ( on )
	{
        d_data->paintAttributes |= attribute;
	}
    else
	{
        d_data->paintAttributes &= ~attribute;

    	if ( attribute == BackingStore )
        	clearBackingStore();
	}
}

/*!
  Test whether a paint attribute is enabled

  \param attribute Paint attribute
  \return true, when attribute is enabled
  \sa setPaintAttribute()
*/  
bool QwtPlotAbstractGLCanvas::testPaintAttribute( PaintAttribute attribute ) const
{   
    return d_data->paintAttributes & attribute;
}

/*!
  Set the frame style

  \param style The bitwise OR between a shape and a shadow. 
  
  \sa frameStyle(), QFrame::setFrameStyle(), 
      setFrameShadow(), setFrameShape()
 */
void QwtPlotAbstractGLCanvas::setFrameStyle( int style )
{
    if ( style != d_data->frameStyle )
    {
        d_data->frameStyle = style;
        qwtUpdateContentsRect( frameWidth(), canvasWidget() );

        canvasWidget()->update();
    }
}

/*!
  \return The bitwise OR between a frameShape() and a frameShadow()
  \sa setFrameStyle(), QFrame::frameStyle()
 */
int QwtPlotAbstractGLCanvas::frameStyle() const
{
    return d_data->frameStyle;
}

/*!
  Set the frame shadow

  \param shadow Frame shadow
  \sa frameShadow(), setFrameShape(), QFrame::setFrameShadow()
 */
void QwtPlotAbstractGLCanvas::setFrameShadow( QFrame::Shadow shadow )
{
    setFrameStyle(( d_data->frameStyle & QFrame::Shape_Mask ) | shadow );
}

/*!
  \return Frame shadow
  \sa setFrameShadow(), QFrame::setFrameShadow()
 */
QFrame::Shadow QwtPlotAbstractGLCanvas::frameShadow() const
{
    return (QFrame::Shadow) ( d_data->frameStyle & QFrame::Shadow_Mask );
}

/*!
  Set the frame shape

  \param shape Frame shape
  \sa frameShape(), setFrameShadow(), QFrame::frameShape()
 */
void QwtPlotAbstractGLCanvas::setFrameShape( QFrame::Shape shape )
{
    setFrameStyle( ( d_data->frameStyle & QFrame::Shadow_Mask ) | shape );
}

/*!
  \return Frame shape
  \sa setFrameShape(), QFrame::frameShape()
 */
QFrame::Shape QwtPlotAbstractGLCanvas::frameShape() const
{
    return (QFrame::Shape) ( d_data->frameStyle & QFrame::Shape_Mask );
}

/*!
   Set the frame line width

   The default line width is 2 pixels.

   \param width Line width of the frame
   \sa lineWidth(), setMidLineWidth()
*/
void QwtPlotAbstractGLCanvas::setLineWidth( int width )
{
    width = qMax( width, 0 );
    if ( width != d_data->lineWidth )
    {
        d_data->lineWidth = qMax( width, 0 );
        qwtUpdateContentsRect( frameWidth(), canvasWidget() );
        canvasWidget()->update();
    }
}

/*!
  \return Line width of the frame
  \sa setLineWidth(), midLineWidth()
 */
int QwtPlotAbstractGLCanvas::lineWidth() const
{
    return d_data->lineWidth;
}

/*!
   Set the frame mid line width

   The default midline width is 0 pixels.

   \param width Midline width of the frame
   \sa midLineWidth(), setLineWidth()
*/
void QwtPlotAbstractGLCanvas::setMidLineWidth( int width )
{
    width = qMax( width, 0 );
    if ( width != d_data->midLineWidth )
    {
        d_data->midLineWidth = width;
        qwtUpdateContentsRect( frameWidth(), canvasWidget() );
        canvasWidget()->update();
    }
}

/*!
  \return Midline width of the frame
  \sa setMidLineWidth(), lineWidth()
 */ 
int QwtPlotAbstractGLCanvas::midLineWidth() const
{
    return d_data->midLineWidth;
}

/*!
  \return Frame width depending on the style, line width and midline width.
 */
int QwtPlotAbstractGLCanvas::frameWidth() const
{
    return ( frameStyle() != QFrame::NoFrame ) ? d_data->lineWidth : 0;
}

/*!
   Invalidate the paint cache and repaint the canvas
   \sa invalidatePaintCache()
*/
void QwtPlotAbstractGLCanvas::replot()
{
    invalidateBackingStore();
    
    QWidget *w = canvasWidget();
    if ( testPaintAttribute( QwtPlotAbstractGLCanvas::ImmediatePaint ) )
        w->repaint( w->contentsRect() );
    else
        w->update( w->contentsRect() );
}

//! \return The rectangle where the frame is drawn in.
QRect QwtPlotAbstractGLCanvas::frameRect() const
{
    const int fw = frameWidth();
    return canvasWidget()->contentsRect().adjusted( -fw, -fw, fw, fw );
}

void QwtPlotAbstractGLCanvas::draw( QPainter *painter )
{
#if FIX_GL_TRANSLATION
    if ( painter->paintEngine()->type() == QPaintEngine::OpenGL2 )
    {
        // work around a translation bug of QPaintEngine::OpenGL2
        painter->translate( 1, 1 );
    }
#endif

    if ( canvasWidget()->testAttribute( Qt::WA_StyledBackground ) )
        drawStyled( painter, true );
    else
        drawUnstyled( painter );

    if ( frameWidth() > 0 )
        drawBorder( painter );
}
