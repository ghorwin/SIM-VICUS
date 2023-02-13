#ifndef SVCOLORLEGEND_H
#define SVCOLORLEGEND_H

#include <QWidget>

class SVColorLegend : public QWidget
{
	Q_OBJECT
public:
	explicit SVColorLegend(QWidget *parent = nullptr);

	void init(double * minVal, double * maxVal, QColor * minColor, QColor * maxColor);

	void updateUi();

	void setTitle(const QString & title);

	double							*m_minValue = nullptr;
	double							*m_maxValue = nullptr;
	QColor							*m_minColor = nullptr;
	QColor							*m_maxColor = nullptr;
	QString							m_title;

signals:


protected:
	void paintEvent(QPaintEvent * /*event*/) override;
};

#endif // SVCOLORLEGEND_H
