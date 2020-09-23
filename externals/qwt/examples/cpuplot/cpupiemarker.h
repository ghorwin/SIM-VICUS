//-----------------------------------------------------------------
// This class shows how to extend QwtPlotItems. It displays a
// pie chart of user/total/idle cpu usage in percent.
//-----------------------------------------------------------------

#ifndef CPU_PIE_MARKER_H
#define CPU_PIE_MARKER_H

#include <qwt_plot_item.h>

class CpuPieMarker: public QwtPlotItem
{
public:
    CpuPieMarker();

    virtual int rtti() const;

    virtual void draw( QPainter *,
        const QwtScaleMap &, const QwtScaleMap &, const QRectF & ) const;
};

#endif
