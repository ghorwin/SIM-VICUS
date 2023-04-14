#ifndef VICUS_DRAWING_H
#define VICUS_DRAWING_H

#include <IBKMK_Vector2D.h>
#include <IBKMK_Vector3D.h>
#include <IBK_Line.h>

#include <VICUS_RotationMatrix.h>

#include <libdxfrw.h>
#include <drw_interface.h>
#include <drw_objects.h>
#include <drw_base.h>

#include <QColor>
#include <QDebug>

namespace VICUS {

class Drawing
{
public:
	Drawing();

	struct Layer {

		QString			m_name;

		QColor			m_color = QColor(255, 255, 255);

		double			m_lineWidth;

		bool			m_visible;

	};

	struct AbstractObject {

		QString		         m_layername;

		Layer               *m_parentLayer;

		QColor				m_color = QColor(255, 255, 255);



	public:

		QColor color();
	};

	struct Point : public AbstractObject {

		IBKMK::Vector2D m_point;

		// default thickness value is 0
		double          m_thickness = 0;

	};

	struct Line : public AbstractObject {

		IBK::Line       m_line;

		// default thickness value is 0
		double          m_thickness = 0;

		double          m_lineWidth;

		bool            m_visible;

	};

	struct LWPolyLine : public AbstractObject {

		std::vector<IBKMK::Vector2D>    m_lwpolyline;

		// default thickness value is 0
		double                          m_thickness = 0;
		// true if the end point connected to the start
		int                            m_polyline_flag = 0;

	};

	struct PolyLine : public AbstractObject {

		std::vector<IBKMK::Vector2D>    m_polyline;

		// default thickness value is 0
		double                          m_thickness = 0;
		// true if the end point connected to the start
		int                            m_polyline_flag = 0;

	};

	IBKMK::Vector3D															m_origin = IBKMK::Vector3D(0,0,0);

	RotationMatrix															m_rotationMatrix;

	std::vector<Layer>                                                      m_layer;

	std::vector<Point>                                                      m_points;

	std::vector<Line>                                                       m_lines;

	std::vector<PolyLine>                                                   m_polylines;

	std::vector<LWPolyLine>                                                 m_lwpolylines;

	void updatePointer();


private:

	Layer *findLayerPointer(const QString &layername);

};



} // namespace VICUS

#endif // VICUS_DRAWING_H
