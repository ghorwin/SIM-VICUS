/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_plot_vectorfield.h"
#include "qwt_scale_map.h"
#include "qwt_color_map.h"
#include "qwt_painter.h"
#include "qwt_math.h"
#include "qwt_graphic.h"
#include <qpainter.h>
#include <qpainterpath.h>

#define DEBUG_RENDER 0

#if DEBUG_RENDER
#include <qelapsedtimer.h>
#include <qdebug.h>
#endif

static inline double qwtVector2Radians( double vx, double vy )
{
    if ( vx == 0.0 )
        return ( vy >= 0 ) ? M_PI_2 : 3 * M_PI_2;

    return ::atan2( vy, vx );
}

static inline double qwtVector2Magnitude( double vx, double vy )
{
    return sqrt( vx * vx + vy * vy );
}

QwtPlotVectorField::Arrow::Arrow( double headWidth, double tailWidth):
    m_headWidth( headWidth ),
    m_tailWidth( tailWidth ),
    m_length( headWidth + 4),
    m_path(new QPainterPath)
{
    /*
        Arrow is drawn horizontally, pointing into positive x direction
        with tip at 0,0.
    */
    m_path->lineTo( -m_headWidth, m_headWidth );
    m_path->lineTo( -m_headWidth, m_tailWidth );
    m_path->lineTo( -m_length, m_tailWidth );
    m_path->lineTo( -m_length, -m_tailWidth );
    m_path->lineTo( -m_headWidth, -m_tailWidth );
    m_path->lineTo( -m_headWidth, -m_headWidth );

    m_path->closeSubpath();
}

QwtPlotVectorField::Arrow::~Arrow() {
    delete m_path;
}

void QwtPlotVectorField::Arrow::setTailLength( qreal length ) {
    m_length = m_headWidth + length;
    m_path->setElementPositionAt( 3, -m_length, m_tailWidth );
    m_path->setElementPositionAt( 4, -m_length, -m_tailWidth );
}

void QwtPlotVectorField::Arrow::setLength( qreal length ) {
    if (length < m_headWidth) {
        setTailLength(0); // only the triangle will be drawn
    }
    else {
        setTailLength(length - m_headWidth);
    }
}

double QwtPlotVectorField::Arrow::length() const {
    return m_length;
}

void QwtPlotVectorField::Arrow::paint(QPainter * p) const {
    p->drawPath(*m_path);
}



QwtPlotVectorField::ThinArrow::ThinArrow( double headWidth):
    m_headWidth( headWidth ),
    m_length( headWidth + 4),
    m_path(new QPainterPath)
{
    /*
        Arrow is drawn horizontally, pointing into positive x direction
        with tip at 0,0.
    */
    m_path->lineTo( -m_headWidth, m_headWidth*0.6 );
    m_path->moveTo( 0, 0);
    m_path->lineTo( -m_headWidth, -m_headWidth*0.6 );
    m_path->moveTo( 0, 0);
    m_path->lineTo( -m_length, 0);

    m_path->closeSubpath();
}

QwtPlotVectorField::ThinArrow::~ThinArrow() {
    delete m_path;
}

void QwtPlotVectorField::ThinArrow::setLength( qreal length ) {
    int adjustedHeadWidth = m_headWidth;

    if (length < m_headWidth*3) {
        // make head smaller so that it is half of the length
        adjustedHeadWidth = length/3;
        m_path->setElementPositionAt( 1, -adjustedHeadWidth, adjustedHeadWidth*0.6 );
        m_path->setElementPositionAt( 3, -adjustedHeadWidth, -adjustedHeadWidth*0.6 );
    }
    else {
        m_path->setElementPositionAt( 1, -m_headWidth, m_headWidth*0.6 );
        m_path->setElementPositionAt( 3, -m_headWidth, -m_headWidth*0.6 );
    }
    m_length = length;
    m_path->setElementPositionAt( 5, -m_length, 0 );
}

double QwtPlotVectorField::ThinArrow::length() const {
    return m_length;
}

void QwtPlotVectorField::ThinArrow::paint(QPainter * p) const {
    p->drawPath(*m_path);
}


namespace
{
    class FilterMatrix
    {
    public:
        class Entry
        {
        public:
            Entry():
                count( 0 ),
                x( 0 ),
                y( 0 ),
                vx( 0 ),
                vy( 0 )
            {
            }

            inline void addSample( double sx, double sy,
                double svx, double svy )
            {
                x += sx;
                y += sy;

                vx += svx;
                vy += svy;

                count++;
            }

            int count;

            double x;
            double y;
            double vx;
            double vy;
        };

        FilterMatrix( const QRectF& dataRect,
            const QRectF& canvasRect, const QSizeF& cellSize )
        {
            d_dx = cellSize.width();
            d_dy = cellSize.height();

            d_x0 = dataRect.x();
            if ( d_x0 < canvasRect.x() )
                d_x0 += int ( ( canvasRect.x() - d_x0 ) / d_dx ) * d_dx;

            d_y0 = dataRect.y();
            if ( d_y0 < canvasRect.y() )
                d_y0 += int ( ( canvasRect.y() - d_y0 ) / d_dy ) * d_dy;

            d_numColumns = canvasRect.width() / d_dx + 1;
            d_numRows = canvasRect.height() / d_dy + 1;

            // limit column and row count to a maximum of 100000, so that memory
            // usage is not an issue
            if (d_numColumns > 1000) {
                d_dx = canvasRect.width()/1000;
                d_numColumns = canvasRect.width() / d_dx + 1;
            }
            if (d_numRows > 1000) {
                d_dy = canvasRect.height()/1000;
                d_numRows = canvasRect.height() / d_dx + 1;
            }

            d_x1 = d_x0 + d_numColumns * d_dx;
            d_y1 = d_y0 + d_numRows * d_dy;

            d_entries.resize( d_numRows * d_numColumns );
        }

        inline int numColumns() const
        {
            return d_numColumns;
        }

        inline int numRows() const
        {
            return d_numRows;
        }

        inline void addSample( double x, double y,
            double u, double v )
        {
            if ( x >= d_x0 && x < d_x1
                && y >= d_y0 && y < d_y1 )
            {
                Entry &entry = d_entries[ indexOf( x, y ) ];
                entry.addSample( x, y, u, v );
            }
        }

        const FilterMatrix::Entry* entries() const
        {
            return d_entries.constData();
        }

    private:
        inline int indexOf( qreal x, qreal y ) const
        {
            const int col = ( x - d_x0 ) / d_dx;
            const int row = ( y - d_y0 ) / d_dy;

            return row * d_numColumns + col;
        }

        qreal d_x0, d_x1, d_y0, d_y1, d_dx, d_dy;
        int d_numColumns;
        int d_numRows;

        QVector< Entry > d_entries;
    };
}

class QwtPlotVectorField::PrivateData
{
public:
    PrivateData():
        pen( Qt::NoPen ),
        brush( Qt::black ),
        indicatorOrigin( QwtPlotVectorField::OriginHead ),
        magnitudeRange(0,0), // invalid range
        magnitudeScaleFactor( 1.0 ),
        rasterSize( 20, 20 ),
        magnitudeModes( MagnitudeAsLength )
    {
        colorMap = new QwtLinearColorMap();
#if 0
        arrow = new Arrow();
#else
        arrow = new ThinArrow();
        pen = QPen(Qt::black);
        brush = QBrush(Qt::NoBrush);
#endif
    }

    ~PrivateData() {
        delete colorMap;
        delete arrow;
    }

    QPen pen;
    QBrush brush;

    IndicatorOrigin indicatorOrigin;
    ArrowSymbol * arrow;
    QwtColorMap * colorMap;
    /*! Stores the range of magnitudes to be used for the color map.
        If invalid (min=max or negative values), the range is determined from the data samples themselves.
    */
    QwtInterval			magnitudeRange;

    qreal magnitudeScaleFactor;
    QSizeF rasterSize;

    PaintAttributes paintAttributes;
    MagnitudeModes magnitudeModes;
};

/*!
  Constructor
  \param title Title of the curve
*/
QwtPlotVectorField::QwtPlotVectorField( const QwtText &title ):
    QwtPlotSeriesItem( title )
{
    init();
}

/*!
  Constructor
  \param title Title of the curve
*/
QwtPlotVectorField::QwtPlotVectorField( const QString &title ):
    QwtPlotSeriesItem( QwtText( title ) )
{
    init();
}

//! Destructor
QwtPlotVectorField::~QwtPlotVectorField()
{
    delete d_data;
}

/*!
  \brief Initialize data members
*/
void QwtPlotVectorField::init()
{
    setItemAttribute( QwtPlotItem::Legend );
    setItemAttribute( QwtPlotItem::AutoScale );

    d_data = new PrivateData;
    setData( new QwtVectorFieldData() );

    setZ( 20.0 );
}

void QwtPlotVectorField::setPen( const QPen &pen )
{
    if ( d_data->pen != pen )
    {
        d_data->pen = pen;

        itemChanged();
        legendChanged();
    }
}

QPen QwtPlotVectorField::pen() const
{
    return d_data->pen;
}

void QwtPlotVectorField::setBrush( const QBrush &brush )
{
    if ( d_data->brush != brush )
    {
        d_data->brush = brush;

        itemChanged();
        legendChanged();
    }
}

QBrush QwtPlotVectorField::brush() const
{
    return d_data->brush;
}

void QwtPlotVectorField::setIndicatorOrigin( IndicatorOrigin origin )
{
    d_data->indicatorOrigin = origin;
    if ( d_data->indicatorOrigin != origin )
    {
        d_data->indicatorOrigin = origin;
        itemChanged();
    }
}

QwtPlotVectorField::IndicatorOrigin QwtPlotVectorField::indicatorOrigin() const
{
    return d_data->indicatorOrigin;
}

void QwtPlotVectorField::setMagnitudeScaleFactor( qreal factor )
{
    if ( factor != d_data->magnitudeScaleFactor )
    {
        d_data->magnitudeScaleFactor = factor;
        itemChanged();
    }
}

qreal QwtPlotVectorField::magnitudeScaleFactor() const
{
    return d_data->magnitudeScaleFactor;
}

void QwtPlotVectorField::setRasterSize( const QSizeF& size )
{
    if ( size != d_data->rasterSize )
    {
        d_data->rasterSize = size;
        itemChanged();
    }
}

QSizeF QwtPlotVectorField::rasterSize() const
{
    return d_data->rasterSize;
}

/*!
  Specify an attribute how to draw the curve

  \param attribute Paint attribute
  \param on On/Off
  \sa testPaintAttribute()
*/
void QwtPlotVectorField::setPaintAttribute(
    PaintAttribute attribute, bool on )
{
    if ( on )
        d_data->paintAttributes |= attribute;
    else
        d_data->paintAttributes &= ~attribute;
}

/*!
    \return True, when attribute is enabled
    \sa PaintAttribute, setPaintAttribute()
*/
bool QwtPlotVectorField::testPaintAttribute(
    PaintAttribute attribute ) const
{
    return ( d_data->paintAttributes & attribute );
}

//! \return QwtPlotItem::Rtti_PlotField
int QwtPlotVectorField::rtti() const
{
    return QwtPlotItem::Rtti_PlotVectorField;
}

void QwtPlotVectorField::setMagnitudeMode( MagnitudeMode mode, bool on )
{
    if ( on == testMagnitudeMode( mode ) )
        return;

    if ( on )
        d_data->magnitudeModes |= mode;
    else
        d_data->magnitudeModes &= ~mode;

    itemChanged();
}

bool QwtPlotVectorField::testMagnitudeMode( MagnitudeMode mode ) const
{
    return d_data->magnitudeModes & mode;
}

void QwtPlotVectorField::setMagnitudeModes( MagnitudeModes modes )
{
    if ( d_data->magnitudeModes != modes )
    {
        d_data->magnitudeModes = modes;
        itemChanged();
    }
}

/*!
    Sets a new arrow symbol (implementation of arrow drawing code).
    Ownership is transferred to QwtPlotVectorField.
 */
void QwtPlotVectorField::setArrowSymbol(ArrowSymbol * symbol) {
    d_data->arrow = symbol;
    itemChanged();
    legendChanged();
}


QwtPlotVectorField::MagnitudeModes QwtPlotVectorField::magnitudeModes() const
{
    return d_data->magnitudeModes;
}

/*!
  Initialize data with an array of samples.
  \param samples Vector of points
*/
void QwtPlotVectorField::setSamples( const QVector<QwtVectorSample> &samples )
{
    setData( new QwtVectorFieldData( samples ) );
}

/*!
  Assign a series of samples

  setSamples() is just a wrapper for setData() without any additional
  value - beside that it is easier to find for the developer.

  \param data Data
  \warning The item takes ownership of the data object, deleting
           it when its not used anymore.
*/
void QwtPlotVectorField::setSamples( QwtVectorFieldData *data )
{
    setData( data );
}

/*!
  Change the color map

  Often it is useful to display the mapping between intensities and
  colors as an additional plot axis, showing a color bar.

  \param colorMap Color Map

  \sa colorMap(), QwtScaleWidget::setColorBarEnabled(),
      QwtScaleWidget::setColorMap()
*/
void QwtPlotVectorField::setColorMap( QwtColorMap *colorMap )
{
    if ( colorMap == NULL )
        return;

    if ( colorMap != d_data->colorMap )
    {
        delete d_data->colorMap;
        d_data->colorMap = colorMap;
    }

    legendChanged();
    itemChanged();
}

/*!
   \return Color Map used for mapping the intensity values to colors
   \sa setColorMap()
*/
const QwtColorMap *QwtPlotVectorField::colorMap() const
{
    return d_data->colorMap;
}

/*!
   Sets the min/max magnitudes to be used for color map lookups.
   If invalid (min=max=0 or negative values), the range is determined from
   the current range of magnitudes in the vector samples.
 */
void QwtPlotVectorField::setMagnitudeRange( const QwtInterval & magnitudeRange) {
    d_data->magnitudeRange = magnitudeRange;
}

/*!
   Computes length of the arrow in screen coordinate units based on
   its magnitude. Default implementation simply scales the vector
   using the magnitudeScaleFactor property.
   Re-implement this function to provide special handling for zero/non-zero
   magnitude arrows, or impose minimum/maximum arrow length limits.
   \return Length of arrow to be drawn in dependence of vector magnitude.
   \sa setMagnitudeScaleFactor
*/
double QwtPlotVectorField::arrowLength(double magnitude) const {
#if 0
    /*
       Normalize magnitude with respect to value range.
       Then, magnitudeScaleFactor is the number of pixels to draw (times the arrow tail width) for
       a vector of length equal to magnitudeRange.maxValue().
       The relative scaling ensures that change of data samples of very different magnitudes
       will always lead to a reasonable display on screen.
    */
    const QwtVectorFieldData * vectorData = dynamic_cast<const QwtVectorFieldData *>(data());
    if (d_data->magnitudeRange.maxValue() > 0)
        magnitude /= d_data->magnitudeRange.maxValue();
#endif
    double l = magnitude * d_data->magnitudeScaleFactor;
    if ( d_data->paintAttributes & LimitLength )
    {
        // TODO : make 30 a parameter? or leave this to user code and remove LimitLength altogether?
        l = qMin(l, 50.0);
        // ensure non-zero arrows are always at least 3 pixels long
        if (l != 0)
            l = qMax(l, 3.0);
    }
    return l;
}

QRectF QwtPlotVectorField::boundingRect() const
{
#if 0
    /*
        The bounding rectangle of the samples comes from the origins
        only, but as we know the scaling factor for the magnitude
        ( qwtVector2Magnitude ) here, we could try to include it ?
     */
#endif

    return QwtPlotSeriesItem::boundingRect();
}

QwtGraphic QwtPlotVectorField::legendIcon(
    int index, const QSizeF &size ) const
{
    Q_UNUSED( index );

    QwtGraphic icon;
    icon.setDefaultSize( size );

    if ( size.isEmpty() )
        return icon;

    QPainter painter( &icon );
    painter.setRenderHint( QPainter::Antialiasing,
        testRenderHint( QwtPlotItem::RenderAntialiased ) );

    painter.translate( -size.width(), -0.5 * size.height() );

    painter.setPen( d_data->pen );
    painter.setBrush( d_data->brush );

    d_data->arrow->setLength( size.width()-2 );
    d_data->arrow->paint(&painter);

    return icon;
}

/*!
  Draw a subset of the points

  \param painter Painter
  \param xMap Maps x-values into pixel coordinates.
  \param yMap Maps y-values into pixel coordinates.
  \param canvasRect Contents rectangle of the canvas
  \param from Index of the first sample to be painted
  \param to Index of the last sample to be painted. If to < 0 the
         series will be painted to its last sample.

  \sa drawDots()
*/
void QwtPlotVectorField::drawSeries( QPainter *painter,
    const QwtScaleMap &xMap, const QwtScaleMap &yMap,
    const QRectF &canvasRect, int from, int to ) const
{
    if ( !painter || dataSize() <= 0 )
        return;

    if ( to < 0 )
        to = dataSize() - 1;

    if ( from < 0 )
        from = 0;

    if ( from > to )
        return;

#if DEBUG_RENDER
    QElapsedTimer timer;
    timer.start();
#endif

    drawSymbols( painter, xMap, yMap, canvasRect, from, to );

#if DEBUG_RENDER
    qDebug() << timer.elapsed();
#endif
}

void QwtPlotVectorField::drawSymbols( QPainter *painter,
    const QwtScaleMap &xMap, const QwtScaleMap &yMap,
    const QRectF &canvasRect, int from, int to ) const
{
    const bool doAlign = QwtPainter::roundingAlignment( painter );
    const bool doClip = false;

    const bool isInvertingX = xMap.isInverting();
    const bool isInvertingY = yMap.isInverting();

    const QwtSeriesData<QwtVectorSample> *series = data();

    if ( d_data->magnitudeModes & MagnitudeAsColor )
    {
        // user input error, can't draw without color map
        // TODO: Discuss! Without colormap, silently fall back to uniform colors?
        if ( d_data->colorMap == NULL)
            return;
    }
    else {
        painter->setPen( d_data->pen );
        painter->setBrush( d_data->brush );
    }

    if ( ( d_data->paintAttributes & FilterVectors ) && !d_data->rasterSize.isEmpty() )
    {
        const QRectF dataRect = QwtScaleMap::transform(
            xMap, yMap, boundingRect() );

        // TODO: Discuss. How to handle raster size when switching from screen to print size!
        //       DPI-aware adjustment of rastersize? Or make "rastersize in screen coordinate"
        //       or "rastersize in plotcoordinetes" a user option?
#if 1
        // define filter matrix based on screen/print coordinates
        FilterMatrix matrix( dataRect, canvasRect, d_data->rasterSize );
#else
        // define filter matrix based on real coordinates

        // get scale factor from real coordinates to screen coordinates
        double xScale = 1;
        if (xMap.sDist() != 0)
            xScale = xMap.pDist()/xMap.sDist();
        double yScale = 1;
        if (yMap.sDist() != 0)
            yScale = yMap.pDist()/yMap.sDist();
        QSizeF canvasRasterSize(xScale*d_data->rasterSize.width(), yScale*d_data->rasterSize.height());
        FilterMatrix matrix( dataRect, canvasRect, canvasRasterSize );
#endif

        for ( int i = from; i <= to; i++ )
        {
            const QwtVectorSample sample = series->sample( i );
            if (!sample.isNull() )
            {
                matrix.addSample( xMap.transform( sample.x ),
                    yMap.transform( sample.y ), sample.vx, sample.vy );
            }
        }

        const int numEntries = matrix.numRows() * matrix.numColumns();
        const FilterMatrix::Entry* entries = matrix.entries();

        for ( int i = 0; i < numEntries; i++ )
        {
            const FilterMatrix::Entry &entry = entries[i];

            if ( entry.count == 0 )
                continue;

            double xi = entry.x / entry.count;
            double yi = entry.y / entry.count;

            if ( doAlign )
            {
                xi = qRound( xi );
                yi = qRound( yi );
            }

            const double vx = entry.vx / entry.count;
            const double vy = entry.vy / entry.count;

            drawSymbol( painter, xi, yi,
                isInvertingX ? -vx : vx, isInvertingY ? -vy : vy );
        }
    }
    else
    {
        for ( int i = from; i <= to; i++ )
        {
            const QwtVectorSample sample = series->sample( i );

            // arrows with zero length are never drawn
            if ( sample.isNull() )
                continue;

            double xi = xMap.transform( sample.x );
            double yi = yMap.transform( sample.y );

            if ( doAlign )
            {
                xi = qRound( xi );
                yi = qRound( yi );
            }

            if ( doClip )
            {
                if ( !canvasRect.contains( xi, yi ) )
                    continue;
            }

            drawSymbol( painter, xi, yi,
                isInvertingX ? -sample.vx : sample.vx,
                isInvertingY ? -sample.vy : sample.vy );
        }
    }
}

void QwtPlotVectorField::drawSymbol( QPainter *painter,
    double x, double y, double vx, double vy ) const
{
    const double magnitude = qwtVector2Magnitude( vx, vy );

    /*
        Set up transformation - rotation is independent of arrow/symbol drawing style.
    */

    const QTransform oldTransform = painter->transform();

    QTransform transform = oldTransform;
    if ( !transform.isIdentity() )
    {
        transform.translate( x, y );

        const double radians = qwtVector2Radians( vx, vy );
        transform.rotateRadians( radians );
    }
    else
    {
        /*
            When starting with no transformation ( f.e on screen )
            the matrix can be found without having to use
            trigonometric functions
         */

        qreal sin, cos;
        if ( magnitude == 0.0 )
        {
            // something
            sin = 1.0;
            cos = 0.0;
        }
        else
        {
            sin = vy / magnitude;
            cos = vx / magnitude;
        }

        transform.setMatrix( cos, sin, 0.0, -sin, cos, 0.0, x, y, 1.0 );
    }

    ArrowSymbol *arrow = d_data->arrow;

    double length = 0.0;

    if ( d_data->magnitudeModes & MagnitudeAsLength )
    {
        length = arrowLength( magnitude );
    }

    arrow->setLength( length );

    if( d_data->indicatorOrigin == OriginTail )
    {
        const qreal dx = arrow->length();
        transform.translate( dx, 0.0 );
    }
    else if ( d_data->indicatorOrigin == OriginCenter )
    {
        const qreal dx = arrow->length();
        transform.translate( 0.5 * dx, 0.0 );
    }

    painter->setWorldTransform( transform, false );

    /*
       Determine color for arrow if colored by magnitude.
    */
    if ( d_data->magnitudeModes & MagnitudeAsColor )
    {
        const QwtVectorFieldData * vectorData = dynamic_cast<const QwtVectorFieldData *>(data());
        Q_ASSERT(vectorData);
        QwtInterval range = d_data->magnitudeRange;
        if (range.minValue() == 0 && range.maxValue() == 0) {
            double minVal = vectorData->minMagnitude();
            double maxVal = vectorData->maxMagnitude();
            if ( maxVal == minVal )
                maxVal += 1;
            range = QwtInterval(minVal, maxVal);
        }
        QColor c = d_data->colorMap->rgb( range, magnitude );
        painter->setBrush(QBrush(c));
        painter->setPen(QPen(c));
    }

    arrow->paint(painter);

    // restore previous matrix
    painter->setWorldTransform( oldTransform, false );
}
