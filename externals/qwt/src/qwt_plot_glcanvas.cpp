/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_plot_glcanvas.h"
#include "qwt_plot.h"
#include "qwt_painter.h"
#include <qevent.h>
#include <qglframebufferobject.h>

class QwtPlotGLCanvas::PrivateData
{
public:
    PrivateData():
        fboDirty( true ),
        fbo( NULL )
    {
    }

    ~PrivateData()
    {
        delete fbo;
    }

    bool fboDirty;
    QGLFramebufferObject* fbo;
};

class QwtPlotGLCanvasFormat: public QGLFormat
{
public:
    QwtPlotGLCanvasFormat():
        QGLFormat( QGLFormat::defaultFormat() )
    {
        setSampleBuffers( true );
    }
};

/*! 
  \brief Constructor

  \param plot Parent plot widget
  \sa QwtPlot::setCanvas()
*/
QwtPlotGLCanvas::QwtPlotGLCanvas( QwtPlot *plot ):
    QGLWidget( QwtPlotGLCanvasFormat(), plot ),
    QwtPlotAbstractGLCanvas( this )
{
    d_data = new PrivateData;
#if 1
    setAttribute( Qt::WA_OpaquePaintEvent, true );
#endif
}

QwtPlotGLCanvas::QwtPlotGLCanvas( const QGLFormat &format, QwtPlot *plot ):
    QGLWidget( format, plot ),
    QwtPlotAbstractGLCanvas( this )
{
    d_data = new PrivateData;
#if 1
    setAttribute( Qt::WA_OpaquePaintEvent, true );
#endif
}

//! Destructor
QwtPlotGLCanvas::~QwtPlotGLCanvas()
{
    delete d_data;
}

/*!
  Paint event

  \param event Paint event
  \sa QwtPlot::drawCanvas()
*/
void QwtPlotGLCanvas::paintEvent( QPaintEvent *event )
{
    QGLWidget::paintEvent( event );
}

/*!
  Qt event handler for QEvent::PolishRequest and QEvent::StyleChange
  \param event Qt Event
  \return See QGLWidget::event()
*/
bool QwtPlotGLCanvas::event( QEvent *event )
{
    const bool ok = QGLWidget::event( event );

    if ( event->type() == QEvent::PolishRequest ||
        event->type() == QEvent::StyleChange )
    {
        // assuming, that we always have a styled background
        // when we have a style sheet

        setAttribute( Qt::WA_StyledBackground,
            testAttribute( Qt::WA_StyleSheet ) );
    }

    return ok;
}

void QwtPlotGLCanvas::replot()
{
    QwtPlotAbstractGLCanvas::replot();
}

void QwtPlotGLCanvas::invalidateBackingStore()
{
    d_data->fboDirty = true;
}

void QwtPlotGLCanvas::clearBackingStore()
{
    delete d_data->fbo;
    d_data->fbo = NULL;
}

QPainterPath QwtPlotGLCanvas::borderPath( const QRect &rect ) const
{
    return borderPath2( rect );
}

void QwtPlotGLCanvas::initializeGL()
{
}

void QwtPlotGLCanvas::paintGL()
{
    const bool hasFocusIndicator =
        hasFocus() && focusIndicator() == CanvasFocusIndicator;

    QPainter painter;

#if QT_VERSION < 0x040600
    painter.begin( this );
    draw( &painter );
#else
    if ( testPaintAttribute( QwtPlotGLCanvas::BackingStore ) )
    {
        const qreal pixelRatio = QwtPainter::devicePixelRatio( NULL );
        const QRect rect( 0, 0, width() * pixelRatio, height() * pixelRatio );

        if ( hasFocusIndicator )
            painter.begin( this );

        if ( d_data->fbo )
        {
            if ( d_data->fbo->size() != rect.size() )
            {
                delete d_data->fbo;
                d_data->fbo = NULL;
            }
        }

        if ( d_data->fbo == NULL )
        {
            QGLFramebufferObjectFormat format;
            format.setSamples( 4 );
            format.setAttachment(QGLFramebufferObject::CombinedDepthStencil);

            d_data->fbo = new QGLFramebufferObject( rect.size(), format );
            d_data->fboDirty = true;
        }

        if ( d_data->fboDirty )
        {
            QPainter fboPainter( d_data->fbo );
            fboPainter.scale( pixelRatio, pixelRatio );
            draw( &fboPainter );
            fboPainter.end();

            d_data->fboDirty = false;
        }

        /*
            Why do we have this strange translation - but, anyway 
            QwtPlotGLCanvas in combination with scaling factor
            is not very likely to happen as using QwtPlotOpenGLCanvas
            usually makes more sense then.
         */

        QGLFramebufferObject::blitFramebuffer( NULL,
            rect.translated( 0, height() - rect.height() ), d_data->fbo, rect );
    }
    else
    {
        painter.begin( this );
        draw( &painter );
    }
#endif

    if ( hasFocusIndicator )
        drawFocusIndicator( &painter );
}

void QwtPlotGLCanvas::resizeGL( int, int )
{
    // nothing to do
}
