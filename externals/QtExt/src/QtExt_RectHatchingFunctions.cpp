/*	Authors: H. Fechner, A. Nicolai

	This file is part of the QtExt Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#include "QtExt_RectHatchingFunctions.h"

#include <QPainter>

#include <cmath>

#include <IBK_math.h>

namespace QtExt {

static const qreal SQRT2 = 1.414213562373095;
static const qreal PI  = 3.141592653589793;

/*! Macro convert relative x position into absolute x position.*/
#define tx(xx) (xx + rect.x())
/*! Macro convert relative y position into absolute y position.*/
#define ty(yy) (yy + rect.y())

void drawHorizontalLines(QPainter* painter, const QRectF& rect, qreal distance) {
	Q_ASSERT(painter);

	qreal height = rect.height();
	qreal width = rect.width();
	int nmax = int(height / distance);
	if(nmax == 0)
		return;

	qreal x0 = 0;
	qreal x1 = width;

	for(int i=0; i<nmax; ++i) {
		double y = (i + 1) * distance;
		painter->drawLine(QPointF(tx(x0), ty(y)), QPointF(tx(x1), ty(y)));
	}
}

void drawVerticalLines(QPainter* painter, const QRectF& rect, qreal distance) {
	Q_ASSERT(painter);

	qreal height = rect.height();
	qreal width = rect.width();
	int nmax = int(width / distance);
	if(nmax == 0)
		return;

	qreal y0 = 0;
	qreal y1 = height;

	for(int i=0; i<nmax; ++i) {
		double x = (i + 1) * distance;
		painter->drawLine(QPointF(tx(x), ty(y0)), QPointF(tx(x), ty(y1)));
	}
}

void drawLinesObliqueLeftToRight(QPainter* painter, const QRectF& rect, qreal distance) {
	Q_ASSERT(painter);

	qreal a = SQRT2 * distance;
	qreal height = rect.height();
	qreal width = rect.width();
	int nmax = int((height + width) / a);

	for(int i=0; i<nmax; ++i) {
		qreal x0 = width - (i + 1) * a;
		qreal y0 = 0;
		if( x0 < 0) {
			qreal diff = x0 * -1.0;
			x0 = 0;
			y0 = diff;
		}

		qreal x1 = width;
		qreal y1 = (i + 1) * a;
		if(y1 > height) {
			qreal diff = y1 - height;
			y1 = height;
			x1 = width - diff;
		}

		painter->drawLine(QPointF(tx(x0), ty(y0)), QPointF(tx(x1), ty(y1)));
	}
}

void drawLinesObliqueRightToLeft(QPainter* painter, const QRectF& rect, qreal distance) {
	Q_ASSERT(painter);

	qreal a = SQRT2 * distance;
	qreal height = rect.height();
	qreal width = rect.width();
	int nmax = int((height + width) / a);

	for(int i=0; i<nmax; ++i) {
		qreal x0 = 0;
		qreal y0 = (i + 1) * a;
		if( y0 > height) {
			qreal diff = y0 - height;
			y0 = height;
			x0 = diff;
		}

		qreal x1 = (i + 1) * a;
		qreal y1 = 0;
		if(x1 > width) {
			qreal diff = x1 - width;
			x1 = width;
			y1 = diff;
		}
		if(IBK::near_equal(x0, x1))
			break;

		painter->drawLine(QPointF(tx(x0), ty(y0)), QPointF(tx(x1), ty(y1)));
	}
}

static int degree(qreal arc) {
	int res = int(arc * 180.0 / PI);
	return res;
}

static int degree16(qreal angle) {
	return int(angle * 16.0);
}

void drawInsulationHatch(QPainter* painter, const QRectF& rect, qreal density) {
	Q_ASSERT(painter);

	qreal height = rect.height();
	qreal width = rect.width();
	if(density < 3)
		density = 3.0;

	if( height >= width) {
		int n = int(height / width * density);
		qreal hs = height / n;
		qreal r = hs / 2.0;

		qreal K = r / (width - hs);
		qreal KSqr = K * K;
		qreal x0 = r * (1 + K);
		qreal x1 = width - x0;
		qreal u = r * std::sqrt(1- KSqr);
		qreal beta0Arc = std::asin(std::sqrt(1.0 - KSqr));
		qreal beta0 = degree(beta0Arc);
		qreal gamma = 360.0 - 2 * beta0;
		qreal beta1 = 180.0 - beta0;

		for(int i=0; i<n; ++i) {
			QRectF arcRect0(tx(0), ty(i * hs), hs, hs);
			painter->drawArc(arcRect0, degree16(beta0), degree16(gamma));

			qreal ym = i * hs + r;
			qreal yo1 = ym - u;
			qreal yu1 = ym + u;
			qreal yo2 = ym - r + u;
			qreal yu2 = ym + r - u;
			painter->drawLine(QPointF(tx(x0), ty(yo1)), QPointF(tx(x1), ty(yo2)));
			painter->drawLine(QPointF(tx(x0), ty(yu1)), QPointF(tx(x1), ty(yu2)));

			QRectF arcRect1(tx(width - hs), ty((i - 0.5) * hs), hs, hs);
			painter->drawArc(arcRect1, degree16(0), degree16(beta1) * -1);

			QRectF arcRect2(tx(width - hs), ty((i + 0.5) * hs), hs, hs);
			painter->drawArc(arcRect2, degree16(0), degree16(beta1));
		}
	}
	else {
		int n = int(width / height * density);
		qreal bs = width / n;
		qreal r = bs / 2.0;

		qreal K = r / (height - bs);
		qreal KSqr = K * K;
		qreal y0 = r * (1 + K);
		qreal y1 = height - y0;
		qreal u = r * std::sqrt(1- KSqr);
		qreal beta0Arc = std::asin(std::sqrt(1.0 - KSqr));
		qreal beta0 = 90.0 - degree(beta0Arc);
		qreal gamma = 180.0 + 2 * beta0;
		qreal beta1 = 90.0 + beta0;

		for(int i=0; i<n; ++i) {
			QRectF arcRect0(tx(i * bs), ty(0), bs, bs);
			painter->drawArc(arcRect0, degree16(beta0) * -1, degree16(gamma));

			qreal xm = i * bs + r;
			qreal xl1 = xm - u;
			qreal xr1 = xm + u;
			qreal xl2 = xm - r + u;
			qreal xr2 = xm + r - u;
			painter->drawLine(QPointF(tx(xl1), ty(y0)), QPointF(tx(xl2), ty(y1)));
			painter->drawLine(QPointF(tx(xr1), ty(y0)), QPointF(tx(xr2), ty(y1)));

			QRectF arcRect1(tx((i - 0.5) * bs), ty(height - bs), bs, bs);
			painter->drawArc(arcRect1, degree16(-90), degree16(beta1));

			QRectF arcRect2(tx((i + 0.5) * bs), ty(height - bs), bs, bs);
			painter->drawArc(arcRect2, degree16(-90), degree16(beta1) * -1);

		}

	}
}

} // namespace QtExt
