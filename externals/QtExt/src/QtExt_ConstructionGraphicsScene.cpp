#include "QtExt_ConstructionGraphicsScene.h"

#include <memory>

#include <QAbstractTextDocumentLayout>
#include <QTextBlock>
#include <QTextLayout>
#include <QGraphicsTextItem>
#include <QGraphicsSceneMouseEvent>
#include <QDebug>
#include <QTextDocument>

#include <QtExt_TextFrame.h>

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

/*! Internal struct that contains all pens.*/
struct ConstructionGraphicsScene::InternalPens {
	/*! Standard constructor.*/
	InternalPens(ConstructionGraphicsScene* parent) : p(parent)
	{}

	ConstructionGraphicsScene* p;			///< Parent diagram scene

	/*! Set all diagram pens.*/
	void setPens();

	QPen m_layerBoundPen;		///< Pen for layer boundaries.
	QPen m_dimlinePen;			///< Pen for dimension lines.
	QPen m_vborderPen;			///< Pen for vertical borders.
	QPen m_hborderPen;			///< Pen for horizontal borders.
};

void ConstructionGraphicsScene::InternalPens::setPens() {

	// boundary layers
	m_layerBoundPen.setWidthF(std::max(p->m_res * 0.6, 2.0));
	m_layerBoundPen.setColor(p->m_onScreen ? Qt::darkGray : Qt::black);
	m_layerBoundPen.setCapStyle(Qt::FlatCap);


	// dimension lines
	m_dimlinePen.setWidthF(std::max(p->m_res * 0.2, 1.0));
	m_dimlinePen.setColor(p->m_onScreen ? Qt::gray : Qt::black);

	// vertical border
	m_vborderPen.setWidth(p->m_onScreen ? 2 : 0.8 * p->m_res);
	m_vborderPen.setColor(Qt::black);

	// horizontal border
	m_hborderPen.setWidthF(p->m_onScreen ? 1 : 0.2 * p->m_res);
	m_hborderPen.setColor(Qt::darkGray);
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
	{}


	/*! Set all fixed strings.*/
	void setStrings();

	ConstructionGraphicsScene* p;								///< Parent diagram scene

	QGraphicsTextItem*	m_dimensionDescTextItem;	///< Text item for dimension description.
};

void ConstructionGraphicsScene::InternalStringItems::setStrings() {
	// Description for dimension
	QString x_label = tr("Layer widths in [mm]");
	m_dimensionDescTextItem = p->addText(x_label, p->m_axisTitleFont);
	m_dimensionDescTextItem->setVisible(false);
}


ConstructionGraphicsScene::ConstructionGraphicsScene(bool onScreen, QPaintDevice *device, QObject* parent) :
	QGraphicsScene(parent),
	m_xiLeft(0),
	m_xiRight(0),
	m_wallWidth(0),
	m_textHeight10(15),
	m_textWidthT10(10),
	m_onScreen(onScreen),
	m_res(1.0),
	m_device(device),
	m_internalPens(new InternalPens(this)),
	m_internalStringItems(new InternalStringItems(this))
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

	m_internalPens->setPens();
	m_internalStringItems->setStrings();
}

ConstructionGraphicsScene::~ConstructionGraphicsScene() {
	delete m_internalPens;
	delete m_internalStringItems;
}

void ConstructionGraphicsScene::clear() {
	QGraphicsScene::clear();
	m_internalStringItems->setStrings();
	m_inputData.clear();
}

QGraphicsTextItem* ConstructionGraphicsScene::addText(const QString& text, const QFont& font) {
	QGraphicsTextItem* textItem = this->QGraphicsScene::addText(text);
	textItem->document()->documentLayout()->setPaintDevice(m_device);
	setAlignment(textItem->document(), Qt::AlignVCenter);
	textItem->setFont(font);
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
	if(hatchPen == QPen()) {
		QPen hatchPenNew(Qt::black);
		hatchPenNew.setWidth(4);
		hatchedRect->setHatchPen(hatchPenNew);
	}
	else {
		hatchedRect->setHatchPen(hatchPen);
	}
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
						 const QVector<ConstructionLayer>& layers)
{
	Q_ASSERT(device);
	m_device = device;
	m_res = res;
	// If no change no calculations needed
	bool noCalculationNeeded = frame == m_frame;
	noCalculationNeeded = noCalculationNeeded && layers == m_inputData;

	if( noCalculationNeeded)
		return;

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
	m_internalPens->setPens();


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
	drawDimensions();
	drawWall();

	// stop if only construction sketch is required
	setSceneRect(m_frame);
	emit changed(QList<QRectF>() << frame);
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
	int air_layer;             // for boundary layer

	if (m_onScreen) {
		axis_left = 40;
		axis_right = 50;
		air_layer = static_cast<int>(std::max(m_innerFrame.width()*0.03, 10.0));
	} else {
		axis_left = static_cast<int>(5 * m_res);
		axis_right = static_cast<int>(5 * m_res);
		air_layer = static_cast<int>(std::max(m_innerFrame.width()*0.03, 10.0 * m_res));
	}

	m_xiLeft = m_innerFrame.left() + axis_left;
	m_xiRight = m_innerFrame.right() - axis_right;

	// calculate width of wall itself in pixdel
	int pixwidth = m_xiRight - m_xiLeft - 2 * air_layer;

	// resize vector for x positions
	m_xpos.resize(m_inputData.size() + 1);
	m_xpos[0] = m_xiLeft + air_layer; // left wall boundary

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
	const double hatchDist = static_cast<int>(std::min(0.8 * m_res, 5.0));

	for (unsigned int i=1; i<m_xpos.size(); ++i) {
		int left = m_xpos[i-1];
		int width = m_xpos[i] - left;

		QGraphicsRectItem* rectItem = addHatchedRect(left, yt, width, rectHeight, m_inputData[i-1].m_hatch, hatchDist, m_internalPens->m_dimlinePen,
						   QPen(m_inputData[i-1].m_color), QBrush(m_inputData[i-1].m_color));
		rectItem->setFlag(QGraphicsItem::ItemIsSelectable, true);
		rectItem->setData(0, i-1);

		QGraphicsTextItem* materialName = addText(m_inputData[i-1].m_name, m_tickFont);
		materialName->setZValue(2);
		setAlignment(materialName->document(), Qt::AlignVCenter);
		materialName->document()->setTextWidth(rectHeight);
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

	// draw outside wall lines twice as thick
	addLine(xl, yt, xl, yb, m_internalPens->m_vborderPen);
	addLine(xr, yt, xr, yb, m_internalPens->m_vborderPen);

	addLine(xl, yt + m_internalPens->m_hborderPen.width(), xr, yt + m_internalPens->m_hborderPen.width(), m_internalPens->m_hborderPen);
	addLine(xl, yb, xr, yb, m_internalPens->m_hborderPen);

	// draw inside/outside text
	QGraphicsTextItem* outsideLeft = addText(tr("outside"), m_tickFont);
	setAlignment(outsideLeft->document(), Qt::AlignVCenter);
	outsideLeft->document()->setTextWidth(m_xpos.front());
	double textWidth = outsideLeft->document()->idealWidth();
	outsideLeft->setPos((m_xpos.front() - textWidth) / 2, yt - 2);

	QGraphicsTextItem* insideRight = addText(tr("inside"), m_tickFont);
	setAlignment(insideRight->document(), Qt::AlignVCenter);
	int outerBond = m_frame.width() - m_xpos.back();
	insideRight->document()->setTextWidth(outerBond);
	textWidth = insideRight->document()->idealWidth();
	insideRight->setPos(m_xpos.back() + (outerBond - textWidth) / 2, yt - 2);

}


} // namespace QtExt
