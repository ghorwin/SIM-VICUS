/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_PLOT_VECTOR_FIELD_H
#define QWT_PLOT_VECTOR_FIELD_H

#include "qwt_global.h"
#include "qwt_plot_seriesitem.h"
#include "qwt_series_data.h"

class QPen;
class QPainter;
class QBrush;
class QwtColorMap;

class QWT_EXPORT QwtPlotVectorField:
    public QwtPlotSeriesItem, QwtSeriesStore<QwtVectorSample>
{
public:
    enum IndicatorOrigin
    {
        OriginHead,
        OriginTail,
        OriginCenter
    };

    /*!
        Attributes to modify the rendering
        \sa setPaintAttribute(), testPaintAttribute()
    */
    enum PaintAttribute
    {
        FilterVectors        = 0x01,
        LimitLength          = 0x02
    };

    //! Paint attributes
    typedef QFlags<PaintAttribute> PaintAttributes;

    enum MagnitudeMode
    {
        MagnitudeAsColor = 0x01,
        MagnitudeAsLength = 0x02
    };

    //! Paint attributes
    typedef QFlags<MagnitudeMode> MagnitudeModes;

    /*!
        Defines abstract interface for arrow drawing routines.

        Arrow needs to be drawn horizontally with arrow tip at coordinate 0,0.
        arrowLength() shall return the entire length of the arrow (needed
        to translate the arrow for tail/centered alignment).
        setArrowLength() defines arror length in pixels (screen coordinates). It
        can be implemented to adjust other geometric properties such as
        the head size and width of the arrow. It is _always_ called before
        paint().

        A new arrow implementation can be set with QwtPlotVectorField::setArrowSymbol(), whereby
        ownership is transferred to the plot field.
    */
    class ArrowSymbol {
    public:
        virtual ~ArrowSymbol() {}
        virtual void setLength( qreal length ) = 0;
        virtual double length() const = 0;
        virtual void paint(QPainter * p) const = 0;
    };

    /*!
        Arrow implementation that only used lines, with optionally a filled arrow or only
        lines.
     */
    class ThinArrow : public QwtPlotVectorField::ArrowSymbol {
        Q_DISABLE_COPY(ThinArrow)
    public:
        ThinArrow( double headWidth = 6.0);
        virtual ~ThinArrow();
        virtual void setLength( qreal length ) override;
        virtual double length() const override;
        virtual void paint(QPainter * p) const override;
    private:
        const double m_headWidth;
        double m_length;
        QPainterPath * m_path;
    };

    /*!
        Arrow implementation that draws a filled arrow with outline, using
        a triangular head of constant width.
     */
    class Arrow : public QwtPlotVectorField::ArrowSymbol {
        Q_DISABLE_COPY(Arrow)
    public:
        Arrow( double headWidth = 6.0, double tailWidth = 1.0 );
        virtual ~Arrow();
        virtual void setLength( qreal length ) override;
        virtual double length() const override;
        virtual void paint(QPainter * p) const override;
    private:
        void setTailLength( qreal length );
        const double m_headWidth;
        const double m_tailWidth;
        double m_length;
        QPainterPath * m_path;
    };


    explicit QwtPlotVectorField( const QString &title = QString() );
    explicit QwtPlotVectorField( const QwtText &title );

    virtual ~QwtPlotVectorField();

    void setPaintAttribute( PaintAttribute, bool on = true );
    bool testPaintAttribute( PaintAttribute ) const;

    void setMagnitudeMode( MagnitudeMode, bool on = true );
    bool testMagnitudeMode( MagnitudeMode ) const;

    MagnitudeModes magnitudeModes() const;
    void setMagnitudeModes( MagnitudeModes );

    void setArrowSymbol(ArrowSymbol * symbol);

    void setPen( const QPen & );
    QPen pen() const;

    void setBrush( const QBrush & );
    QBrush brush() const;

    void setRasterSize( const QSizeF& );
    QSizeF rasterSize() const;

    void setIndicatorOrigin( IndicatorOrigin );
    IndicatorOrigin indicatorOrigin() const;

    void setSamples( const QVector<QwtVectorSample> & );
    void setSamples( QwtVectorFieldData * );

    void setColorMap( QwtColorMap * );
    const QwtColorMap *colorMap() const;
    void setMagnitudeRange( const QwtInterval & magnitudeRange);

    virtual double arrowLength(double magnitude) const;

    virtual QRectF boundingRect() const;

    virtual void drawSeries( QPainter *,
        const QwtScaleMap &xMap, const QwtScaleMap &yMap,
        const QRectF &canvasRect, int from, int to ) const;

    virtual int rtti() const;

    virtual QwtGraphic legendIcon( int index, const QSizeF & ) const;

    void setMagnitudeScaleFactor( qreal factor );
    qreal magnitudeScaleFactor() const;

protected:
    virtual void drawSymbols( QPainter *,
        const QwtScaleMap &xMap, const QwtScaleMap &yMap,
        const QRectF &canvasRect, int from, int to ) const;

    virtual void drawSymbol( QPainter *,
        double x, double y, double vx, double vy ) const;

private:
    void init();

    class PrivateData;
    PrivateData *d_data;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QwtPlotVectorField::PaintAttributes )
Q_DECLARE_OPERATORS_FOR_FLAGS( QwtPlotVectorField::MagnitudeModes )

#endif
