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

#ifndef QtExt_ConstructionGraphicsSceneH
#define QtExt_ConstructionGraphicsSceneH

#include <QObject>
#include <QGraphicsScene>


#include "QtExt_GraphicsRectItemWithHatch.h"
#include "QtExt_ConstructionLayer.h"

namespace QtExt {

/*! \brief Graphicsscene for showing a simple one dimensional wall structure.
	All necessary informations are given by setup.
*/
class ConstructionGraphicsScene : public QGraphicsScene
{
	Q_OBJECT
public:
	enum VisibleItems {
		VI_Dimensions = 0x01,
		VI_MaterialNames = 0x02,
		VI_BoundaryLabels = 0x04,
		VI_All = 0x07
	};

	/*! Default constructor.
		\param fontFamily Font family used for all fonts.
		\param onScreen Is true if painting is on screen.
		\param device Used paint device.
		\param parent Parent widget.
	*/
	ConstructionGraphicsScene(bool onScreen, QPaintDevice *device, QObject* parent = 0);

	/*! Destructor delete internal pens and strings.*/
	~ConstructionGraphicsScene();

	/*! The main interface for painting the diagram.
		\param frame Rectangle for showing graphic.
		\param device Device which is responsible for rendering.
		\param res  Resolution of paint device. For screen normally 1.0
		\param layers Vector of layers for drawing (material and thickness).
		\param leftLabel Text for the left boundary label
		\param rightLabel Text for the right boundary label
		\param visibleItems Integer value as combination of type VisibleItems.
	*/
	void setup(QRect frame, QPaintDevice *device, double res,
			   const QVector<ConstructionLayer>& layers, const QString & leftLabel, const QString & rightLabel,
			   int visibleItems = VI_All);


signals:

public slots:
	/*! Own clear function. Calls clear of QGraphicsScene.*/
	void clear();

protected:

private:
	struct InternalPens;			///< Struct contains pen informations.
	struct InternalStringItems;		///< struct contains internal string informations.

	/*! Helper function for drawing a rectangle with hatching. Create a GraphicsRectItemWithHatch item.
		\param x Left position.
		\param y Top position.
		\param w Rectangle width.
		\param h Rectangle height.
		\param hatchType Type of hatching (\sa HatchingType).
		\param hatchDistance Distance between lines for line hatching.
		\param hatchPen Pen for drawing hatching.
		\param pen Pen for drawing rectangle boundary.
		\param brush Brush of rectangle.
	*/
	QtExt::GraphicsRectItemWithHatch* addHatchedRect ( qreal x, qreal y, qreal w, qreal h, QtExt::HatchingType hatchType, int hatchDistance,
													   const QPen & hatchPen, const QPen & pen, const QBrush & brush);

	/*! Helper function for adding text. Sets alos the paintDevice.*/
	QGraphicsTextItem* addText(const QString& text, const QFont& font);
	/*! Updates m_xpos, m_innerFrame, m_xiLeft, m_xiRight and m_fontScale based on the input data.*/
	void calculatePositions();
	/*! Calculates and draws the dimensions.*/
	void drawDimensions();
	/*! Draws the wall including hashing.*/
	void drawWall();

	/*! Generates a number formatted for a dimension line.
		\param d Width in [m].
	*/
	static QString dimLabel(double d);

	/*! Local copy of all input data to be used by the drawing code.
		We store a local copy of the data so that graphics updates can be
		done individually from outer code.
	*/
	QVector<ConstructionLayer> m_inputData;

	/*! Font to be used for dimensions in the construction.*/
	QFont				m_tickFont;
	/*! Font to be used for axis titles in the diagram.*/
	QFont				m_axisTitleFont;

	QFont				m_fontScreen;
	QFont				m_fontPrinter;
	QFont				m_axisTitleFontScreen;
	QFont				m_axisTitleFontPrinter;

	/*! Boundaries for the chart in pixel.*/
	QRectF				m_frame;
	/*! Boundaries for the wall in pixel.*/
	QRectF				m_innerFrame;
	/*! X-coordinates for layers in pixel.*/
	std::vector<double>	m_xpos;
	/*! X-coordinate (left) of inner frame.*/
	int					m_xiLeft;
	/*! X-coordinate (right) of inner frame.*/
	int					m_xiRight;

	/*! Width of the wall in [m].*/
	double				m_wallWidth;

	/*! Text height for the given resolution and fontsize 10.*/
	double				m_textHeight10;
	/*! Text width of a 'T' for the given resolution and fontsize 10.*/
	double				m_textWidthT10;
	/*! True if chart is drawn on screen, false if chart is printed
		or exported as picture.
	*/
	bool				m_onScreen;
	/*! The resolution of the chart on a printer or exported image in pixel/mm.
		This is used to calculate the various distances and lengths.
	*/
	double				m_res;
	QPaintDevice*		m_device;					///< Paintdevice.

	// Pens
	InternalPens*			m_internalPens;
	InternalStringItems*	m_internalStringItems;

	QString					m_leftSideLabel;
	QString					m_rightSideLabel;
	bool					m_visibleDimensions;
	bool					m_visibleMaterialNames;
	bool					m_visibleBoundaryLabels;
};

} // namespace QtExt

#endif // QtExt_ConstructionGraphicsSceneH
