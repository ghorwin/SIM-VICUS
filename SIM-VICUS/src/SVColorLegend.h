#ifndef SVCOLORLEGEND_H
#define SVCOLORLEGEND_H

#include <QWidget>

/*! Class that paints the colorbar legend, the value labels and the property title. */
class SVColorLegend : public QWidget
{
	Q_OBJECT
public:
	explicit SVColorLegend(QWidget *parent = nullptr);

	/*! Sets pointer to min/max values and colors. */
	void initialize(const double * minVal, const double * maxVal, const QColor * minColor, const QColor * maxColor);

	/*! Used to call update() from outside */
	void updateUi();

	/*! Sets title string */
	void setTitle(const QString & title);

protected:
	void paintEvent(QPaintEvent * /*event*/) override;

private:
	/*! Pointer to min/max values and colors */
	const double					*m_minValue = nullptr;
	const double					*m_maxValue = nullptr;
	const QColor					*m_minColor = nullptr;
	const QColor					*m_maxColor = nullptr;

	/*! Title string */
	QString							m_title;

};

#endif // SVCOLORLEGEND_H
