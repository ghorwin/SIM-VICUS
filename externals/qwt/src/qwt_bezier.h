/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_BEZIER_H
#define QWT_BEZIER_H

#include "qwt_global.h"
#include <qpolygon.h>

/*!
  \brief An implementation of the de Casteljau’s Algorithm for interpolating
         Bézier curves

  The flatness criterion for terminating the subdivison is based on
  "Piecewise Linear Approximation of Bézier Curves" by
  Roger Willcocks ( http://www.rops.org )

  This article explains the maths behind in a very nice way:
  https://jeremykun.com/2013/05/11/bezier-curves-and-picasso
 */
class QWT_EXPORT QwtBezier
{
public:
    QwtBezier( double tolerance = 0.5 );
    ~QwtBezier();

    void setTolerance( double tolerance );
    double tolerance() const;
        
    QPolygonF toPolygon( const QPointF &p1, const QPointF &cp1,
        const QPointF &cp2, const QPointF &p2 ) const;

    void appendToPolygon( const QPointF &p1, const QPointF &cp1,
        const QPointF &cp2, const QPointF &p2, QPolygonF &polygon ) const;

    static QPointF pointAt( const QPointF &p1, const QPointF &cp1,
        const QPointF &cp2, const QPointF &p2, double t );

private:
    double m_tolerance;
    double m_flatness;
};

/*!
  \return Tolerance, that is used as criterion for the subdivisn
  \sa setTolerance()
 */
inline double QwtBezier::tolerance() const
{
    return m_tolerance;
}

/*!
  Find a point on a Bézier Curve

  \param p1 Start point
  \param cp1 First control point
  \param cp2 Second control point
  \param p2 End point
  \param t Parameter value, something between [0,1]

  \return Point on the curve
 */
inline QPointF QwtBezier::pointAt( const QPointF &p1,
    const QPointF &cp1, const QPointF &cp2, const QPointF &p2, double t )
{
    const double d1 = 3.0 * t;
    const double d2 = 3.0 * t * t;
    const double d3 = t * t * t;
    const double s  = 1.0 - t;

    const double x = (( s * p1.x() + d1 * cp1.x() ) * s + d2 * cp2.x() ) * s + d3 * p2.x();
    const double y = (( s * p1.y() + d1 * cp1.y() ) * s + d2 * cp2.y() ) * s + d3 * p2.y();

    return QPointF( x, y );
}

#endif
