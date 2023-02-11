#include "SVColorLegend.h"

#include <QPainter>

#include "SVStyle.h"
#include "SVSettings.h"

SVColorLegend::SVColorLegend(QWidget *parent)
	: QWidget{parent}
{

}


void SVColorLegend::init(double * minVal, double * maxVal, QColor * minColor, QColor * maxColor){
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
}


void SVColorLegend::paintEvent(QPaintEvent * event) {

	if (m_minValue==nullptr ||
		m_maxValue==nullptr ||
		m_minColor==nullptr ||
		m_maxColor==nullptr)
		return;

	double hMax, sMax, vMax, hMin, sMin, vMin, hNew;
	m_maxColor->getHsvF(&hMax, &sMax, &vMax);
	m_minColor->getHsvF(&hMin, &sMin, &vMin);

	double rectH = height() / 100;
	int offset = 1;
	int barWidth = 10;
	int labelWidth = 55;
	int titleWidth = 15;

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

	for (unsigned int i=0; i<=100; ++i) {
		hNew = hMax - double(i)/100 * (hMax - hMin);
		col.setHsvF(hNew, sMax, vMax);
		brush.setColor(col);
		painter.setBrush(brush);
		painter.drawRect( QRect(0, offset + int(i*rectH), barWidth, int(rectH)) );
	}

	QFont fnt;
	fnt.setPointSize(10);
	painter.setFont(fnt);
	painter.setPen(penText);
	painter.setBrush(QBrush());
	for (unsigned int i=0; i<=100; i+=25) {
		double y = *m_maxValue - double(i)/100 * (*m_maxValue - *m_minValue);
		QRect rect(barWidth +2, offset + int(i*rectH), labelWidth, int(1.5*rectH));
//		rect.
		painter.drawText( QRect(barWidth +2, offset + int(i*rectH), labelWidth, int(1.5*rectH)), QString::number(y, 'g', 2) );
	}

//	// draw vertically ???
	QTextOption opt;
	opt.setAlignment(Qt::AlignCenter);
	opt.setWrapMode(QTextOption::WordWrap);
	painter.rotate(-90);
	int flags = Qt::AlignHCenter | Qt::AlignTop;
	painter.drawText(-height(), barWidth+labelWidth-10, height(), titleWidth, flags, m_title);

}
