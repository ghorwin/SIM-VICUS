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

	/*! Dummy Struct for Blocks */
	struct Block {

		QString			m_name;

		QColor m_color = QColor();

		int m_lineWeight;

	};

	/*! Layer struct with relevant attributes */
	struct Layer {

		// TODO Maik: dokustring
		/*! Name of layer */
		QString			m_name;
		/*! Color of layer if defined */
		QColor			m_color = QColor();
		/*! Line weight of layer if defined */
		int				m_lineWeight;
		/*! Visibility */
		bool			m_visible;
		/*! If Layer belongs to a block, pointer to block is here, else nullptr */
		Block			*m_block = nullptr;

	};

	/* Abstract class for all directly drawable dxf entities */
	struct AbstractDrawingObject {
		/*! name of Entity */
		QString		        m_layername;
		/*! Layer of Entity */
		const Layer         *m_parentLayer;
		/*! Color of Entity if defined, use getter color() instead */
		QColor				m_color = QColor();
		/*! Line weight of Entity, use getter lineWeight() instead */
		double				m_lineWeight = 0;
		/* used to get correct color of entity */
		const QColor*		color() const;
		/* used to get correct lineWeight of entity */
		double				lineWeight() const;
		/* integer to create a drawing hierarchy in a dxf file to avoid overlapping of entities */
		int					m_zposition;
		/* Block Entity belongs to, if nullptr, no block is used */
		Block				*m_block = nullptr;

	};

	/*! Stores attributes of line */
	struct Point : public AbstractDrawingObject {
		/*! Point coordinate */
		IBKMK::Vector2D m_point;
	};

	/*! Stores attributes of line */
	struct Line : public AbstractDrawingObject {
		/*! line coordinates */
		IBK::Line       m_line;

	};

	/*! Stores both LW and normal polyline */
	struct PolyLine : public AbstractDrawingObject {
		/*! polyline coordinates */
		std::vector<IBKMK::Vector2D>    m_polyline;
		/*! 1 if the end point connected to the start */
		int                            m_polyline_flag = 0;

	};

	/* Stores attributes of circle */
	struct Circle : public AbstractDrawingObject {
		/*! Circle center */
		IBKMK::Vector2D    m_center;
		/*! Circle radius */
		double             m_radius;
	};

	/* Stores attributes of ellipse */
	struct Ellipse : public AbstractDrawingObject {
		/*! Ellipse center */
		IBKMK::Vector2D    m_center;
		/*! Ellipse major axis */
		IBKMK::Vector2D    m_majorAxis;
		/*! Ellipse minor axis as ratio of majorAxis*/
		double             m_ratio;
		/*! Ellipse start angle */
		double             m_startAngle;
		/*! Ellipse end angle */
		double             m_endAngle;

	};

	/* Stores attributes of arc */
	struct Arc : public AbstractDrawingObject {
		/*! Arc center */
		IBKMK::Vector2D    m_center;
		/*! Arc radius */
		double             m_radius;
		/*! Arc start angle */
		double             m_startAngle;
		/*! Arc end angle */
		double             m_endAngle;

	};

	/* Stores attributes of solid, dummy struct */
	struct Solid : public AbstractDrawingObject {

		IBKMK::Vector2D    m_point1;

		IBKMK::Vector2D    m_point2;

		IBKMK::Vector2D    m_point3;

		IBKMK::Vector2D    m_point4;

	};

	/*! point of origin */
	IBKMK::Vector3D															m_origin = IBKMK::Vector3D(0,0,0);
	/*! rotation matrix */
	RotationMatrix															m_rotationMatrix = RotationMatrix(QQuaternion(1.0,0.0,0.0,0.0));
	/*! scale factor */
	double																	m_scalingFactor = 0.01;
	/*! list of blocks, dummy implementation */
	std::vector<Block>                                                      m_blocks;
	/*! list of layers */
	std::vector<Layer>                                                      m_layer;
	/*! list of points */
	std::vector<Point>                                                      m_points;
	/*! list of lines */
	std::vector<Line>                                                       m_lines;
	/*! list of polylines */
	std::vector<PolyLine>                                                   m_polylines;
	/*! list of circles */
	std::vector<Circle>                                                     m_circles;
	/*! list of ellipses */
	std::vector<Ellipse>                                                    m_ellipses;
	/*! list of arcs */
	std::vector<Arc>                                                        m_arcs;
	/*! list of solids, dummy implementation */
	std::vector<Solid>                                                      m_solids;
	/*! Counter of entities, used to create a drawing hierarchy
	 *   in a dxf file to avoid overlapping of entities */
	int																		m_zcounter = 0;
	/* used to assign the correct layer to an entity */
	void updatePointer();


private:
	/*! Helper function to assign the correct layer to an entity */
	Layer *findLayerPointer(const QString &layername);

};



} // namespace VICUS

#endif // VICUS_DRAWING_H
