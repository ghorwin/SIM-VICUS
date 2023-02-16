#include "SVColorLegend.h"

#include <QPainter>

#include "SVStyle.h"
#include "SVSettings.h"

SVColorLegend::SVColorLegend(QWidget *parent)
	: QWidget{parent}
{

}


void SVColorLegend::initialize(const double * minVal, const double * maxVal, const QColor * minColor, const QColor * maxColor){
	m_minValue = minVal;
	m_maxValue = maxVal;
	m_minColor = minColor;
	m_maxColor = maxColor;
}


void SVColorLegend::updateUi() {
	update();
}


void SVColorLegend::setTitle(const QString &title) {
	m_title = title;
	update();
}


void SVColorLegend::paintEvent(QPaintEvent * /*event*/) {

	if (m_minValue==nullptr ||
		m_maxValue==nullptr ||
		m_minColor==nullptr ||
		m_maxColor==nullptr)
		return;

	double hMax, sMax, vMax, hMin, sMin, vMin, hNew;
	m_maxColor->getHsvF(&hMax, &sMax, &vMax);
	m_minColor->getHsvF(&hMin, &sMin, &vMin);

	int offsetV = 0;
	int offsetH = 0;
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
		hNew = hMax - double(i)/100 * (hMax - hMin);
		col.setHsvF(hNew, sMax, vMax);
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
		QString label = QString::number(y, 'f', 2);
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

	setFixedWidth(offsetH+barWidth+maxLabelWidth+titleWidth+10);
}
