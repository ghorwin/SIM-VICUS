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
#include "VICUS_Object.h"

#include <QColor>
#include <QDebug>


namespace VICUS {

/*!
	Drawing class is data structure for all primitive drawing objects
	such as Layer, Lines, Circles, ... .
 */
class Drawing : public Object {

public:

	/*! Type-info string. */
	const char * typeinfo() const override {
		return "Drawing";
	}

	// *** PUBLIC MEMBER FUNCTIONS ***

	Drawing();

	//VICUS_COMPARE_WITH_ID

	void updateParents() {
		m_children.clear();
		for (DrawingLayer & dl : m_layers) {
			m_children.push_back(&dl);
			dl.m_parent = this;
		}
		updatePointer();
	}

	/*! Dummy Struct for Blocks */
	struct Block {
		/*! Name of Block. */
		QString			m_name;
		/*! Block color. */
		QColor			m_color = QColor();
		/*! Line weight. */
		unsigned		m_lineWeight;
	};

	/*! Layer struct with relevant attributes */
	struct DrawingLayer : public Object {

		DrawingLayer() {}

		const char * typeinfo() const override {
			return "DrawingLayer";
		}

		// VICUS_COMPARE_WITH_ID

		// TODO Maik: dokustring

		/*! Color of layer if defined */
		QColor			m_color = QColor();
		/*! Line weight of layer if defined */
		int				m_lineWeight;
		/*! If Layer belongs to a block, pointer to block is here, else nullptr */
		Block			*m_block = nullptr;

	};

	/* Abstract class for all directly drawable dxf entities */
	struct AbstractDrawingObject {
		/* used to get correct color of entity */
		const QColor &color() const;
		/* used to get correct lineWeight of entity */
		double lineWeight() const;

		/*! name of Entity */
		QString						m_layerName;
		/*! Layer of Entity */
		const DrawingLayer			*m_parentLayer = nullptr;
		/*! Color of Entity if defined, use getter color() instead */
		QColor						m_color = QColor();
		/*! Line weight of Entity, use getter lineWeight() instead */
		double						m_lineWeight = 0;
		/* integer to create a drawing hierarchy in a dxf file to avoid overlapping of entities */
		unsigned int				m_zPosition;
		/* Block Entity belongs to, if nullptr, no block is used */
		Block						*m_block = nullptr;

		/*! ID of object. */
		unsigned int				m_id;

		/*! Points of objects. */
		std::vector<IBKMK::Vector2D>	m_points;
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
		/*! When end point is connected to start */
		bool							m_endConnected = false;
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

		/*! Point 1 */
		IBKMK::Vector2D    m_point1;
		/*! Point 2 */
		IBKMK::Vector2D    m_point2;
		/*! Point 3 */
		IBKMK::Vector2D    m_point3;
		/*! Point 4 */
		IBKMK::Vector2D    m_point4;
	};

	/*! Returns the drawing object based on the ID. */
	const AbstractDrawingObject* objectByID(unsigned int id) const;

	/*! point of origin */
	IBKMK::Vector3D															m_origin = IBKMK::Vector3D(0,0,0);
	/*! rotation matrix */
	RotationMatrix															m_rotationMatrix = RotationMatrix(QQuaternion(1.0,0.0,0.0,0.0));
	/*! scale factor */
	double																	m_scalingFactor = 0.01;
	/*! list of blocks, dummy implementation */
	std::vector<Block>														m_blocks;
	/*! list of layers */
	std::vector<DrawingLayer>												m_layers;
	/*! list of points */
	std::vector<Point>														m_points;
	/*! list of lines */
	std::vector<Line>														m_lines;
	/*! list of polylines */
	std::vector<PolyLine>													m_polylines;
	/*! list of circles */
	std::vector<Circle>														m_circles;
	/*! list of ellipses */
	std::vector<Ellipse>													m_ellipses;
	/*! list of arcs */
	std::vector<Arc>														m_arcs;
	/*! list of solids, dummy implementation */
	std::vector<Solid>														m_solids;
	/*! Counter of entities, used to create a drawing hierarchy
		in a dxf file to avoid overlapping of entities */
	unsigned int															m_zCounter = 0;
	/*! Is the default color when no other color was specified */
	QColor																	m_defaultColor = QColor();
	/*! used to assign the correct layer to an entity */
	void updatePointer();

private:
	/*! Helper function to assign the correct layer to an entity */
	DrawingLayer *findLayerPointer(const QString &layername);

	/*! Cached unique-ID -> object ptr map. Greatly speeds up objectByID() and any other lookup functions.
		This map is updated in updatePointers().
	*/
	std::map<unsigned int, VICUS::Drawing::AbstractDrawingObject*>			m_objectPtr;

};

} // namespace VICUS



#endif // VICUS_DRAWING_H
