#ifndef VICUS_DRAWING_H
#define VICUS_DRAWING_H

#include <IBKMK_Vector2D.h>
#include <IBKMK_Vector3D.h>
#include <IBK_Line.h>

#include <QColor>

namespace VICUS {

class Drawing
{
public:
	Drawing();

	struct Layer {

		std::string		m_name;

		QColor			m_color;

		double			m_lineWidth;

		bool			m_visible;

	};


private:

	IBKMK::Vector3D																m_origin = IBKMK::Vector3D(0,0,0);

	std::set<Layer>																m_layer;

	std::map<std::string, std::vector<IBKMK::Vector2D> >						m_points;

	std::map<std::string, std::vector<IBK::Line> >								m_lines;

	std::map<std::string,  std::vector< std::vector<IBKMK::Vector2D> > >		m_polyLines;

};

} // namespace VICUS

#endif // VICUS_DRAWING_H
