#ifndef VICUS_DRAWING_H
#define VICUS_DRAWING_H

#include <IBKMK_Vector2D.h>
#include <IBKMK_Vector3D.h>
#include <IBK_Line.h>

#include <VICUS_RotationMatrix.h>
#include <QQuaternion>

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

		// TODO Maik: dokustring
		/*! here */
		QString			m_name;

		QColor			m_color = QColor();

		int				m_lineWeight;

		bool			m_visible;

	};

	struct AbstractDrawingObject {

		QString		        m_layername;

		const Layer         *m_parentLayer;

		QColor				m_color = QColor();

		double				m_lineWeight = 0;

		const QColor*		color() const;

		double				lineWeight() const;
	};

	struct Point : public AbstractDrawingObject {

		IBKMK::Vector2D m_point;
	};

	struct Line : public AbstractDrawingObject {

		IBK::Line       m_line;

	};

	/*! Stores both LW and normal polyline */
	struct PolyLine : public AbstractDrawingObject {

		std::vector<IBKMK::Vector2D>    m_polyline;

		// true if the end point connected to the start
		int                            m_polyline_flag = 0;

	};

	/*! Stores both LW and normal polyline */
	struct Circle : public AbstractDrawingObject {

		IBKMK::Vector2D    m_center;

		double             m_radius;

		bool				m_clockwise;

	};

	struct Ellipse : public AbstractDrawingObject {

		IBKMK::Vector2D    m_center;

		IBKMK::Vector2D    m_majorAxis;

		double             m_ratio;

		double             m_startAngle;

		double             m_endAngle;

	};

	struct Arc : public AbstractDrawingObject {

		IBKMK::Vector2D    m_center;

		double             m_radius;

		double             m_startAngle;

		double             m_endAngle;

	};




	IBKMK::Vector3D															m_origin = IBKMK::Vector3D(0,0,0);

	RotationMatrix															m_rotationMatrix = RotationMatrix(QQuaternion(1.0,0.0,0.0,0.0));

	// scales in relation to meters
	double																	m_scalingFactor = 0.01;

	std::vector<Layer>                                                      m_layer;

	std::vector<Point>                                                      m_points;

	std::vector<Line>                                                       m_lines;

	std::vector<PolyLine>                                                   m_polylines;

	std::vector<Circle>                                                     m_circles;

	std::vector<Ellipse>                                                    m_ellipses;

	std::vector<Arc>                                                        m_arcs;

	void updatePointer();


private:

	Layer *findLayerPointer(const QString &layername);

};



} // namespace VICUS

#endif // VICUS_DRAWING_H
