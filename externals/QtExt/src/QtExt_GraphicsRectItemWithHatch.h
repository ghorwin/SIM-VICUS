/*	Authors: H. Fechner, A. Nicolai

	This file is part of the QtExt Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#ifndef QtExt_GraphicsRectItemWithHatchH
#define QtExt_GraphicsRectItemWithHatchH

#include <QGraphicsRectItem>
#include <QPen>

#include <QtExt_Constants.h>

namespace QtExt {

	/*! Class similar with QGraphicsRectItem but with hatching.
		Rect uses own selection method. Currently a thick black rect is drawn.
	*/
	class GraphicsRectItemWithHatch : public QGraphicsRectItem {
	public:
		/*! Standard constructor.
			\param hatchingType Set the hatching type. HT_NoHatch draws a normal rect
			\param distance Distance between hatching elements or hatching density.
		*/
		GraphicsRectItemWithHatch(HatchingType hatchingType = HT_NoHatch, qreal distance = 1);

		/*! Paint event draws the rect and hatching if some is given.*/
		void paint ( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0 );

		/*! Return pen for hatching lines.*/
		const QPen& hatchPen() const;

		/*! Set pen for hatching lines.*/
		void setHatchPen(const QPen& pen);

	private:
		HatchingType	m_hatchingType;	///< Type of hatching.
		qreal			m_distance;		///< Distance between lines in pixel.
		QPen			m_hatchPen;		///< Pen for drawing hatch lines.

	};

} // namespace QtExt

#endif // QtExt_GraphicsRectItemWithHatchH
