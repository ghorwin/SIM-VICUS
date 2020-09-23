/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_plot_canvas.h"
#include "qwt_painter.h"
#include "qwt_math.h"
#include "qwt_plot.h"

#ifndef QWT_NO_OPENGL

#if QT_VERSION < 0x050000
#define FBO_OPENGL 0
#else
#define FBO_OPENGL 1
#endif

#if FBO_OPENGL
#include <qopenglcontext.h>
#include <qopenglframebufferobject.h>
#include <qopenglpaintdevice.h>

#if QT_VERSION >= 0x050100
#include <qoffscreensurface.h>
typedef QOffscreenSurface QwtPlotCanvasSurfaceGL;

#else
#include <qwindow.h>
class QwtPlotCanvasSurfaceGL: public QWindow
{
public:
    QwtPlotCanvasSurfaceGL() { setSurfaceType( QWindow::OpenGLSurface ); }
};
#endif

#else
#include <qglframebufferobject.h>
typedef QGLWidget QwtPlotCanvasSurfaceGL;
#endif

#endif // !QWT_NO_OPENGL

#include <qpainter.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qpaintengine.h>
#include <qevent.h>

class QwtPlotCanvas::PrivateData
{
public:
    PrivateData():
        paintAttributes( 0 ),
#ifndef QWT_NO_OPENGL
        surfaceGL( NULL ),
#endif
        backingStore( NULL )
    {
    }

    ~PrivateData()
    {
        delete backingStore;

#ifndef QWT_NO_OPENGL
        delete surfaceGL;
#endif
    }

    QwtPlotCanvas::PaintAttributes paintAttributes;

#ifndef QWT_NO_OPENGL
    QwtPlotCanvasSurfaceGL *surfaceGL;
#endif

    QPixmap *backingStore;
};

/*! 
  \brief Constructor

  \param plot Parent plot widget
  \sa QwtPlot::setCanvas()
*/
QwtPlotCanvas::QwtPlotCanvas( QwtPlot *plot ):
    QFrame( plot ),
    QwtPlotAbstractCanvas( this )
{
    d_data = new PrivateData;

    setPaintAttribute( QwtPlotCanvas::BackingStore, true );
    setPaintAttribute( QwtPlotCanvas::Opaque, true );
    setPaintAttribute( QwtPlotCanvas::HackStyledBackground, true );
}

//! Destructor
QwtPlotCanvas::~QwtPlotCanvas()
{
    delete d_data;
}

/*!
  \brief Changing the paint attributes

  \param attribute Paint attribute
  \param on On/Off

  \sa testPaintAttribute(), backingStore()
*/
void QwtPlotCanvas::setPaintAttribute( PaintAttribute attribute, bool on )
{
    if ( bool( d_data->paintAttributes & attribute ) == on )
        return;

    if ( on )
        d_data->paintAttributes |= attribute;
    else
        d_data->paintAttributes &= ~attribute;

    switch ( attribute )
    {
        case BackingStore:
        {
            if ( on )
            {
                if ( d_data->backingStore == NULL )
                    d_data->backingStore = new QPixmap();

                if ( isVisible() )
                {
#if QT_VERSION >= 0x050000
                    *d_data->backingStore = grab( rect() );
#else
                    *d_data->backingStore = 
                        QPixmap::grabWidget( this, rect() );
#endif
                }
            }
            else
            {
                delete d_data->backingStore;
                d_data->backingStore = NULL;
            }
            break;
        }
        case Opaque:
        {
            if ( on )
                setAttribute( Qt::WA_OpaquePaintEvent, true );

            break;
        }
        default:
        {
            break;
        }
    }
}

/*!
  Test whether a paint attribute is enabled

  \param attribute Paint attribute
  \return true, when attribute is enabled
  \sa setPaintAttribute()
*/
bool QwtPlotCanvas::testPaintAttribute( PaintAttribute attribute ) const
{
    return d_data->paintAttributes & attribute;
}

//! \return Backing store, might be null
const QPixmap *QwtPlotCanvas::backingStore() const
{
    return d_data->backingStore;
}

//! Invalidate the internal backing store
void QwtPlotCanvas::invalidateBackingStore()
{
    if ( d_data->backingStore )
        *d_data->backingStore = QPixmap();
}

/*!
  Qt event handler for QEvent::PolishRequest and QEvent::StyleChange

  \param event Qt Event
  \return See QFrame::event()
*/
bool QwtPlotCanvas::event( QEvent *event )
{
    if ( event->type() == QEvent::PolishRequest ) 
    {
        if ( testPaintAttribute( QwtPlotCanvas::Opaque ) )
        {
            // Setting a style sheet changes the 
            // Qt::WA_OpaquePaintEvent attribute, but we insist
            // on painting the background.
            
            setAttribute( Qt::WA_OpaquePaintEvent, true );
        }
    }

    if ( event->type() == QEvent::PolishRequest || 
        event->type() == QEvent::StyleChange )
    {
        updateStyleSheetInfo();
    }

    return QFrame::event( event );
}

/*!
  Paint event
  \param event Paint event
*/
void QwtPlotCanvas::paintEvent( QPaintEvent *event )
{
    QPainter painter( this );
    painter.setClipRegion( event->region() );

    if ( testPaintAttribute( QwtPlotCanvas::BackingStore ) &&
        d_data->backingStore != NULL )
    {
        QPixmap &bs = *d_data->backingStore;
        if ( bs.size() != size() * QwtPainter::devicePixelRatio( &bs ) )
        {
            bs = QwtPainter::backingStore( this, size() );

#ifndef QWT_NO_OPENGL
            if ( testPaintAttribute( OpenGLBuffer ) )
            {
                QPainter p( &bs );
                p.drawImage( 0, 0, toImageFBO( size() ) );
                p.end();
            }
            else
#endif
            if ( testAttribute(Qt::WA_StyledBackground) )
            {
                QPainter p( &bs );
                drawStyled( &p, testPaintAttribute( HackStyledBackground ) );
            }
            else
            {
                QPainter p;
                if ( borderRadius() <= 0.0 )
                {
                    QwtPainter::fillPixmap( this, bs );
                    p.begin( &bs );
                    drawCanvas( &p );
                }
                else
                {
                    p.begin( &bs );
                    drawUnstyled( &p );
                }

                if ( frameWidth() > 0 )
                    drawBorder( &p );
            }
        }

        painter.drawPixmap( 0, 0, *d_data->backingStore );
    }
    else
    {
#ifndef QWT_NO_OPENGL
        if ( testPaintAttribute( OpenGLBuffer ) )
        {
            painter.drawImage( 0, 0, toImageFBO( size() ) );
        }
        else
#endif
        if ( testAttribute(Qt::WA_StyledBackground ) )
        {
            if ( testAttribute( Qt::WA_OpaquePaintEvent ) )
            {
                drawStyled( &painter, testPaintAttribute( HackStyledBackground ) );
            }
            else
            {
                drawCanvas( &painter );
            }
        }
        else
        {
            if ( testAttribute( Qt::WA_OpaquePaintEvent ) )
            {
                if ( autoFillBackground() )
                {
                    fillBackground( &painter );
                    drawBackground( &painter );
                }
            }
            else
            {
                if ( borderRadius() > 0.0 )
                {
                    QPainterPath clipPath;
                    clipPath.addRect( rect() );
                    clipPath = clipPath.subtracted( borderPath( rect() ) );

                    painter.save();

                    painter.setClipPath( clipPath, Qt::IntersectClip );
                    fillBackground( &painter );
                    drawBackground( &painter );

                    painter.restore();
                }
            }

            drawCanvas( &painter );

            if ( frameWidth() > 0 ) 
                drawBorder( &painter );
        }
    }

    if ( hasFocus() && focusIndicator() == CanvasFocusIndicator )
        drawFocusIndicator( &painter );
}

/*!
  Draw the border of the plot canvas

  \param painter Painter
  \sa setBorderRadius()
*/
void QwtPlotCanvas::drawBorder( QPainter *painter )
{
#if QT_VERSION >= 0x040500
    if ( borderRadius() <= 0 )
    {
        drawFrame( painter );
        return;
    }
#endif

    QwtPlotAbstractCanvas::drawBorder( painter );
}

/*!
  Resize event
  \param event Resize event
*/
void QwtPlotCanvas::resizeEvent( QResizeEvent *event )
{
    QFrame::resizeEvent( event );
    updateStyleSheetInfo();
}

/*!
   Invalidate the paint cache and repaint the canvas
   \sa invalidatePaintCache()
*/
void QwtPlotCanvas::replot()
{
    invalidateBackingStore();

    if ( testPaintAttribute( QwtPlotCanvas::ImmediatePaint ) )
        repaint( contentsRect() );
    else
        update( contentsRect() );
}

/*!
   Calculate the painter path for a styled or rounded border

   When the canvas has no styled background or rounded borders
   the painter path is empty.

   \param rect Bounding rectangle of the canvas
   \return Painter path, that can be used for clipping
*/
QPainterPath QwtPlotCanvas::borderPath( const QRect &rect ) const
{
    return borderPath2( rect );
}

#ifndef QWT_NO_OPENGL

#define FIX_GL_TRANSLATION 0

QImage QwtPlotCanvas::toImageFBO( const QSize &size ) 
{
    const int numSamples = 4;

#if FBO_OPENGL

    if ( d_data->surfaceGL == NULL )
    {
        d_data->surfaceGL = new QwtPlotCanvasSurfaceGL();
        d_data->surfaceGL->create();
    }

    QOpenGLContext context;
    context.create();

    context.makeCurrent( d_data->surfaceGL );


    QOpenGLFramebufferObjectFormat fboFormat;
    fboFormat.setSamples(numSamples);
    QOpenGLFramebufferObject fbo( size, fboFormat );

    QOpenGLPaintDevice pd( size );

#else

    if ( d_data->surfaceGL == NULL )
    {
        QGLFormat format = QGLFormat::defaultFormat();
        format.setSampleBuffers( true );
        format.setSamples( numSamples );

        d_data->surfaceGL = new QwtPlotCanvasSurfaceGL( format );
    }

    d_data->surfaceGL->makeCurrent();

#if QT_VERSION >= 0x040600
    QGLFramebufferObjectFormat fboFormat;
    fboFormat.setSamples(numSamples);

    QGLFramebufferObject fbo( size, fboFormat );
#else
    QGLFramebufferObject fbo( size );
#endif
    QGLFramebufferObject &pd = fbo;

#endif

    QPainter painter( &pd );

    if ( testAttribute( Qt::WA_StyledBackground ) )
        drawStyled( &painter, testPaintAttribute( HackStyledBackground ) );
    else
        drawUnstyled( &painter );
    
    if ( frameWidth() > 0 )
        drawBorder( &painter );
    
    painter.end();

    QImage image = fbo.toImage();

#if QT_VERSION >= 0x050000
    image.setDevicePixelRatio( QwtPainter::devicePixelRatio( this ) );
#endif
    return image;
}

#else

QImage QwtPlotCanvas::toImageFBO( const QSize &)
{
    // will never be called
    return QImage();
}

#endif
