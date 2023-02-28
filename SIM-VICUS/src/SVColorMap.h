#ifndef SVCOLORMAPH
#define SVCOLORMAPH

#include <QColor>

class TiXmlElement;


class SVColorMap
{
public:

	/*! Contains color stops for linear colormap.*/
	struct ColorStop {
		double	m_pos;		///< Position value, must be between 0 and 1.
		QColor	m_color;	///< Color for this position.

		/*! Standard constructor. */
		ColorStop() :
			m_pos(0),
			m_color(Qt::black)
		{
		}

		/*! Value constructor.
			\arg val Position value.
			\arg col Color for position.
		*/
		ColorStop(double val, QColor col) :
			m_pos(val),
			m_color(col)
		{
		}
	};


	/*! Standard constructor.*/
	SVColorMap();

	/*! Reads the data from the xml element.
		Throws an IBK::Exception if a syntax error occurs.
	*/
	void readXML(const TiXmlElement * element);

	/*! Appends the element to the parent xml element.
		Throws an IBK::Exception in case of invalid data.
	*/
	void writeXML(TiXmlElement * parent) const;

	void interpolateColor(const double &val, QColor &color) const;

	std::vector<ColorStop>		m_linearColorStops;		///< Color stops of linear colormap

};

#endif // SVCOLORMAPH
