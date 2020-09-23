/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_PLOT_GLCANVAS_H
#define QWT_PLOT_GLCANVAS_H

#include "qwt_global.h"
#include "qwt_plot_abstract_canvas.h"
#include <qgl.h>

class QwtPlot;

/*!
  \brief An alternative canvas for a QwtPlot derived from QGLWidget
  
  QwtPlotGLCanvas implements the very basics to act as canvas
  inside of a QwtPlot widget. It might be extended to a full
  featured alternative to QwtPlotCanvas in a future version of Qwt.

  Even if QwtPlotGLCanvas is not derived from QFrame it imitates
  its API. When using style sheets it supports the box model - beside
  backgrounds with rounded borders.

  \sa QwtPlot::setCanvas(), QwtPlotCanvas, QwtPlotCanvas::OpenGLBuffer

  \note With Qt4 you might want to use the QPaintEngine::OpenGL paint engine
        ( see QGL::setPreferredPaintEngine() ). On a Linux test system 
        QPaintEngine::OpenGL2 shows very basic problems like translated
        geometries.

  \note Another way for getting hardware accelerated graphics is using
        an OpenGL offscreen buffer ( QwtPlotCanvas::OpenGLBuffer ) with QwtPlotCanvas.
        Performance is worse, than rendering straight to a QGLWidget, but is usually
        better integrated into a desktop application. 
*/
class QWT_EXPORT QwtPlotGLCanvas: public QGLWidget, public QwtPlotAbstractGLCanvas
{
    Q_OBJECT

    Q_PROPERTY( QFrame::Shadow frameShadow READ frameShadow WRITE setFrameShadow )
    Q_PROPERTY( QFrame::Shape frameShape READ frameShape WRITE setFrameShape )
    Q_PROPERTY( int lineWidth READ lineWidth WRITE setLineWidth )
    Q_PROPERTY( int midLineWidth READ midLineWidth WRITE setMidLineWidth )
    Q_PROPERTY( int frameWidth READ frameWidth )
    Q_PROPERTY( QRect frameRect READ frameRect DESIGNABLE false )

    Q_PROPERTY( double borderRadius READ borderRadius WRITE setBorderRadius )

public:
    explicit QwtPlotGLCanvas( QwtPlot * = NULL );
    explicit QwtPlotGLCanvas( const QGLFormat &, QwtPlot * = NULL );
    virtual ~QwtPlotGLCanvas();

    Q_INVOKABLE virtual void invalidateBackingStore();
    Q_INVOKABLE QPainterPath borderPath( const QRect & ) const;

    virtual bool event( QEvent * );

public Q_SLOTS:
    void replot();

protected:
    virtual void paintEvent( QPaintEvent * );

    virtual void initializeGL();
    virtual void paintGL();
    virtual void resizeGL( int width, int height );

private:
	virtual void clearBackingStore();

    class PrivateData;
    PrivateData *d_data;
};

#endif
