/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_CLIPPER_H
#define QWT_CLIPPER_H

#include "qwt_global.h"
#include "qwt_interval.h"
#include <qpolygon.h>
#include <qvector.h>

class QRect;
class QRectF;

/*!
  \brief Some clipping algorithms
*/

class QWT_EXPORT QwtClipper
{
public:
    static void clipPolygon( const QRect &,  
        QPolygon &, bool closePolygon = false );
    
    static void clipPolygon( const QRectF &, 
        QPolygon &, bool closePolygon = false );
    
    static void clipPolygonF( const QRectF &, 
        QPolygonF &, bool closePolygon = false );

    static QPolygon clippedPolygon( const QRect &, 
        const QPolygon &, bool closePolygon = false );

    static QPolygon clippedPolygon( const QRectF &, 
        const QPolygon &, bool closePolygon = false );

    static QPolygonF clippedPolygonF( const QRectF &, 
        const QPolygonF &, bool closePolygon = false );

    static QVector<QwtInterval> clipCircle(
        const QRectF &, const QPointF &, double radius );
};

#endif
