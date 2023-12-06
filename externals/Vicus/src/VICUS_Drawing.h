#ifndef VICUS_DRAWING_H
#define VICUS_DRAWING_H

#include "tinyxml.h"

#include <IBKMK_Vector2D.h>
#include <IBKMK_Vector3D.h>
#include <IBK_Line.h>

#include "VICUS_PlaneGeometry.h"
#include "VICUS_RotationMatrix.h"
#include "VICUS_CodeGenMacros.h"
#include "VICUS_Object.h"
#include "VICUS_DrawingLayer.h"

#include <QQuaternion>
#include <QMatrix4x4>
#include <QColor>
#include <QDebug>
#include <QTransform>


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
		for (DrawingLayer & dl : m_drawingLayers) {
			m_children.push_back(&dl);
			dl.m_parent = dynamic_cast<VICUS::Object *>(this);
		}
		updatePointer();
	}

	/*! Insert structure for Blocks */
	struct Block {

		TiXmlElement * writeXML(TiXmlElement * element) const;
		void readXML(const TiXmlElement * element);

		/*! ID of Block. */
		unsigned int	m_id;
		/*! Name of Block. */
		QString			m_name;
		/*! Block color. */
		QColor			m_color = QColor();
		/*! Line weight. */
		int				m_lineWeight;
		/*! Base point */
		IBKMK::Vector2D	m_basePoint;
	};

	/*! Insert structure for blocks. */
	struct Insert {

		TiXmlElement * writeXML(TiXmlElement * element) const;
		void readXML(const TiXmlElement * element);

		unsigned int			m_id;					// unsigned int
		QString					m_currentBlockName;		// name of block
		QString					m_parentBlockName;		// name of block

		/* Block Entities that belongs to name. */
		Block					*m_currentBlock = nullptr;
		Block					*m_parentBlock = nullptr;

		double					m_xScale = 1.0;			// scale-factor in x direction
		double					m_yScale = 1.0;			// scale-factor in y direction
		double					m_zScale = 1.0;			// scale-factor in z direction

		double					m_angle = 0.0;			// angle of rotation

		IBKMK::Vector2D			m_insertionPoint;		// insertion point
	};

	/* Abstract class for all directly drawable dxf entities */
	struct AbstractDrawingObject {
		/*! Standard C'tor. */
		AbstractDrawingObject() = default;

		/*! D'tor. */
		virtual ~AbstractDrawingObject() {}

		virtual void readXML(const TiXmlElement * element) = 0;
		virtual TiXmlElement * writeXML(TiXmlElement * parent) const = 0;

		/*! Function to update points, needed for easier
			handling of objects in scene3D to construct 3D
			Objects and for picking operations.

			Point will be recalculated when m_dirty is true;
		*/
		virtual const std::vector<IBKMK::Vector2D>& points2D() const = 0 ;
		virtual const std::vector<VICUS::PlaneGeometry>& planeGeometries() const = 0;

		/* used to get correct color of entity */
		const QColor &color() const;
		/* used to get correct lineWeight of entity */
		double lineWeight() const;
		/*! Indicates when triangulation has to be redone. */
		void updateGeometry() {
			m_dirtyTriangulation = true;
		}

		/*! Get parent drawing where object is included. */
		const Drawing *drawing() const {
			Q_ASSERT(m_parentLayer != nullptr);
			const VICUS::Drawing *drawing = dynamic_cast<const VICUS::Drawing *>(m_parentLayer->m_parent);
			Q_ASSERT(drawing != nullptr);
			// Return pointer to drawing
			return drawing;
		}

		/*! name of Entity */
		QString										m_layerName;
		/*! Layer of Entity */
		const DrawingLayer							*m_parentLayer = nullptr;
		/*! Color of Entity if defined, use getter color() instead */
		QColor										m_color = QColor();
		/*! Line weight of Entity, use getter lineWeight() instead */
		double										m_lineWeight = 0;
		/* integer to create a drawing hierarchy in a dxf file to avoid overlapping of entities */
		unsigned int								m_zPosition;
		/*! Name of block. */
		QString										m_blockName;
		/* Block Entity belongs to, if nullptr, no block is used */
		Block										*m_block = nullptr;
		/*! ID of object. */
		unsigned int								m_id;
		/*! Transformation matrix. */
		QMatrix4x4									m_trans = QMatrix4x4();

	protected:
		/*! Flag to indictate recalculation of points. */
		mutable bool								m_dirtyPoints = true;
		/*! Flag to indictate recalculation triangulation. */
		mutable bool								m_dirtyTriangulation = true;
		/*! Points of objects. */
		mutable std::vector<IBKMK::Vector2D>		m_pickPoints;
		/*! Plane Geometries with all triangulated data.
		*/
		mutable std::vector<VICUS::PlaneGeometry>	m_planeGeometries;
	};


	/*! Struct that holds all data for Dimension styling.
		For now all the basic implementation is just done.
	*/
	struct DimStyle {
		/*!
			https://ezdxf.readthedocs.io/en/stable/tutorials/linear_dimension.html
		*/

		TiXmlElement * writeXML(TiXmlElement * element) const;
		void readXML(const TiXmlElement * element);

		/*! Name of Dim style. */
		QString				m_name;

		/*! ID of object. */
		unsigned int		m_id;

		/*! Fixed length of extension line. */
		bool				m_fixedExtensionLength = false;

		/*! Extension line length. */
		double				m_extensionLineLength = 0.0;

		/*! Point 2 of Line. */
		double				m_textHeight = 0.0;

		/*! Distance between measure line and uper point of
			extension line */
		double				m_upperLineDistance = 0.0;

		/*!	Distance between extension line lower point and object. */
		double				m_extensionLineLowerDistance = 0.0;
	};


	/*! Stores attributes of line */
	struct Point : public AbstractDrawingObject {

		TiXmlElement * writeXML(TiXmlElement * element) const override;
		void readXML(const TiXmlElement * element) override;

		/*! Calculate points. */
		const std::vector<IBKMK::Vector2D>& points2D() const override;
		/*! Calculate Plane geometries if m_dirtyTriangulation is true and/or returns them.
			Drawing is only needed when m_dirtyTriangulation is true
		*/
		const std::vector<VICUS::PlaneGeometry>& planeGeometries() const override;

		/*! Point coordinate */
		IBKMK::Vector2D					m_point;

	};

	/*! Stores attributes of line */
	struct Line : public AbstractDrawingObject {

		TiXmlElement * writeXML(TiXmlElement * element) const override;
		void readXML(const TiXmlElement * element) override;

		/*! Calculate points. */
		const std::vector<IBKMK::Vector2D>& points2D() const override;
		/*! Calculate Plane geometries if m_dirtyTriangulation is true and/or returns them.
			Drawing is only needed when m_dirtyTriangulation is true
		*/
		const std::vector<VICUS::PlaneGeometry>& planeGeometries() const override;

		/*! Point coordinate */
		IBKMK::Vector2D					m_point1;
		/*! Point coordinate */
		IBKMK::Vector2D					m_point2;
	};

	/*! Stores both LW and normal polyline */
	struct PolyLine : public AbstractDrawingObject {

		TiXmlElement * writeXML(TiXmlElement * element) const override;
		void readXML(const TiXmlElement * element) override;

		/*! Calculate points. */
		const std::vector<IBKMK::Vector2D>& points2D() const override;
		/*! Calculate Plane geometries if m_dirtyTriangulation is true and/or returns them.
			Drawing is only needed when m_dirtyTriangulation is true
		*/
		const std::vector<VICUS::PlaneGeometry>& planeGeometries() const override;

		/*! polyline coordinates */
		std::vector<IBKMK::Vector2D>    m_polyline;
		/*! When end point is connected to start */
		bool							m_endConnected = false;
	};

	/* Stores attributes of circle */
	struct Circle : public AbstractDrawingObject {

		TiXmlElement * writeXML(TiXmlElement * element) const override;
		void readXML(const TiXmlElement * element) override;
		/*! Calculate points. */
		const std::vector<IBKMK::Vector2D> &points2D() const override;
		/*! Calculate Plane geometries if m_dirtyTriangulation is true and/or returns them.
			Drawing is only needed when m_dirtyTriangulation is true
		*/
		const std::vector<VICUS::PlaneGeometry>& planeGeometries() const override;

		/*! Circle center */
		IBKMK::Vector2D					m_center;
		/*! Circle radius */
		double							m_radius;
	};

	/* Stores attributes of ellipse */
	struct Ellipse : public AbstractDrawingObject {

		TiXmlElement * writeXML(TiXmlElement * element) const override;
		void readXML(const TiXmlElement * element) override;

		const std::vector<IBKMK::Vector2D> &points2D() const override;
		/*! Calculate Plane geometries if m_dirtyTriangulation is true and/or returns them.
			Drawing is only needed when m_dirtyTriangulation is true
		*/
		const std::vector<VICUS::PlaneGeometry>& planeGeometries() const override;


		/*! Ellipse center */
		IBKMK::Vector2D			m_center;
		/*! Ellipse major axis */
		IBKMK::Vector2D			m_majorAxis;
		/*! Ellipse minor axis as ratio of majorAxis*/
		double					m_ratio;
		/*! Ellipse start angle */
		double					m_startAngle;
		/*! Ellipse end angle */
		double					m_endAngle;
	};

	/* Stores attributes of arc */
	struct Arc : public AbstractDrawingObject {

		TiXmlElement * writeXML(TiXmlElement * element) const override;
		void readXML(const TiXmlElement * element) override;

		const std::vector<IBKMK::Vector2D> &points2D() const override;
		/*! Calculate Plane geometries if m_dirtyTriangulation is true and/or returns them.
			Drawing is only needed when m_dirtyTriangulation is true
		*/
		const std::vector<VICUS::PlaneGeometry>& planeGeometries() const override;


		/*! Arc center */
		IBKMK::Vector2D			m_center;
		/*! Arc radius */
		double					m_radius;
		/*! Arc start angle */
		double					m_startAngle;
		/*! Arc end angle */
		double					m_endAngle;
	};

	/* Stores attributes of solid, dummy struct */
	struct Solid : public AbstractDrawingObject {

		TiXmlElement * writeXML(TiXmlElement * element) const override;
		void readXML(const TiXmlElement * element) override;

		const std::vector<IBKMK::Vector2D> &points2D() const override;
		/*! Calculate Plane geometries if m_dirtyTriangulation is true and/or returns them.
			Drawing is only needed when m_dirtyTriangulation is true
		*/
		const std::vector<VICUS::PlaneGeometry>& planeGeometries() const override;

		/*! Point 1 */
		IBKMK::Vector2D			m_point1;
		/*! Point 2 */
		IBKMK::Vector2D			m_point2;
		/*! Point 3 */
		IBKMK::Vector2D			m_point3;
		/*! Point 4 */
		IBKMK::Vector2D			m_point4;
	};

	/* Stores attributes of text, dummy struct */
	struct Text : public AbstractDrawingObject {

		TiXmlElement * writeXML(TiXmlElement * element) const override;
		void readXML(const TiXmlElement * element) override;

		const std::vector<IBKMK::Vector2D> &points2D() const override;
		/*! Calculate Plane geometries if m_dirtyTriangulation is true and/or returns them.
			Drawing is only needed when m_dirtyTriangulation is true
		*/
		const std::vector<VICUS::PlaneGeometry>& planeGeometries() const override;


		/*! Base point. */
		IBKMK::Vector2D		m_basePoint;
		/*! Height. */
		double				m_rotationAngle = 0;
		/*! Height. */
		double				m_height = 10;
		/*! Alignment. */
		Qt::Alignment		m_alignment;
		/*! Text */
		QString				m_text;
	};

	/* Stores attributes of text, dummy struct */
	struct LinearDimension : public AbstractDrawingObject {

		TiXmlElement * writeXML(TiXmlElement * element) const override;
		void readXML(const TiXmlElement * element) override;

		const std::vector<IBKMK::Vector2D> &points2D() const override;
		/*! Calculate Plane geometries if m_dirtyTriangulation is true and/or returns them.
			Drawing is only needed when m_dirtyTriangulation is true
		*/
		const std::vector<VICUS::PlaneGeometry>& planeGeometries() const override;


		/*! Base point. */
		IBKMK::Vector2D				m_dimensionPoint;

		/*! Base point. */
		mutable IBKMK::Vector2D		m_leftPoint;

		/*! Base point. */
		mutable IBKMK::Vector2D		m_rightPoint;

		/*! Point 1 of Line. */
		IBKMK::Vector2D				m_point1;

		/*! Point 2 of Line. */
		IBKMK::Vector2D				m_point2;

		/*! Point 2 of Line. */
		IBKMK::Vector2D				m_textPoint;

		/*! Angle of rotation. */
		double						m_angle = 0.0;

		/*! Name of Dim style. */
		QString						m_styleName;

		/*! Pointer to style object. Updated in
			updatePointers();
		*/
		DimStyle					*m_style = nullptr;
	};


	// *** PUBLIC MEMBER FUNCTIONS ***
	VICUS_READWRITE

	/*! Returns the drawing object based on the ID. */
	const AbstractDrawingObject* objectByID(unsigned int id) const;

	/*! used to assign the correct layer to an entity */
	void updatePointer();

	/*! Updates all planes, when transformation operations are applied.
		MIND: Always call this function, when the drawing transformation
		(translation, rotation) were changed, since the triangulation is
		redone.
	*/
	void updatePlaneGeometries();

	/*! Generates all inserting geometries. */
	void generateInsertGeometries(unsigned int &nextId);

	/*! Returns 3D Pick points of drawing. */
	const std::map<unsigned int, std::vector<IBKMK::Vector3D>> &pickPoints() const;

	/*! Generates 3D Points from 2D points by applying transformation from drawing. */
	const std::vector<IBKMK::Vector3D> points3D(const std::vector<IBKMK::Vector2D> &verts, unsigned int zPosition, const QMatrix4x4 &trans = QMatrix4x4()) const;

	/*! Returns the normal vector of the drawing. */
	const IBKMK::Vector3D normal() const;

	/*! Returns the normal vector of the drawing. */
	const IBKMK::Vector3D localX() const;

	/*! Returns the normal vector of the drawing. */
	const IBKMK::Vector3D localY() const;

	// *** PUBLIC MEMBER VARIABLES ***

	//:inherited	unsigned int		m_id = INVALID_ID;					// XML:A:required
	//:inherited	QString				m_displayName;						// XML:A
	//:inherited	bool				m_visible = true;					// XML:A

	/*! point of origin */
	IBKMK::Vector3D															m_origin			= IBKMK::Vector3D(0,0,0);						// XML:E
	/*! rotation matrix */
	RotationMatrix															m_rotationMatrix	= RotationMatrix(QQuaternion(1.0,0.0,0.0,0.0)); // XML:E
	/*! scale factor */
	double																	m_scalingFactor		= 1.0;											// XML:E

	/*! list of blocks, dummy implementation */
	std::vector<Block>														m_blocks;			// XML:E
	/*! list of layers */
	std::vector<DrawingLayer>												m_drawingLayers;	// XML:E
	/*! list of points */
	std::vector<Point>														m_points;			// XML:E
	/*! list of lines */
	std::vector<Line>														m_lines;			// XML:E
	/*! list of polylines */
	std::vector<PolyLine>													m_polylines;		// XML:E
	/*! list of circles */
	std::vector<Circle>														m_circles;			// XML:E
	/*! list of ellipses */
	std::vector<Ellipse>													m_ellipses;			// XML:E
	/*! list of arcs */
	std::vector<Arc>														m_arcs;				// XML:E
	/*! list of solids, dummy implementation */
	std::vector<Solid>														m_solids;			// XML:E
	/*! list of texts */
	std::vector<Text>														m_texts;			// XML:E
	/*! list of texts */
	std::vector<LinearDimension>											m_linearDimensions;	// XML:E
	/*! list of Dim Styles */
	std::vector<DimStyle>													m_dimensionStyles;	// XML:E
	/*! list of inserts. */
	std::vector<Insert>														m_inserts;

	/*! Counter of entities, used to create a drawing hierarchy
		in a dxf file to avoid overlapping of entities */
	unsigned int															m_zCounter = 0;	// XML:E

	/*! Is the default color when no other color was specified */
	QColor																	m_defaultColor = QColor(); // XML:E

private:
	/*! Helper function to assign the correct layer to an entity */
	DrawingLayer *findLayerPointer(const QString &layername);

	/*! Helper function to assign the correct block to an entity */
	Block* findBlockPointer(const QString &name);

	/*! Transforms all inserts. */
	void transformInsert(QMatrix4x4 &trans, const VICUS::Drawing::Insert &insert, unsigned int &nextId);

	/*! Function to generate plane geometries from a line. */
	bool generatePlaneFromLine(const IBKMK::Vector3D &startPoint, const IBKMK::Vector3D &endPoint,
							   double width, VICUS::PlaneGeometry &plane,
							   const QMatrix4x4 &trans) const;

	/*! Function to generate plane geometries from a polyline. */
	bool generatePlanesFromPolyline(const std::vector<IBKMK::Vector3D> & polyline,
									bool connectEndStart, double width, std::vector<PlaneGeometry> &planes,
									const QMatrix4x4 &trans) const;

	/*! Function to generate plane geometries from text. Heavy operation. Text is polygonised by QPainterPath with font-size 1
		in order to get a rough letter and less polygons. Some dxfs contain a lot of text and so we would end in having too many
		polygons.
	*/
	void generatePlanesFromText(const std::string &text, double textSize, Qt::Alignment alignment, const double &rotationAngle,
								const IBKMK::Vector2D &basePoint, double zPositon,
								std::vector<VICUS::PlaneGeometry> &planeGeometries,
								const QMatrix4x4 &trans) const;


	/*! Cached unique-ID -> object ptr map. Greatly speeds up objectByID() and any other lookup functions.
		This map is updated in updatePointers().
	*/
	std::map<unsigned int, VICUS::Drawing::AbstractDrawingObject*>	m_objectPtr;

	/*! Cached pick points of drawing.
		\param Key is ID of drawing object, to get better referencing in picking.
		\param Value is vector with 3D pick points
	*/
	mutable std::map<unsigned int, std::vector<IBKMK::Vector3D>>	m_pickPoints;

	/*! Mark if pick points have to be recalculated. */
	mutable bool													m_dirtyPickPoints = true;

};

} // namespace VICUS



#endif // VICUS_DRAWING_H
