/*	Authors: H. Fechner, A. Nicolai

	This file is part of the QtExt Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

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
