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

		// TODO Maik: dokustring
		/*! here */
		QString			m_name;

		QColor			m_color = QColor(255, 255, 255);

		double			m_lineWidth;

		bool			m_visible;

	};

	struct AbstractDrawingObject {

		QString		        m_layername;

		const Layer         *m_parentLayer;

		QColor				m_color = QColor(255, 255, 255);

		double				m_lineWidth = 0;

		const QColor*		color() const;

		double				lineWidth() const;
	};

	struct Point : public AbstractDrawingObject {

		IBKMK::Vector2D m_point;
	};

	struct Line : public AbstractDrawingObject {

		IBK::Line       m_line;

	};

	// TODO Maik: remove
	struct LWPolyLine : public AbstractDrawingObject {

		std::vector<IBKMK::Vector2D>    m_lwpolyline;

		// true if the end point connected to the start
		int                            m_polyline_flag = 0;

	};

	/*! Stores both LW and normal polyline */
	struct PolyLine : public AbstractDrawingObject {

		std::vector<IBKMK::Vector2D>    m_polyline;

		// true if the end point connected to the start
		int                            m_polyline_flag = 0;

	};

	IBKMK::Vector3D															m_origin = IBKMK::Vector3D(0,0,0);

	RotationMatrix															m_rotationMatrix;

	// scales in relation to meters
	double																	m_scalingFactor = 1;

	std::vector<Layer>                                                      m_layer;

	std::vector<Point>                                                      m_points;

	std::vector<Line>                                                       m_lines;

	std::vector<PolyLine>                                                   m_polylines;

	// TODO Maik: remove
	std::vector<LWPolyLine>                                                 m_lwpolylines;

	void updatePointer();


private:

	Layer *findLayerPointer(const QString &layername);

};



} // namespace VICUS

#endif // VICUS_DRAWING_H
