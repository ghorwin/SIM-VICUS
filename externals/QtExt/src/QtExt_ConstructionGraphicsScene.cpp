/*	QtExt - Qt-based utility classes and functions (extends Qt library)

	Copyright (c) 2014-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Heiko Fechner    <heiko.fechner -[at]- tu-dresden.de>
	  Andreas Nicolai

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.

	Dieses Programm ist Freie Software: Sie können es unter den Bedingungen
	der GNU General Public License, wie von der Free Software Foundation,
	Version 3 der Lizenz oder (nach Ihrer Wahl) jeder neueren
	veröffentlichten Version, weiter verteilen und/oder modifizieren.

	Dieses Programm wird in der Hoffnung bereitgestellt, dass es nützlich sein wird, jedoch
	OHNE JEDE GEWÄHR,; sogar ohne die implizite
	Gewähr der MARKTFÄHIGKEIT oder EIGNUNG FÜR EINEN BESTIMMTEN ZWECK.
	Siehe die GNU General Public License für weitere Einzelheiten.

	Sie sollten eine Kopie der GNU General Public License zusammen mit diesem
	Programm erhalten haben. Wenn nicht, siehe <https://www.gnu.org/licenses/>.
*/

#include "QtExt_ConstructionGraphicsScene.h"

#include <memory>

#include <QAbstractTextDocumentLayout>
#include <QTextBlock>
#include <QTextLayout>
#include <QGraphicsTextItem>
#include <QGraphicsSceneMouseEvent>
#include <QDebug>
#include <QTextDocument>
#include <QApplication>
#include <QStyle>
#include <QWidget>
#include <QPainter>

#include "QtExt_TextFrame.h"
#include "QtExt_GraphicsTextItemInRect.h"

namespace QtExt {

void setAlignment(QTextDocument* doc, Qt::Alignment align) {
	QTextBlock block = doc->firstBlock();
	if( !block.isValid())
		return;
	QTextLayout* layout = block.layout();
	if(  layout->lineCount() == 0)
		return;
	// set of alignment options
	QTextOption option = layout->textOption();
	option.setAlignment(align);
	layout->setTextOption(option);
}

static QColor contrastColor(const QColor& background, const QColor& origin) {
	double r = background.redF();
	double g = background.greenF();
	double b = background.blueF();
	double brightness = std::pow(r,2.2) * 0.2126 +
			  std::pow(g,2.2) * 0.7152 +
			  std::pow(b,2.2) * 0.0722;

	return (brightness > 0.5) ? origin : Qt::white;
}

/*! Internal struct that contains all pens.*/
struct ConstructionGraphicsScene::InternalPens {
	/*! Standard constructor.*/
	InternalPens(ConstructionGraphicsScene* parent) :
		p(parent),
		m_backgroundColor(parent->backgroundBrush().color())
	{
	}

	ConstructionGraphicsScene* p;			///< Parent diagram scene

	/*! Set all diagram pens.*/
	void setPens(const QColor& backGround);

	QPen m_layerBoundPen;		///< Pen for layer boundaries.
	QPen m_dimlinePen;			///< Pen for dimension lines.
	QPen m_vborderPen;			///< Pen for vertical borders.
	QPen m_hborderPen;			///< Pen for horizontal borders.
	QPen m_hatchingPen;			///< Pen for material rect hatching
	QColor m_backgroundColor;	///< Color of background.
	double m_brightness;		///< Brightness of background color.
};

void ConstructionGraphicsScene::InternalPens::setPens(const QColor& backGround) {

	// boundary layers
	m_layerBoundPen.setWidthF(std::max(p->m_res * 0.6, 2.0));
	m_layerBoundPen.setColor(p->m_onScreen ? QtExt::contrastColor(backGround, Qt::darkGray) : QtExt::contrastColor(backGround, Qt::black));
	m_layerBoundPen.setCapStyle(Qt::FlatCap);


	// dimension lines
	m_dimlinePen.setWidthF(std::max(p->m_res * 0.2, 1.0));
	m_dimlinePen.setColor(p->m_onScreen ? QtExt::contrastColor(backGround, Qt::gray) : QtExt::contrastColor(backGround, Qt::black));

	m_hatchingPen.setWidthF(std::max(p->m_res * 0.1, 1.0));
	m_hatchingPen.setColor(p->m_onScreen ? QtExt::contrastColor(backGround, Qt::white) : QtExt::contrastColor(backGround, Qt::black));

	// vertical border
	m_vborderPen.setWidth(p->m_onScreen ? 2 : 0.8 * p->m_res);
	m_vborderPen.setColor(QtExt::contrastColor(backGround, Qt::black));

	// horizontal border
	m_hborderPen.setWidthF(p->m_onScreen ? 1 : 0.2 * p->m_res);
	m_hborderPen.setColor(QtExt::contrastColor(backGround, Qt::darkGray));
	m_hborderPen.setStyle(Qt::DashLine);
}

/*! Internal struct that contains all const string items.*/
struct ConstructionGraphicsScene::InternalStringItems {
	/*! Standard constructor.
		\param parent DiagramScene that contains the instance of this class.
	*/
	InternalStringItems(ConstructionGraphicsScene* parent) :
			p(parent),
			m_dimensionDescTextItem(0)
	{
	}


	/*! Set all fixed strings.*/
	void setStrings(const QColor& background);

	ConstructionGraphicsScene* p;					///< Parent diagram scene

	QGraphicsTextItem*	m_dimensionDescTextItem;	///< Text item for dimension description.
};

void ConstructionGraphicsScene::InternalStringItems::setStrings(const QColor& background) {
	// Description for dimension
	QString x_label = tr("Layer widths in [mm]");
	m_dimensionDescTextItem = p->addText(x_label, p->m_axisTitleFont);
	m_dimensionDescTextItem->setVisible(false);
	m_dimensionDescTextItem->setDefaultTextColor(contrastColor(background, Qt::black));
}


ConstructionGraphicsScene::ConstructionGraphicsScene(bool onScreen, QPaintDevice *device, QObject* parent) :
	QGraphicsScene(parent),
	m_xiLeft(0),
	m_xiRight(0),
	m_airLayer(0),
	m_wallWidth(0),
	m_textHeight10(15),
	m_textWidthT10(10),
	m_onScreen(onScreen),
	m_res(1.0),
	m_device(device),
	m_backgroundColor(Qt::white),
	m_internalPens(new InternalPens(this)),
	m_internalStringItems(new InternalStringItems(this)),
	m_visibleDimensions(true),
	m_visibleMaterialNames(true),
	m_visibleBoundaryLabels(true),
	m_markedLayer(-1),
	m_externalChange(false)
{
	Q_ASSERT(m_device);

	m_fontScreen = QFont(); // QFont(fontFamily);
	m_fontPrinter = m_fontScreen;
	m_axisTitleFontScreen = m_fontScreen;
	m_axisTitleFontPrinter = m_fontScreen;

#ifdef Q_OS_WIN
	m_fontScreen.setPointSize(8);
	m_fontPrinter.setPointSize(6);
	m_axisTitleFontScreen.setPointSize(10);
	m_axisTitleFontPrinter.setPointSize(8);
#elif defined(Q_OS_MAC)
	m_fontScreen.setPointSize(8);
	m_fontPrinter.setPointSize(6);
	m_axisTitleFontScreen.setPointSize(10);
	m_axisTitleFontPrinter.setPointSize(8);
	m_fontScreen.setKerning(false);
	m_fontPrinter.setKerning(false);
	m_axisTitleFontScreen.setKerning(false);
	m_axisTitleFontPrinter.setKerning(false);
#else
	m_fontScreen.setPointSize(8);
	m_fontPrinter.setPointSize(6);
	m_axisTitleFontScreen.setPointSize(10);
	m_axisTitleFontPrinter.setPointSize(8);
#endif

	QColor backGrd = QApplication::style()->standardPalette().brush(QPalette::Background).color();
	m_internalPens->setPens(backGrd);
	m_internalStringItems->setStrings(backGrd);

}

ConstructionGraphicsScene::~ConstructionGraphicsScene() {
	delete m_internalPens;
	delete m_internalStringItems;
}

void ConstructionGraphicsScene::clear() {
	QGraphicsScene::clear();
	m_internalPens->setPens(m_backgroundColor);
	m_internalStringItems->setStrings(m_backgroundColor);
	m_inputData.clear();
}

QGraphicsTextItem* ConstructionGraphicsScene::addText(const QString& text, const QFont& font) {
	QGraphicsTextItem* textItem = this->QGraphicsScene::addText(text);
	textItem->document()->documentLayout()->setPaintDevice(m_device);
	setAlignment(textItem->document(), Qt::AlignVCenter);
	textItem->setFont(font);
	textItem->setDefaultTextColor(contrastColor(m_backgroundColor, Qt::black));
	return textItem;
}

QGraphicsTextItem* ConstructionGraphicsScene::addTextInRect(const QString& text, const QFont& font) {
	GraphicsTextItemInRect* textItem = new GraphicsTextItemInRect(text);
	addItem(textItem);
	textItem->document()->documentLayout()->setPaintDevice(m_device);
	setAlignment(textItem->document(), Qt::AlignVCenter);
	textItem->setFont(font);
	textItem->setFillColor(Qt::white);
	textItem->setRectFramePen(QPen(QBrush(Qt::black), 2));
	textItem->setDefaultTextColor(contrastColor(textItem->fillColor(), Qt::black));
	return textItem;
}
QtExt::GraphicsRectItemWithHatch* ConstructionGraphicsScene::addHatchedRect ( qreal x, qreal y, qreal w, qreal h, QtExt::HatchingType hatchType, int hatchDistance,
											const QPen & hatchPen, const QPen & pen, const QBrush & brush ) {

	QtExt::GraphicsRectItemWithHatch* hatchedRect =
				new QtExt::GraphicsRectItemWithHatch(hatchType, hatchDistance);

	hatchedRect->setRect(x, y, w, h);
	if(brush != QBrush()) {
		hatchedRect->setBrush(brush);
	}
	hatchedRect->setPen(pen);
	hatchedRect->setHatchPen(hatchPen);
	addItem(hatchedRect);
	return hatchedRect;
}

// local function, composes dimension number in mm with either 0 or 1 digit after the point
QString ConstructionGraphicsScene::dimLabel(double d) {
	QString text;
	double dInmm = d * 1000.0;
	if (std::fabs(dInmm - std::floor(dInmm)) < 0.1)
		text = QString("%1").arg(std::floor(dInmm + 0.5),0, 'f', 0);
	else
		text = QString("%1").arg(dInmm, 0, 'f', 1);
	return text;
}

void ConstructionGraphicsScene::setup(QRect frame, QPaintDevice *device, double res,
									  const QVector<ConstructionLayer>& layers,
									  const QString & leftLabel, const QString & rightLabel,
									  int visibleItems)
{

	int currentVisibilty = 0;
	if(m_visibleDimensions)
		currentVisibilty += VI_Dimensions;
	if(m_visibleMaterialNames)
		currentVisibilty += VI_MaterialNames;
	if(m_visibleBoundaryLabels)
		currentVisibilty += VI_BoundaryLabels;
	m_visibleDimensions = visibleItems & VI_Dimensions;
	m_visibleMaterialNames = visibleItems & VI_MaterialNames;
	m_visibleBoundaryLabels = visibleItems & VI_BoundaryLabels;

	Q_ASSERT(device);
	m_device = device;
	m_res = res;
	// If no change no calculations needed
	bool noCalculationNeeded = frame == m_frame;
	noCalculationNeeded = noCalculationNeeded && layers == m_inputData;
	noCalculationNeeded = noCalculationNeeded && visibleItems == currentVisibilty;
	noCalculationNeeded = noCalculationNeeded && !m_externalChange;

	m_leftSideLabel = leftLabel;
	m_rightSideLabel = rightLabel;

	if( noCalculationNeeded)
		return;

	m_externalChange = false;

	if (m_onScreen) {
		m_tickFont = m_fontScreen;
		m_axisTitleFont = m_axisTitleFontScreen;
	}
	else {
		m_tickFont = m_fontPrinter;
		m_axisTitleFont = m_axisTitleFontPrinter;
	}

	{
		QtExt::TextFrame testText(0);
		testText.setDefaultFont(m_tickFont);
		testText.setText(QString("T"));
		m_textHeight10 = testText.frameRect(m_device, -1).height();
		m_textWidthT10 = testText.frameRect(m_device, -1).width();
	}

	clear();
	m_inputData = layers;
	m_frame = frame;
	m_internalPens->setPens(m_backgroundColor);
	m_internalStringItems->setStrings(m_backgroundColor);


	// check if we have valid construction data for drawing the construction sketch
	bool valid = true;
	m_wallWidth = 0;
	for (int i=0; i<m_inputData.size(); ++i) {
		double w = m_inputData[i].m_width;
		if (w <= 0) {
			valid = false;
			break;
		}
		m_wallWidth += w;
	}
	if (m_inputData.empty() || !valid)
		return; // nothing to draw

	// update coordinates, normally only necessary when wall construction changes or view is resized
	calculatePositions();
	if(m_visibleDimensions)
		drawDimensions();
	drawWall();
	drawMarkerLines();
	drawMarkerArea();

	setSceneRect(m_frame);
	emit changed(QList<QRectF>() << frame);
}

void ConstructionGraphicsScene::clearLineMarkers() {
	if(!m_lineMarker.empty()) {
		m_lineMarker.clear();
		m_externalChange = true;
	}
}

void ConstructionGraphicsScene::addLinemarker(double pos, const QPen& pen, const QString& name) {
	m_lineMarker.push_back(LineMarker(pos,pen,name));
	m_externalChange = true;
}

void ConstructionGraphicsScene::addLinemarker(const LineMarker& lineMarker) {
	m_lineMarker.push_back(lineMarker);
	m_externalChange = true;
}

void ConstructionGraphicsScene::setBackground(const QColor& bkgColor) {
	if(m_backgroundColor != bkgColor) {
		m_backgroundColor = bkgColor;
		m_externalChange = true;
	}
}

void ConstructionGraphicsScene::markLayer(int layerIndex) {
	if(m_markedLayer != layerIndex) {
		m_markedLayer = layerIndex;
		m_externalChange = true;
	}
}

void ConstructionGraphicsScene::setAreaMarker(const AreaMarker& am) {
	m_areaMarker = am;
}

void ConstructionGraphicsScene::removeAreaMarker() {
	m_areaMarker.m_xStart = 0;
	m_areaMarker.m_xEnd = 0;
}


void ConstructionGraphicsScene::calculatePositions() {
	if(m_inputData.empty())
		return;

	// get axis text geometry
	int leftAxisMargins = m_textHeight10;
	int rightAxisMargins = m_textHeight10;
	int top = m_frame.top() + 5;
	int bottom = m_frame.height() - 15;

	int frameTop = m_frame.top();
	int frameBottom = m_frame.bottom();

	if (!m_onScreen) {
		top = static_cast<int>(frameTop + 5 * m_res);
		bottom = static_cast<int>(frameBottom - 5 * m_res);
	}

	std::unique_ptr<QTextDocument> textDocument(new QTextDocument);
	textDocument->documentLayout()->setPaintDevice(m_device);
	textDocument->setDefaultFont(m_axisTitleFont);

	// on screen, we always keep space at the top and bottom
	leftAxisMargins = 0;
	rightAxisMargins = 0;

	int left = m_frame.left() + leftAxisMargins;
	int right = m_frame.width() - rightAxisMargins;
	m_innerFrame.setRect(left, top, right - left, bottom - top);

	// calculation x positions
	int axis_left, axis_right; // for axes and description
	int m_airLayer;             // for boundary layer

	if (m_onScreen) {
		axis_left = 40;
		axis_right = 50;
		m_airLayer = static_cast<int>(std::max(m_innerFrame.width()*0.03, 10.0));
	} else {
		axis_left = static_cast<int>(5 * m_res);
		axis_right = static_cast<int>(5 * m_res);
		m_airLayer = static_cast<int>(std::max(m_innerFrame.width()*0.03, 10.0 * m_res));
	}
	if(!m_visibleBoundaryLabels) {
		m_airLayer = 0;
	}

	m_xiLeft = m_innerFrame.left() + axis_left;
	m_xiRight = m_innerFrame.right() - axis_right;

	// calculate width of wall itself in pixdel
	int pixwidth = m_xiRight - m_xiLeft - 2 * m_airLayer;

	// resize vector for x positions
	m_xpos.resize(m_inputData.size() + 1);
	m_xpos[0] = m_xiLeft + m_airLayer; // left wall boundary

	// calculate scaling factor for wall coordinates in pixel/m
	double dw = pixwidth/m_wallWidth;
	// calculate layer thicknesses in pixel and store x positions
	for (int i=0; i<m_inputData.size(); ++i) {
		m_xpos[i+1] = m_xpos[i] + static_cast<int>(m_inputData[i].m_width * dw);
	}
}

/*! Contains the properties of diagram lables.*/
struct LabelProperties {
	QString				m_text;		///< Label text.
	QRectF				m_brect;	///< Bounding rect.
	bool				m_fit;		///< Indicates if the label fits in the available space.
	QGraphicsTextItem*	m_textItem;	///< Text item for label.
	int					m_index;	///< Label index.
	/*! Standard constructor.*/
	LabelProperties() :
		m_fit(true),
		m_textItem(0),
		m_index(-1)
	{}
};

void ConstructionGraphicsScene::drawDimensions() {
	if(m_inputData.empty())
		return;

#ifdef Q_OS_MAC
		const double MACYShift = 10.0;
#endif

	setFont(m_tickFont);

	// set here the distance in pixels between dim label and dim line
	// this distance is used everywhere as distance
	const double dist = static_cast<int>(std::max(0.8 * m_res, 3.0));

	// store number of x positions
	const int n = (int)m_xpos.size();

	// check if all dimensions fit between the helping lines,
	// and while at it, calculate text widths

	std::vector<LabelProperties> labelVectTop;
	std::vector<LabelProperties> labelVectBottom;
	double maxTextHeight(0);
	for (int i=0; i<n-1; ++i) {
		LabelProperties tempLabel;
		tempLabel.m_text = dimLabel(m_inputData[i].m_width);
		tempLabel.m_textItem = addText(tempLabel.m_text, m_tickFont);
		tempLabel.m_brect = tempLabel.m_textItem->boundingRect();
		tempLabel.m_index = i;
		maxTextHeight = std::max(maxTextHeight, tempLabel.m_brect.height());
		int width_available = m_xpos[i+1] - m_xpos[i] - 2 * dist;
		// check for fit between lines
		if (tempLabel.m_brect.width() > width_available) {
			tempLabel.m_fit = false;
			labelVectBottom.push_back(tempLabel);
		} else {
			labelVectTop.push_back(tempLabel);
		}
	}
	double linespacing = maxTextHeight;

	// calculate dim lines and labels
	std::vector<double> ylines;
	double linePos = m_innerFrame.bottom() - linespacing;
	ylines.push_back(linePos);		// main dim line
	double yo = m_innerFrame.bottom() - 2.1 * linespacing;
	if( n > 2)
		ylines.push_back(yo);		// secondary dim line


	// did we have a label too big for the available space?
	if (!labelVectBottom.empty() && n > 1) {
		yo = m_innerFrame.bottom() - 3.1 * linespacing;
		ylines.back() = yo;	// secondary dim line
	}

	// now update the inner frame i.e. the wall frame
	m_innerFrame.setBottom(ylines.back() - linespacing);

	// some helping constants
	double yu = ylines.front();

	double xl = m_xpos.front();
	double xr = m_xpos.back();

	// set appropriate pen for the dimensioning lines.

	for( size_t i=0, count=labelVectTop.size(); i<count; ++i) {
		int index = labelVectTop[i].m_index;
		double layer_pixwidth = m_xpos[index+1] - m_xpos[index];
		double xmiddle = m_xpos[index] + layer_pixwidth / 2.0;
		double x_text = xmiddle - labelVectTop[i].m_brect.width() / 2;

		QGraphicsTextItem* textItem = labelVectTop[i].m_textItem;
		textItem->setFont(m_tickFont);
		textItem->setDefaultTextColor(contrastColor(m_backgroundColor, Qt::black));
		setAlignment(textItem->document(), Qt::AlignVCenter);
		double ypos = yo - labelVectTop[i].m_brect.height();
#ifdef Q_OS_MAC
		ypos += labelVectTop[i].m_brect.height() / MACYShift;
#endif
		textItem->setPos(x_text, ypos);

	}
	for( size_t i=0, count=labelVectBottom.size(); i<count; ++i) {
		int index = labelVectBottom[i].m_index;
		double layer_pixwidth = m_xpos[index+1] - m_xpos[index];
		double xmiddle = m_xpos[index] + layer_pixwidth / 2.0;
		double x_text = xmiddle - labelVectBottom[i].m_brect.width() / 2;
		QGraphicsTextItem* textItem = labelVectBottom[i].m_textItem;
		setAlignment(textItem->document(), Qt::AlignVCenter);
		textItem->setFont(m_tickFont);
		textItem->setDefaultTextColor(contrastColor(m_backgroundColor, Qt::black));
		double ypos = yo;
		if( i>0) {
			double xEndBefore = labelVectBottom[i-1].m_textItem->pos().x() + labelVectBottom[i-1].m_brect.width();
			if( x_text <= xEndBefore) {
				x_text = xEndBefore;
			}
		}
#ifdef Q_OS_MAC
		ypos += labelVectBottom[i].m_brect.height() / MACYShift;
#endif
		textItem->setPos(x_text, ypos);

		addLine(xmiddle, yo, x_text + labelVectBottom[i].m_brect.width() / 2.0, ypos + 2 * dist, m_internalPens->m_dimlinePen);
	}

	// put label on main dim line
	QString wallWidthText = dimLabel(m_wallWidth);
	QGraphicsTextItem* textItem = addText(wallWidthText, m_tickFont);
	int xw = static_cast<int>((xl + xr) / 2 - textItem->boundingRect().width() / 2);
	double ypos = yu - textItem->boundingRect().height();
#ifdef Q_OS_MAC
		ypos += textItem->boundingRect().height() / MACYShift;
#endif
	textItem->setDefaultTextColor(contrastColor(m_backgroundColor, Qt::black));
	textItem->setPos(xw, ypos);

	m_internalStringItems->m_dimensionDescTextItem->setVisible(true);
	xw = static_cast<int>((xl + xr)/2 - m_internalStringItems->m_dimensionDescTextItem->boundingRect().width() / 2);

	m_internalStringItems->m_dimensionDescTextItem->setPos(xw, yu + dist);

	// draw dim helping lines

	// lower dim line
	addLine(xl - dist, yu, xr + dist, yu, m_internalPens-> m_dimlinePen);
	// upper dim line
	addLine(xl - dist, yo, xr + dist, yo, m_internalPens->m_dimlinePen);

	int dist_diag = static_cast<int>(std::floor( dist * 0.7 ));
	for (int i=0; i<n; ++i) {
		int xstart = m_xpos[i];
		int ystart = yo + dist - linespacing;
		if (i==0 || i==n-1) {
			// draw outer lines
			addLine(xstart, ystart, xstart, yu+dist, m_internalPens->m_dimlinePen);
			// also draw diagonals at outer lines
			addLine(xstart - dist_diag, yu + dist_diag, xstart + dist_diag, yu - dist_diag, m_internalPens->m_dimlinePen);
		}
		else {
			// draw inner lines
			addLine(xstart, ystart, xstart, yo + dist, m_internalPens->m_dimlinePen);
		}

		// draw upper diagonals
		addLine(xstart - dist_diag, yo + dist_diag, xstart + dist_diag, yo - dist_diag, m_internalPens->m_dimlinePen);
	}
}

void ConstructionGraphicsScene::drawWall() {
	if(m_inputData.empty())
		return;

	const int yt = m_innerFrame.top();
	const int yb = m_innerFrame.bottom();
	const int xl = m_xpos.front();
	const int xr = m_xpos.back();

	int rectHeight = yb - yt;

	// Draw material colors
	int hatchDist = 0;

	int markedLayer = m_markedLayer >= m_inputData.size() ? -1 : m_markedLayer;

	for (unsigned int i=1; i<m_xpos.size(); ++i) {
		int left = m_xpos[i-1];
		int width = m_xpos[i] - left;

		QtExt::HatchingType hatching = m_inputData[i-1].m_hatch;
		if (int(i-1) == markedLayer) {
			hatchDist = std::max(width / 20, (int)(m_internalPens->m_hatchingPen.widthF() * 3));
			hatching = QtExt::HT_LinesObliqueLeftToRight;
			m_internalPens->m_hatchingPen.setColor(QtExt::contrastColor(m_inputData[i-1].m_color, Qt::black));
		}

		QGraphicsRectItem* rectItem = addHatchedRect(left, yt, width, rectHeight, hatching, hatchDist, m_internalPens->m_hatchingPen,
						   QPen(m_inputData[i-1].m_color), QBrush(m_inputData[i-1].m_color));
		rectItem->setFlag(QGraphicsItem::ItemIsSelectable, true);
		rectItem->setData(0, i-1);

		if(m_visibleMaterialNames) {
			QGraphicsTextItem* materialName = addText(m_inputData[i-1].m_name, m_tickFont);
			materialName->setZValue(2);
			setAlignment(materialName->document(), Qt::AlignVCenter);
			materialName->document()->setTextWidth(rectHeight);
			materialName->setDefaultTextColor(QtExt::contrastColor(m_inputData[i-1].m_color, Qt::black));
			double textWidth = materialName->document()->idealWidth();

#if QT_VERSION >= 0x040600
			materialName->setRotation(-90);
#else
			materialName->rotate( -90 );
#endif

			int xpos = left + (width - materialName->boundingRect().height()) / 2.0;
			int ypos = yb - (rectHeight - textWidth) / 2.0;
			materialName->setPos(xpos, ypos);

			materialName->setVisible(materialName->boundingRect().height() < width - 2);
		}
	}

	// draw outside wall lines twice as thick
	addLine(xl, yt, xl, yb, m_internalPens->m_vborderPen);
	addLine(xr, yt, xr, yb, m_internalPens->m_vborderPen);

	addLine(xl, yt + m_internalPens->m_hborderPen.width(), xr, yt + m_internalPens->m_hborderPen.width(), m_internalPens->m_hborderPen);
	addLine(xl, yb, xr, yb, m_internalPens->m_hborderPen);

	// draw inside/outside text
	if(m_visibleBoundaryLabels) {
		QGraphicsTextItem* outsideLeft = addText(m_leftSideLabel, m_tickFont);
		setAlignment(outsideLeft->document(), Qt::AlignVCenter);
		outsideLeft->document()->setTextWidth(m_xpos.front());
		double textWidth = outsideLeft->document()->idealWidth();
		outsideLeft->setPos((m_xpos.front() - textWidth) / 2, yt - 2);
		outsideLeft->setDefaultTextColor(contrastColor(m_backgroundColor, Qt::black));

		QGraphicsTextItem* insideRight = addText(m_rightSideLabel, m_tickFont);
		setAlignment(insideRight->document(), Qt::AlignVCenter);
		int outerBond = m_frame.width() - m_xpos.back();
		insideRight->document()->setTextWidth(outerBond);
		textWidth = insideRight->document()->idealWidth();
		insideRight->setPos(m_xpos.back() + (outerBond - textWidth) / 2, yt - 2);
		insideRight->setDefaultTextColor(contrastColor(m_backgroundColor, Qt::black));
	}
}

void ConstructionGraphicsScene::drawMarkerLines() {
	if(m_lineMarker.empty())
		return;

	const int yt = m_innerFrame.top();
	const int yb = m_innerFrame.bottom();
	const int xl = m_xpos.front();
	int pixwidth = m_xiRight - m_xiLeft - 2 * m_airLayer;
	double dw = pixwidth/m_wallWidth;

	int number = 1;
	qreal lastHeight = 0;
	for(const auto& line : m_lineMarker) {
		QPen pen = line.m_pen;
		pen.setWidthF(pen.widthF() * m_res);
		int pos = (number-1) % 2;
		int xpos = xl + static_cast<int>(line.m_pos * dw);
		addLine(xpos, yb, xpos, yt, pen);

		// numbering
		QGraphicsTextItem* numberText = addTextInRect(line.m_name, m_axisTitleFont);
		setAlignment(numberText->document(), Qt::AlignVCenter);
		int textWidth = numberText->document()->idealWidth();
		numberText->setPos(xpos + (textWidth) / 2, (yb - yt) / 2 + pos * (lastHeight + 2));
		numberText->setDefaultTextColor(contrastColor(m_backgroundColor, Qt::black));
		lastHeight = numberText->boundingRect().height();
		++number;
	}
}

void ConstructionGraphicsScene::drawMarkerArea() {
	if(!m_areaMarker.valid())
		return;

	const int yt = m_innerFrame.top();
	const int yb = m_innerFrame.bottom();
	const int xl = m_xpos.front();
	int pixwidth = m_xiRight - m_xiLeft - 2 * m_airLayer;
	double dw = pixwidth/m_wallWidth;

	QPen pen = m_areaMarker.m_framePen;
	pen.setWidthF(pen.widthF() * m_res);
	int xposStart = xl + static_cast<int>(m_areaMarker.m_xStart * dw);
	int xposEnd = xl + static_cast<int>(m_areaMarker.m_xEnd * dw);

	int hatchDist = std::max((xposEnd - xposStart) / 20, (int)(m_internalPens->m_hatchingPen.widthF() * 3));
	m_internalPens->m_hatchingPen.setColor( Qt::gray);

	QGraphicsRectItem* rectItem = addHatchedRect(xposStart, yt, xposEnd - xposStart, yb - yt, QtExt::HT_CrossHatchOblique, hatchDist, m_internalPens->m_hatchingPen,
				   pen, m_areaMarker.m_areaBrush);
	rectItem->setFlag(QGraphicsItem::ItemIsSelectable, false);

}


} // namespace QtExt
