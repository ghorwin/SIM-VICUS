#ifndef SVCOLORLEGEND_H
#define SVCOLORLEGEND_H

#include <QWidget>

#include <SVColorMap.h>

/*! Class that paints the colorbar legend, the value labels and the property title. */
class SVColorLegend : public QWidget
{
	Q_OBJECT
public:
	explicit SVColorLegend(QWidget *parent = nullptr);

	/*! Sets pointer to min/max values and colors. */
	void initialize(const double * minVal, const double * maxVal, const SVColorMap *colorMap);

	/*! Used to call update() from outside */
	void updateUi();

	/*! Sets title string */
	void setTitle(const QString & title);

	/*! Moves the widget according to the point position and maps it to the parent widgets position.
		\param position The bottom-right corner position of the widget.
	*/
	void setPosition(const double & height, const QPoint &position);

protected:
	void paintEvent(QPaintEvent * /*event*/) override;

private:
	/*! Pointer to min/max values and colors */
	const double					*m_minValue = nullptr;
	const double					*m_maxValue = nullptr;
	const SVColorMap				*m_colorMap = nullptr;

	double							m_containerHeight = 600;
	QPoint							m_containerBottomRight;

	/*! Title string */
	QString							m_title;

};

#endif // SVCOLORLEGEND_H
