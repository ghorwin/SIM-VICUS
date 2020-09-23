/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_spline_curve_fitter.h"
#include "qwt_spline_local.h"
#include "qwt_spline_parametrization.h"

//! Constructor
QwtSplineCurveFitter::QwtSplineCurveFitter():
    QwtCurveFitter( QwtCurveFitter::Path )
{
    d_spline = new QwtSplineLocal( QwtSplineLocal::Cardinal );
    d_spline->setParametrization( QwtSplineParametrization::ParameterUniform );
}

//! Destructor
QwtSplineCurveFitter::~QwtSplineCurveFitter()
{
    delete d_spline;
}

/*!
  Assign a spline

  The spline needs to be allocated by new and will be deleted
  in the destructor of the fitter.

  \param spline Spline
  \sa spline()
*/
void QwtSplineCurveFitter::setSpline( QwtSpline *spline )
{
    if ( d_spline == spline )
        return;

    delete d_spline;
    d_spline = spline;
}

/*!
  \return Spline
  \sa setSpline()
*/
const QwtSpline *QwtSplineCurveFitter::spline() const
{
    return d_spline;
}

/*!
  \return Spline
  \sa setSpline()
*/
QwtSpline *QwtSplineCurveFitter::spline() 
{
    return d_spline;
}

/*!
  Find a curve which has the best fit to a series of data points

  \param points Series of data points
  \return Fitted Curve

  \sa fitCurvePath()
*/
QPolygonF QwtSplineCurveFitter::fitCurve( const QPolygonF &points ) const
{
    const QPainterPath path = fitCurvePath( points );

    const QList<QPolygonF> subPaths = path.toSubpathPolygons();
    if ( subPaths.size() == 1 )
        subPaths.first();

    return QPolygonF();
}

/*!
  Find a curve path which has the best fit to a series of data points

  \param points Series of data points
  \return Fitted Curve

  \sa fitCurve()
*/
QPainterPath QwtSplineCurveFitter::fitCurvePath( const QPolygonF &points ) const
{
    QPainterPath path;

    if ( d_spline )
        path = d_spline->painterPath( points );

    return path;
}
