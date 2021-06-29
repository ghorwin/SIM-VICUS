/*	QtExt - Qt-based utility classes and functions (extends Qt library)

	Copyright (c) 2014-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Heiko Fechner
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 3 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

*/

#ifndef QtExt_RectHatchingFunctionsH
#define QtExt_RectHatchingFunctionsH

#include <QRectF>

class QPainter;

namespace QtExt {

void drawHorizontalLines(QPainter* painter, const QRectF& rect, qreal distance);

void drawVerticalLines(QPainter* painter, const QRectF& rect, qreal distance);

void drawLinesObliqueLeftToRight(QPainter* painter, const QRectF& rect, qreal distance);

void drawLinesObliqueRightToLeft(QPainter* painter, const QRectF& rect, qreal distance);

void drawInsulationHatch(QPainter* painter, const QRectF& rect, qreal density);

} // namespace QtExt

#endif // QtExt_RectHatchingFunctionsH
