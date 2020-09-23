#ifndef SHAPE_FACTORY_H
#define SHAPE_FACTORY_H

#include <qpainterpath.h>

namespace ShapeFactory
{
    enum Shape
    {
        Rect,
        Triangle,
        Ellipse,
        Ring,
        Star,
        Hexagon
    };

    QPainterPath path( Shape, const QPointF &, const QSizeF & );
};

#endif
