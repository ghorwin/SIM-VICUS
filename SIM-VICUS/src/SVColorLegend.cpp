#include "SVColorLegend.h"

#include <QPainter>

#include "SVSettings.h"
#include "SVViewStateHandler.h"


SVColorLegend::SVColorLegend(QWidget *parent)
	: QWidget{parent}
{
	setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::MSWindowsFixedSizeDialogHint);

	setWindowOpacity(0.8);

	SVViewStateHandler::instance().m_colorLegend = this;
}


void SVColorLegend::initialize(const double * minVal, const double * maxVal, const SVColorMap *colorMap){
	m_minValue = minVal;
	m_maxValue = maxVal;
	m_colorMap = colorMap;
}


void SVColorLegend::updateUi() {
	update();
}


void SVColorLegend::setTitle(const QString &title) {
	m_title = title;
	update();
}

void SVColorLegend::setPosition(const double & height, const QPoint & position) {
	m_containerHeight = height;
	m_containerBottomRight = position;
	resize(65, int(m_containerHeight * 0.9));
	move(m_containerBottomRight.x() - width(), int(m_containerBottomRight.y() - 0.95*m_containerHeight) );
}


void SVColorLegend::paintEvent(QPaintEvent * /*event*/) {

	if (m_minValue==nullptr ||
		m_maxValue==nullptr ||
		m_colorMap==nullptr)
		return;


	int offsetV = 10;
	int offsetH = 10;
	int barWidth = 10;
	int labelWidth = 55;
	int titleWidth = 15;
	double rectHeight = (height() - 2*offsetV)  / 100.0;

	QPainter painter(this);
	painter.setPen(Qt::NoPen);
	QColor col;
	QBrush brush;
	brush.setStyle(Qt::SolidPattern);
	QPen penText;
	if (SVSettings::instance().m_theme == SVSettings::TT_Dark)
		penText.setColor(Qt::white);
	else
		penText.setColor(Qt::black);
	penText.setWidth(1);

	// draw colormap
	for (unsigned int i=0; i<100; ++i) {
		m_colorMap->interpolateColor(1-double(i)/100, col);
		brush.setColor(col);
		painter.setBrush(brush);
		painter.drawRect( QRectF(offsetH, offsetV + double(i)*rectHeight, barWidth, rectHeight) );
	}

	//draw labels
	QFont fnt;
	fnt.setPointSize(10);
	painter.setFont(fnt);
	painter.setPen(penText);
	painter.setBrush(QBrush());
	QFontMetrics fm(fnt);
	int labelHeight = fm.lineSpacing();
	int maxLabelWidth = -1;
	// we draw 5 labels
	for (unsigned int i=0; i<=100; i+=20) {
		double y = *m_maxValue - double(i)/100 * (*m_maxValue - *m_minValue);
		// use floating point for numbers with more than 4 digits
		QString label;
		if (y>9999)
			label = QString::number(y, 'e', 2);
		else
			label = QString::number(y, 'f', 1);

		if (i<99)
			painter.drawText( QRectF(offsetH + barWidth + 2, offsetV + double(i)*rectHeight, labelWidth, labelHeight), label );
		else
			painter.drawText( QRectF(offsetH + barWidth + 2, offsetV + double(i)*rectHeight - 1.5*rectHeight, labelWidth, labelHeight), label );
		// determine max width of number label
		QRect bounds = painter.boundingRect(0, 0, labelWidth, int(1.5*rectHeight), 0, label);
		if (bounds.width() > maxLabelWidth)
			maxLabelWidth = bounds.width();
	}

	// draw title
	QTextOption opt;
	opt.setAlignment(Qt::AlignCenter);
	opt.setWrapMode(QTextOption::WordWrap);
	painter.rotate(-90);
	int flags = Qt::AlignHCenter | Qt::AlignTop;
	painter.drawText(-height(), offsetH+barWidth+maxLabelWidth+5, height(), titleWidth, flags, m_title);
	painter.rotate(90);

	setFixedWidth(offsetH + barWidth + maxLabelWidth + titleWidth + 10);

	move(m_containerBottomRight.x() - width(), int(m_containerBottomRight.y() - 0.95*m_containerHeight) );
}
