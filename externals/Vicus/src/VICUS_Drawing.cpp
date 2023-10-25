#include "VICUS_Drawing.h"
#include "IBKMK_3DCalculations.h"
#include "VICUS_Constants.h"

#include "IBK_MessageHandler.h"
#include "IBK_messages.h"
#include "IBK_physics.h"

#include "NANDRAD_Utilities.h"

#include "qfont.h"
#include "qpainterpath.h"
#include "tinyxml.h"

/*! IBKMK::Vector3D to QVector3D conversion macro. */
inline QVector3D IBKVector2QVector(const IBKMK::Vector3D & v) {
	return QVector3D((float)v.m_x, (float)v.m_y, (float)v.m_z);
}

/*! QVector3D to IBKMK::Vector3D to conversion macro. */
inline IBKMK::Vector3D QVector2IBKVector(const QVector3D & v) {
	return IBKMK::Vector3D((double)v.x(), (double)v.y(), (double)v.z());
}

namespace VICUS {

Drawing::Drawing() :
	m_blocks(std::vector<Block>()),
	m_drawingLayers(std::vector<DrawingLayer>()),
	m_points(std::vector<Point>()),
	m_lines(std::vector<Line>()),
	m_polylines(std::vector<PolyLine>()),
	m_circles(std::vector<Circle>()),
	m_ellipses(std::vector<Ellipse>()),
	m_arcs(std::vector<Arc>()),
	m_solids(std::vector<Solid>()),
	m_texts(std::vector<Text>())
{}


TiXmlElement * Drawing::Text::writeXML(TiXmlElement * parent) const {
	if (m_id == VICUS::INVALID_ID)  return nullptr;

	TiXmlElement * e = new TiXmlElement("Text");
	parent->LinkEndChild(e);

	if (m_id != VICUS::INVALID_ID)
		e->SetAttribute("id", IBK::val2string<unsigned int>(m_id));
	if (m_color.isValid())
		e->SetAttribute("color", m_color.name().toStdString());
	if (m_zPosition != 0.0)
		e->SetAttribute("zPosition", IBK::val2string<unsigned int>(m_zPosition));
	if (!m_text.isEmpty())
		e->SetAttribute("text", m_text.toStdString());
	if (!m_layerName.isEmpty())
		e->SetAttribute("layer", m_layerName.toStdString());
	if (m_height != 10.0)
		e->SetAttribute("height", IBK::val2string<double>(m_height));

	TiXmlElement::appendSingleAttributeElement(e, "BasePoint", nullptr, std::string(), m_basePoint.toString());

	return e;
}

void Drawing::Text::readXML(const TiXmlElement *element){
	FUNCID(Drawing::Text::readXMLPrivate);

	try {
		// search for mandatory attributes
		if (!TiXmlAttribute::attributeByName(element, "id"))
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
									  IBK::FormatString("Missing required 'id' attribute.") ), FUNC_ID);

		const TiXmlAttribute * attrib = element->FirstAttribute();
		while (attrib) {
			const std::string & attribName = attrib->NameStr();
			if (attribName == "id")
				m_id = NANDRAD::readPODAttributeValue<unsigned int>(element, attrib);
			else if (attribName == "color")
				m_color = QColor(QString::fromStdString(attrib->ValueStr()));
			else if (attribName == "zPosition")
				m_zPosition = NANDRAD::readPODAttributeValue<unsigned int>(element, attrib);
			else if (attribName == "text")
				m_text = QString::fromStdString(attrib->ValueStr());
			else if (attribName == "layer")
				m_layerName = QString::fromStdString(attrib->ValueStr());
			else if (attribName == "height")
				m_height = NANDRAD::readPODAttributeValue<double>(element, attrib);
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ATTRIBUTE).arg(attribName).arg(element->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			attrib = attrib->Next();
		}

		// reading elements
		const TiXmlElement * c = element->FirstChildElement();
		while (c) {
			const std::string & cName = c->ValueStr();
			if (cName == "BasePoint") {
				try {
					m_basePoint = IBKMK::Vector2D::fromString(c->GetText());
				} catch (IBK::Exception & ex) {
					throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(c->Row())
										  .arg("Invalid vector data."), FUNC_ID);
				}
			}
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			c = c->NextSiblingElement();
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'Drawing::Text' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'Drawing::Text' element.").arg(ex2.what()), FUNC_ID);
	}
}

const std::vector<IBKMK::Vector2D> &Drawing::Text::points2D() const {
	if (m_dirtyPoints) {
		m_pickPoints.clear();
		m_pickPoints.push_back(m_basePoint);
	}
	return m_pickPoints;
}

const std::vector<PlaneGeometry> &Drawing::Text::planeGeometries(const Drawing &drawing) const {
	FUNCID(Drawing::Text::planeGeometries);
	try {
		if (m_dirtyTriangulation) {
			m_planeGeometries.clear();

			generatePlanesFromText(m_text.toStdString(), m_height, m_alignment, m_rotationAngle, drawing.m_rotationMatrix,
								   drawing.m_origin, m_basePoint, drawing.m_scalingFactor, m_zPosition * Z_MULTIPLYER,
								   m_planeGeometries, m_trans);

			//	if (!success)
			//		throw IBK::Exception(IBK::FormatString("Could not generate plane geometry for Drawing Element #%1.").arg(m_id), FUNC_ID);

			m_dirtyTriangulation = false;
		}

		return m_planeGeometries;
	} catch (IBK::Exception &ex) {
		throw IBK::Exception(IBK::FormatString("Could not generate plane geometries of 'Drawing::Text'\n%1").arg(ex.what()), FUNC_ID);
	}

}


TiXmlElement * Drawing::Solid::writeXML(TiXmlElement * parent) const {
	if (m_id == VICUS::INVALID_ID)  return nullptr;

	TiXmlElement * e = new TiXmlElement("Solid");
	parent->LinkEndChild(e);

	if (m_id != VICUS::INVALID_ID)
		e->SetAttribute("id", IBK::val2string<unsigned int>(m_id));
	if (m_color.isValid())
		e->SetAttribute("color", m_color.name().toStdString());
	if (m_zPosition != 0.0)
		e->SetAttribute("zPosition", IBK::val2string<unsigned int>(m_zPosition));
	if (!m_layerName.isEmpty())
		e->SetAttribute("layer", m_layerName.toStdString());

	TiXmlElement::appendSingleAttributeElement(e, "Point1", nullptr, std::string(), m_point1.toString());
	TiXmlElement::appendSingleAttributeElement(e, "Point2", nullptr, std::string(), m_point2.toString());
	TiXmlElement::appendSingleAttributeElement(e, "Point3", nullptr, std::string(), m_point3.toString());
	TiXmlElement::appendSingleAttributeElement(e, "Point4", nullptr, std::string(), m_point4.toString());

	return e;
}

void Drawing::Solid::readXML(const TiXmlElement *element){
	FUNCID(Drawing::Solid::readXMLPrivate);

	try {
		// search for mandatory attributes
		if (!TiXmlAttribute::attributeByName(element, "id"))
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
									  IBK::FormatString("Missing required 'id' attribute.") ), FUNC_ID);

		const TiXmlAttribute * attrib = element->FirstAttribute();
		while (attrib) {
			const std::string & attribName = attrib->NameStr();
			if (attribName == "id")
				m_id = NANDRAD::readPODAttributeValue<unsigned int>(element, attrib);
			else if (attribName == "color")
				m_color = QColor(QString::fromStdString(attrib->ValueStr()));
			else if (attribName == "zPosition")
				m_zPosition = NANDRAD::readPODAttributeValue<unsigned int>(element, attrib);
			else if (attribName == "layer")
				m_layerName = QString::fromStdString(attrib->ValueStr());
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ATTRIBUTE).arg(attribName).arg(element->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			attrib = attrib->Next();
		}

		// reading elements
		const TiXmlElement * c = element->FirstChildElement();
		while (c) {
			const std::string & cName = c->ValueStr();
			if (cName == "Point1") {
				try {
					m_point1 = IBKMK::Vector2D::fromString(c->GetText());
				} catch (IBK::Exception & ex) {
					throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(c->Row())
										  .arg("Invalid vector data."), FUNC_ID);
				}
			}
			else if (cName == "Point2") {
				try {
					m_point2 = IBKMK::Vector2D::fromString(c->GetText());
				} catch (IBK::Exception & ex) {
					throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(c->Row())
										  .arg("Invalid vector data."), FUNC_ID);
				}
			}
			else if (cName == "Point3") {
				try {
					m_point3 = IBKMK::Vector2D::fromString(c->GetText());
				} catch (IBK::Exception & ex) {
					throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(c->Row())
										  .arg("Invalid vector data."), FUNC_ID);
				}
			}
			else if (cName == "Point4") {
				try {
					m_point4 = IBKMK::Vector2D::fromString(c->GetText());
				} catch (IBK::Exception & ex) {
					throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(c->Row())
										  .arg("Invalid vector data."), FUNC_ID);
				}
			}
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			c = c->NextSiblingElement();
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'Drawing::Solid' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'Drawing::Solid' element.").arg(ex2.what()), FUNC_ID);
	}
}

const std::vector<IBKMK::Vector2D> &Drawing::Solid::points2D() const {
	return m_pickPoints;
}

const std::vector<PlaneGeometry> &Drawing::Solid::planeGeometries(const Drawing &drawing) const {
	FUNCID(Drawing::Line::planeGeometries);

	if (m_dirtyTriangulation) {
		m_planeGeometries.clear();

		IBKMK::Vector3D p1 = IBKMK::Vector3D(drawing.m_scalingFactor * m_point1.m_x,
											 drawing.m_scalingFactor * m_point1.m_y,
											 m_zPosition * Z_MULTIPLYER);
		IBKMK::Vector3D p2 = IBKMK::Vector3D(drawing.m_scalingFactor * m_point2.m_x,
											 drawing.m_scalingFactor * m_point2.m_y,
											 m_zPosition * Z_MULTIPLYER);
		IBKMK::Vector3D p3 = IBKMK::Vector3D(drawing.m_scalingFactor * m_point3.m_x,
											 drawing.m_scalingFactor * m_point3.m_y,
											 m_zPosition * Z_MULTIPLYER);
		IBKMK::Vector3D p4 = IBKMK::Vector3D(drawing.m_scalingFactor * m_point4.m_x,
											 drawing.m_scalingFactor * m_point4.m_y,
											 m_zPosition * Z_MULTIPLYER);

		QVector3D vec1 = drawing.m_rotationMatrix.toQuaternion() * IBKVector2QVector(p1);
		QVector3D vec2 = drawing.m_rotationMatrix.toQuaternion() * IBKVector2QVector(p2);
		QVector3D vec3 = drawing.m_rotationMatrix.toQuaternion() * IBKVector2QVector(p3);
		QVector3D vec4 = drawing.m_rotationMatrix.toQuaternion() * IBKVector2QVector(p4);

		vec1 += IBKVector2QVector(drawing.m_origin);
		vec2 += IBKVector2QVector(drawing.m_origin);
		vec3 += IBKVector2QVector(drawing.m_origin);
		vec4 += IBKVector2QVector(drawing.m_origin);

		std::vector<IBKMK::Vector3D> vertexes(4);
		vertexes[0] = QVector2IBKVector(vec1);
		vertexes[1] = QVector2IBKVector(vec2);
		vertexes[2] = QVector2IBKVector(vec3);
		vertexes[3] = QVector2IBKVector(vec4);


		for (IBKMK::Vector3D &v3D : vertexes)
			v3D = QVector2IBKVector(m_trans * IBKVector2QVector(v3D));


		IBKMK::Polygon3D p(vertexes);
		m_planeGeometries.push_back(VICUS::PlaneGeometry(p));

		//		if (!success)
		//			throw IBK::Exception(IBK::FormatString("Could not generate plane geometry for Drawing Element #%1.").arg(m_id), FUNC_ID);

		m_dirtyTriangulation = false;
	}

	return m_planeGeometries;
}

TiXmlElement * Drawing::LinearDimension::writeXML(TiXmlElement * parent) const {
	if (m_id == VICUS::INVALID_ID)  return nullptr;

	TiXmlElement * e = new TiXmlElement("LinearDimension");
	parent->LinkEndChild(e);

	if (m_id != VICUS::INVALID_ID)
		e->SetAttribute("id", IBK::val2string<unsigned int>(m_id));
	if (m_color.isValid())
		e->SetAttribute("color", m_color.name().toStdString());
	if (m_zPosition != 0.0)
		e->SetAttribute("zPosition", IBK::val2string<unsigned int>(m_zPosition));
	if (m_angle != 0.0)
		e->SetAttribute("angle", IBK::val2string<double>(m_angle));
	if (!m_layerName.isEmpty())
		e->SetAttribute("layer", m_layerName.toStdString());
	if (!m_styleName.isEmpty())
		e->SetAttribute("styleName", m_styleName.toStdString());

	TiXmlElement::appendSingleAttributeElement(e, "Point1", nullptr, std::string(), m_point1.toString());
	TiXmlElement::appendSingleAttributeElement(e, "Point2", nullptr, std::string(), m_point2.toString());
	TiXmlElement::appendSingleAttributeElement(e, "DimensionPoint", nullptr, std::string(), m_dimensionPoint.toString());
	TiXmlElement::appendSingleAttributeElement(e, "LeftPoint", nullptr, std::string(), m_leftPoint.toString());
	TiXmlElement::appendSingleAttributeElement(e, "RightPoint", nullptr, std::string(), m_rightPoint.toString());
	TiXmlElement::appendSingleAttributeElement(e, "TextPoint", nullptr, std::string(), m_textPoint.toString());

	return e;
}

void Drawing::LinearDimension::readXML(const TiXmlElement *element){
	FUNCID(Drawing::LinearDimension::readXMLPrivate);

	try {
		// search for mandatory attributes
		if (!TiXmlAttribute::attributeByName(element, "id"))
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
									  IBK::FormatString("Missing required 'id' attribute.") ), FUNC_ID);

		const TiXmlAttribute * attrib = element->FirstAttribute();
		while (attrib) {
			const std::string & attribName = attrib->NameStr();
			if (attribName == "id")
				m_id = NANDRAD::readPODAttributeValue<unsigned int>(element, attrib);
			else if (attribName == "color")
				m_color = QColor(QString::fromStdString(attrib->ValueStr()));
			else if (attribName == "zPosition")
				m_zPosition = NANDRAD::readPODAttributeValue<unsigned int>(element, attrib);
			else if (attribName == "angle")
				m_zPosition = NANDRAD::readPODAttributeValue<double>(element, attrib);
			else if (attribName == "layer")
				m_layerName = QString::fromStdString(attrib->ValueStr());
			else if (attribName == "styleName")
				m_styleName = QString::fromStdString(attrib->ValueStr());
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ATTRIBUTE).arg(attribName).arg(element->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			attrib = attrib->Next();
		}

		// reading elements
		const TiXmlElement * c = element->FirstChildElement();
		while (c) {
			const std::string & cName = c->ValueStr();
			if (cName == "Point1") {
				try {
					m_point1 = IBKMK::Vector2D::fromString(c->GetText());
				} catch (IBK::Exception & ex) {
					throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(c->Row())
										  .arg("Invalid vector data."), FUNC_ID);
				}
			}
			else if (cName == "Point2") {
				try {
					m_point2 = IBKMK::Vector2D::fromString(c->GetText());
				} catch (IBK::Exception & ex) {
					throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(c->Row())
										  .arg("Invalid vector data."), FUNC_ID);
				}
			}
			else if (cName == "DimensionPoint") {
				try {
					m_dimensionPoint = IBKMK::Vector2D::fromString(c->GetText());
				} catch (IBK::Exception & ex) {
					throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(c->Row())
										  .arg("Invalid vector data."), FUNC_ID);
				}
			}
			else if (cName == "LeftPoint") {
				try {
					m_leftPoint = IBKMK::Vector2D::fromString(c->GetText());
				} catch (IBK::Exception & ex) {
					throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(c->Row())
										  .arg("Invalid vector data."), FUNC_ID);
				}
			}
			else if (cName == "RightPoint") {
				try {
					m_rightPoint = IBKMK::Vector2D::fromString(c->GetText());
				} catch (IBK::Exception & ex) {
					throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(c->Row())
										  .arg("Invalid vector data."), FUNC_ID);
				}
			}

			else if (cName == "TextPoint") {
				try {
					m_textPoint = IBKMK::Vector2D::fromString(c->GetText());
				} catch (IBK::Exception & ex) {
					throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(c->Row())
										  .arg("Invalid vector data."), FUNC_ID);
				}
			}
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			c = c->NextSiblingElement();
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'Drawing::LinearDimension' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'Drawing::LinearDimension' element.").arg(ex2.what()), FUNC_ID);
	}
}

const std::vector<IBKMK::Vector2D> &Drawing::LinearDimension::points2D() const {
	// special handling
	// is populated in planeGeometries
	return m_pickPoints;
}

const std::vector<PlaneGeometry> &Drawing::LinearDimension::planeGeometries(const Drawing &drawing) const {
	FUNCID(Drawing::Text::planeGeometries);

	if (m_dirtyTriangulation) {
		m_planeGeometries.clear();

		if ((m_point1 - m_point2).magnitudeSquared() < 1E-2)
			return m_planeGeometries;

		// Create Vector from start and end point of the line, add point of origin to each coordinate and calculate z value
		double zCoordinate = m_zPosition * Z_MULTIPLYER;

		if (m_leftPoint == IBKMK::Vector2D() ||
				m_rightPoint == IBKMK::Vector2D() ) {

			IBKMK::Vector2D xAxis(1, 0);
			IBKMK::Vector2D lineVec (std::cos(m_angle * IBK::DEG2RAD),
									 std::sin(m_angle * IBK::DEG2RAD));

			IBKMK::Vector2D lineVec2 (lineVec.m_y, -lineVec.m_x);

			const unsigned int SCALING_FACTOR = 1E6;

			IBKMK::Vector2D measurePoint1 = m_point1 + SCALING_FACTOR * lineVec2;
			IBKMK::Vector2D measurePoint2 = m_point2 + SCALING_FACTOR * lineVec2;

			IBK::Line lineMeasure (m_dimensionPoint -   SCALING_FACTOR * lineVec,
								   m_dimensionPoint + 2*SCALING_FACTOR * lineVec);

			IBK::Line lineLeft  (m_point1 - SCALING_FACTOR * lineVec2, measurePoint1);
			IBK::Line lineRight (m_point2 - SCALING_FACTOR * lineVec2, measurePoint2);

			IBKMK::Vector2D intersection1Left, intersection2Left;
			IBKMK::Vector2D intersection1Right, intersection2Right;
			bool intersect1 = lineMeasure.intersects(lineLeft, intersection1Left, intersection2Left) == 1;
			bool intersect2 = lineMeasure.intersects(lineRight, intersection1Right, intersection2Right) == 1;

			if (!intersect1 && !intersect2)
				throw IBK::Exception();

			IBKMK::Vector2D leftPoint, rightPoint;
			IBKMK::Vector2D point1, point2;
			bool left = false, right = false;
			if (intersect1 && (m_dimensionPoint - intersection1Left).magnitudeSquared() > 1E-3 ) {
				m_leftPoint = intersection1Left;
				m_rightPoint = m_dimensionPoint;
				left = true;
			}
			if (intersect2 && (m_dimensionPoint - intersection1Right).magnitudeSquared() > 1E-3 ) {
				m_leftPoint = m_dimensionPoint;
				m_rightPoint = intersection1Right;
				right = true;
			}

			if (!left && !right)
				return m_planeGeometries;
		}

		//		qDebug() << "Left point: X " << m_leftPoint.m_x << " Y " << m_leftPoint.m_y;
		//		qDebug() << "Right point: X " << m_rightPoint.m_x << " Y " << m_rightPoint.m_y;

		m_pickPoints.push_back(m_leftPoint);
		m_pickPoints.push_back(m_rightPoint);

		// MEASURE LINE ================================================================

		IBKMK::Vector3D p1 = IBKMK::Vector3D(drawing.m_scalingFactor * m_leftPoint.m_x,
											 drawing.m_scalingFactor * m_leftPoint.m_y,
											 zCoordinate);

		IBKMK::Vector3D p2 = IBKMK::Vector3D(drawing.m_scalingFactor * m_rightPoint.m_x,
											 drawing.m_scalingFactor * m_rightPoint.m_y,
											 zCoordinate);


		// rotate Vectors
		QVector3D vec1 = drawing.m_rotationMatrix.toQuaternion() * IBKVector2QVector(p1);
		QVector3D vec2 = drawing.m_rotationMatrix.toQuaternion() * IBKVector2QVector(p2);

		// move by origin
		vec1 += IBKVector2QVector(drawing.m_origin);
		vec2 += IBKVector2QVector(drawing.m_origin);

		m_planeGeometries.push_back(PlaneGeometry());
		bool success = generatePlaneFromLine(QVector2IBKVector(vec1), QVector2IBKVector(vec2), drawing.m_rotationMatrix,
											 DEFAULT_LINE_WEIGHT + lineWeight() * DEFAULT_LINE_WEIGHT_SCALING,
											 m_planeGeometries.back(), m_trans);

		if (!success)
			return m_planeGeometries;

		// LINE LEFT  ================================================================

		IBKMK::Vector3D l (m_leftPoint - m_point1);
		IBKMK::Vector3D point;
		if (m_style->m_fixedExtensionLength) {
			IBKMK::Vector3D ext = drawing.m_scalingFactor * m_style->m_fixedExtensionLength * l.normalized();
			point.m_x = m_leftPoint.m_x - ext.m_x;
			point.m_y = m_leftPoint.m_y - ext.m_y;
		}
		else {
			IBKMK::Vector3D ext = drawing.m_scalingFactor * m_style->m_extensionLineLowerDistance * l.normalized();
			point.m_x = m_point1.m_x + ext.m_x;
			point.m_y = m_point1.m_y + ext.m_y;
		}

		IBKMK::Vector3D p1Left = IBKMK::Vector3D(drawing.m_scalingFactor * point.m_x,
												 drawing.m_scalingFactor * point.m_y,
												 zCoordinate);
		IBKMK::Vector3D lowerExtension = drawing.m_scalingFactor * m_style->m_upperLineDistance * l.normalized();
		IBKMK::Vector3D p2Left = IBKMK::Vector3D(drawing.m_scalingFactor * (m_leftPoint.m_x + lowerExtension.m_x),
												 drawing.m_scalingFactor * (m_leftPoint.m_y + lowerExtension.m_y),
												 zCoordinate);

		// rotate Vectors
		QVector3D vec1Left = drawing.m_rotationMatrix.toQuaternion() * IBKVector2QVector(p1Left);
		QVector3D vec2Left = drawing.m_rotationMatrix.toQuaternion() * IBKVector2QVector(p2Left);

		// move by origin
		vec1Left += IBKVector2QVector(drawing.m_origin);
		vec2Left += IBKVector2QVector(drawing.m_origin);

		//		QQuaternion q = QQuaternion::fromAxisAndAngle(QVector3D(vec1Left.x(), vec1Left.y(), 1), linDem.m_angle);
		//		vec2Left = q * vec2Left;

		m_planeGeometries.push_back(PlaneGeometry());
		success = generatePlaneFromLine(QVector2IBKVector(vec1Left), QVector2IBKVector(vec2Left), drawing.m_rotationMatrix,
										DEFAULT_LINE_WEIGHT + lineWeight() * DEFAULT_LINE_WEIGHT_SCALING,
										m_planeGeometries.back(), m_trans);

		if (!success)
			return m_planeGeometries;

		// LINE RIGHT ================================================================

		IBKMK::Vector3D r (m_rightPoint - m_point2);
		if (m_style->m_fixedExtensionLength) {
			IBKMK::Vector3D ext = drawing.m_scalingFactor * m_style->m_fixedExtensionLength * r.normalized();
			point.m_x = m_rightPoint.m_x - ext.m_x;
			point.m_y = m_rightPoint.m_y - ext.m_y;
		}
		else {
			IBKMK::Vector3D ext = drawing.m_scalingFactor * m_style->m_extensionLineLowerDistance * r.normalized();
			point.m_x = m_point2.m_x + ext.m_x;
			point.m_y = m_point2.m_y + ext.m_y;
		}

		IBKMK::Vector3D p1Right = IBKMK::Vector3D(drawing.m_scalingFactor * point.m_x,
												  drawing.m_scalingFactor * point.m_y,
												  zCoordinate);
		lowerExtension = drawing.m_scalingFactor * m_style->m_upperLineDistance * l.normalized();
		IBKMK::Vector3D p2Right = IBKMK::Vector3D(drawing.m_scalingFactor * (m_rightPoint.m_x + lowerExtension.m_x),
												  drawing.m_scalingFactor * (m_rightPoint.m_y + lowerExtension.m_y),
												  zCoordinate);

		// rotate Vectors
		QVector3D vec1Right = drawing.m_rotationMatrix.toQuaternion() * IBKVector2QVector(p1Right);
		QVector3D vec2Right = drawing.m_rotationMatrix.toQuaternion() * IBKVector2QVector(p2Right);

		// move by origin
		vec1Right += IBKVector2QVector(drawing.m_origin);
		vec2Right += IBKVector2QVector(drawing.m_origin);

		m_planeGeometries.push_back(PlaneGeometry());
		success = generatePlaneFromLine(QVector2IBKVector(vec1Right), QVector2IBKVector(vec2Right), drawing.m_rotationMatrix,
										DEFAULT_LINE_WEIGHT + lineWeight() * DEFAULT_LINE_WEIGHT_SCALING,
										m_planeGeometries.back(), m_trans);

		if (!success)
			return m_planeGeometries;

		// Text ======================================================================

		double length = (m_leftPoint - m_rightPoint).magnitude();
		m_pickPoints.push_back(m_textPoint);

		generatePlanesFromText(QString("%1").arg(length).toStdString(), m_style->m_textHeight, Qt::AlignHCenter, m_angle, drawing.m_rotationMatrix, drawing.m_origin,
							   m_textPoint, drawing.m_scalingFactor, m_zPosition * Z_MULTIPLYER,
							   m_planeGeometries, m_trans);

		m_dirtyTriangulation = false;
		m_dirtyPoints = false;
	}

	return m_planeGeometries;
}


TiXmlElement * Drawing::Point::writeXML(TiXmlElement * parent) const {
	if (m_id == VICUS::INVALID_ID)  return nullptr;

	TiXmlElement * e = new TiXmlElement("Point");
	parent->LinkEndChild(e);

	if (m_id != VICUS::INVALID_ID)
		e->SetAttribute("id", IBK::val2string<unsigned int>(m_id));
	if (m_color.isValid())
		e->SetAttribute("color", m_color.name().toStdString());
	if (m_zPosition != 0.0)
		e->SetAttribute("zPosition", IBK::val2string<unsigned int>(m_zPosition));
	if (!m_layerName.isEmpty())
		e->SetAttribute("layer", m_layerName.toStdString());

	TiXmlElement::appendSingleAttributeElement(e, "Point", nullptr, std::string(), m_point.toString());

	return e;
}

void Drawing::Point::readXML(const TiXmlElement *element){
	FUNCID(Drawing::Circle::readXMLPrivate);

	try {
		// search for mandatory attributes
		if (!TiXmlAttribute::attributeByName(element, "id"))
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
									  IBK::FormatString("Missing required 'id' attribute.") ), FUNC_ID);


		const TiXmlAttribute * attrib = element->FirstAttribute();
		while (attrib) {
			const std::string & attribName = attrib->NameStr();
			if (attribName == "id")
				m_id = NANDRAD::readPODAttributeValue<unsigned int>(element, attrib);
			else if (attribName == "color")
				m_color = QColor(QString::fromStdString(attrib->ValueStr()));
			else if (attribName == "zPosition")
				m_zPosition = NANDRAD::readPODAttributeValue<unsigned int>(element, attrib);
			else if (attribName == "layer")
				m_layerName = QString::fromStdString(attrib->ValueStr());
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ATTRIBUTE).arg(attribName).arg(element->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			attrib = attrib->Next();
		}

		// reading elements
		const TiXmlElement * c = element->FirstChildElement();
		while (c) {
			const std::string & cName = c->ValueStr();
			if (cName == "Point") {
				try {
					m_point = IBKMK::Vector2D::fromString(c->GetText());
				} catch (IBK::Exception & ex) {
					throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(c->Row())
										  .arg("Invalid vector data."), FUNC_ID);
				}
			}
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			c = c->NextSiblingElement();
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'Drawing::Point' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'Drawing::Point' element.").arg(ex2.what()), FUNC_ID);
	}
}

const std::vector<IBKMK::Vector2D> &Drawing::Point::points2D() const {
	if (m_dirtyPoints) {
		m_pickPoints.clear();
		m_pickPoints.push_back(m_point);

		m_dirtyPoints = false;
	}

	return m_pickPoints;
}

const std::vector<PlaneGeometry> &Drawing::Point::planeGeometries(const Drawing &drawing) const {
	FUNCID(Drawing::Line::planeGeometries);

	if (m_dirtyTriangulation) {
		m_planeGeometries.clear();

		// Create Vector from point, add point of origin to each coordinate and calculate z value
		IBKMK::Vector3D p(drawing.m_scalingFactor * m_point.m_x,
						  drawing.m_scalingFactor * m_point.m_y,
						  m_zPosition * Z_MULTIPLYER);

		// scale Vector with selected unit

		double pointWeight = (DEFAULT_LINE_WEIGHT + lineWeight() * DEFAULT_LINE_WEIGHT_SCALING) / 2;

		// rotation
		QVector3D vec = drawing.m_rotationMatrix.toQuaternion() * IBKVector2QVector(p);
		vec += IBKVector2QVector(drawing.m_origin);

		IBKMK::Vector3D p1 = QVector2IBKVector(vec);

		IBKMK::Vector3D pExt0 = IBKMK::Vector3D(p1.m_x - pointWeight, p1.m_y - pointWeight, p1.m_z);
		IBKMK::Vector3D pExt1 = IBKMK::Vector3D(p1.m_x + pointWeight, p1.m_y - pointWeight, p1.m_z);
		IBKMK::Vector3D pExt2 = IBKMK::Vector3D(p1.m_x - pointWeight, p1.m_y + pointWeight, p1.m_z);

		IBKMK::Polygon3D po(VICUS::Polygon2D::T_Rectangle, pExt0, pExt2, pExt1);

		for (const IBKMK::Vector3D &v3D : po.vertexes())
			const_cast<IBKMK::Vector3D &>(v3D) = QVector2IBKVector(m_trans * IBKVector2QVector(v3D));


		m_planeGeometries.push_back(VICUS::PlaneGeometry(po));

		//		if (!success)
		//			throw IBK::Exception(IBK::FormatString("Could not generate plane geometry for Drawing Element #%1.").arg(m_id), FUNC_ID);

		m_dirtyTriangulation = false;
	}

	return m_planeGeometries;
}

void Drawing::Line::readXML(const TiXmlElement *element){
	FUNCID(Drawing::Circle::readXMLPrivate);

	try {
		// search for mandatory attributes
		if (!TiXmlAttribute::attributeByName(element, "id"))
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
									  IBK::FormatString("Missing required 'id' attribute.") ), FUNC_ID);


		const TiXmlAttribute * attrib = element->FirstAttribute();
		while (attrib) {
			const std::string & attribName = attrib->NameStr();
			if (attribName == "id")
				m_id = NANDRAD::readPODAttributeValue<unsigned int>(element, attrib);
			else if (attribName == "color")
				m_color = QColor(QString::fromStdString(attrib->ValueStr()));
			else if (attribName == "zPosition")
				m_zPosition = NANDRAD::readPODAttributeValue<unsigned int>(element, attrib);
			else if (attribName == "layer")
				m_layerName = QString::fromStdString(attrib->ValueStr());
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ATTRIBUTE).arg(attribName).arg(element->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			attrib = attrib->Next();
		}

		// reading elements
		const TiXmlElement * c = element->FirstChildElement();
		while (c) {
			const std::string & cName = c->ValueStr();
			if (cName == "Point1") {
				try {
					m_point1 = IBKMK::Vector2D::fromString(c->GetText());
				} catch (IBK::Exception & ex) {
					throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(c->Row())
										  .arg("Invalid vector data."), FUNC_ID);
				}
			}
			else if (cName == "Point2") {
				try {
					m_point2 = IBKMK::Vector2D::fromString(c->GetText());
				} catch (IBK::Exception & ex) {
					throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(c->Row())
										  .arg("Invalid vector data."), FUNC_ID);
				}
			}
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			c = c->NextSiblingElement();
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'Drawing::Line' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'Drawing::Line' element.").arg(ex2.what()), FUNC_ID);
	}
}

const std::vector<IBKMK::Vector2D> &Drawing::Line::points2D() const {
	if (m_dirtyPoints) {
		m_pickPoints.clear();

		m_pickPoints.push_back(m_point1);
		m_pickPoints.push_back(m_point2);

		m_dirtyPoints = false;
	}

	return m_pickPoints;
}

const std::vector<PlaneGeometry> &Drawing::Line::planeGeometries(const Drawing &drawing) const {
	FUNCID(Drawing::Line::planeGeometries);

	if (m_dirtyTriangulation) {
		m_planeGeometries.clear();

		VICUS::PlaneGeometry plane;
		// Create Vector from start and end point of the line, add point of origin to each coordinate and calculate z value
		double zCoordinate = m_zPosition * Z_MULTIPLYER;
		IBKMK::Vector3D p1 = IBKMK::Vector3D(drawing.m_scalingFactor * m_point1.m_x,
											 drawing.m_scalingFactor * m_point1.m_y,
											 zCoordinate);
		IBKMK::Vector3D p2 = IBKMK::Vector3D(drawing.m_scalingFactor * m_point2.m_x,
											 drawing.m_scalingFactor * m_point2.m_y,
											 zCoordinate);

		// rotate Vectors
		QVector3D vec1 = drawing.m_rotationMatrix.toQuaternion() * IBKVector2QVector(p1);
		QVector3D vec2 = drawing.m_rotationMatrix.toQuaternion() * IBKVector2QVector(p2);

		// scale Vector with selected unit
		vec1 += IBKVector2QVector(drawing.m_origin);
		vec2 += IBKVector2QVector(drawing.m_origin);

		bool success = generatePlaneFromLine(QVector2IBKVector(vec1), QVector2IBKVector(vec2), drawing.m_rotationMatrix,
											 DEFAULT_LINE_WEIGHT + lineWeight() * DEFAULT_LINE_WEIGHT_SCALING, plane, m_trans);

		if (!success)
			return m_planeGeometries;

		m_planeGeometries.push_back(plane);
		m_dirtyTriangulation = false;
	}

	return m_planeGeometries;
}

TiXmlElement * Drawing::Line::writeXML(TiXmlElement * parent) const {
	if (m_id == VICUS::INVALID_ID)  return nullptr;

	TiXmlElement * e = new TiXmlElement("Line");
	parent->LinkEndChild(e);

	if (m_id != VICUS::INVALID_ID)
		e->SetAttribute("id", IBK::val2string<unsigned int>(m_id));
	if (m_zPosition != 0.0)
		e->SetAttribute("zPosition", IBK::val2string<unsigned int>(m_zPosition));
	if (m_color.isValid())
		e->SetAttribute("color", m_color.name().toStdString());
	if (!m_layerName.isEmpty())
		e->SetAttribute("layer", m_layerName.toStdString());

	TiXmlElement::appendSingleAttributeElement(e, "Point1", nullptr, std::string(), m_point1.toString());
	TiXmlElement::appendSingleAttributeElement(e, "Point2", nullptr, std::string(), m_point2.toString());

	return e;
}


TiXmlElement * Drawing::Circle::writeXML(TiXmlElement * parent) const {
	if (m_id == VICUS::INVALID_ID)  return nullptr;

	TiXmlElement * e = new TiXmlElement("Circle");
	parent->LinkEndChild(e);

	if (m_id != VICUS::INVALID_ID)
		e->SetAttribute("id", IBK::val2string<unsigned int>(m_id));
	if (m_zPosition != 0.0)
		e->SetAttribute("zPosition", IBK::val2string<double>(m_id));
	if (m_color.isValid())
		e->SetAttribute("color", m_color.name().toStdString());
	if (!m_layerName.isEmpty())
		e->SetAttribute("layer", m_layerName.toStdString());

	TiXmlElement::appendSingleAttributeElement(e, "Center", nullptr, std::string(), m_center.toString());
	TiXmlElement::appendSingleAttributeElement(e, "Radius", nullptr, std::string(), IBK::val2string<double>(m_radius));

	return e;
}

void Drawing::Circle::readXML(const TiXmlElement *element){
	FUNCID(Drawing::Circle::readXMLPrivate);

	try {
		// search for mandatory attributes
		if (!TiXmlAttribute::attributeByName(element, "id"))
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
									  IBK::FormatString("Missing required 'id' attribute.") ), FUNC_ID);

		const TiXmlAttribute * attrib = element->FirstAttribute();
		while (attrib) {
			const std::string & attribName = attrib->NameStr();
			if (attribName == "id")
				m_id = NANDRAD::readPODAttributeValue<unsigned int>(element, attrib);
			else if (attribName == "color")
				m_color = QColor(QString::fromStdString(attrib->ValueStr()));
			else if (attribName == "zPosition")
				m_zPosition = NANDRAD::readPODAttributeValue<unsigned int>(element, attrib);
			else if (attribName == "layer")
				m_layerName = QString::fromStdString(attrib->ValueStr());
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ATTRIBUTE).arg(attribName).arg(element->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			attrib = attrib->Next();
		}

		// reading elements
		const TiXmlElement * c = element->FirstChildElement();
		while (c) {
			const std::string & cName = c->ValueStr();
			if (cName == "Radius")
				m_radius = NANDRAD::readPODElement<double>(c, cName);
			else if (cName == "Center") {
				try {
					m_center = IBKMK::Vector2D::fromString(c->GetText());
				} catch (IBK::Exception & ex) {
					throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(c->Row())
										  .arg("Invalid vector data."), FUNC_ID);
				}
			}
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			c = c->NextSiblingElement();
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'Drawing::Circle' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'Drawing::Circle' element.").arg(ex2.what()), FUNC_ID);
	}
}

const std::vector<IBKMK::Vector2D> &Drawing::Circle::points2D() const {
	if (m_dirtyPoints) {
		m_pickPoints.clear();
		m_pickPoints.resize(SEGMENT_COUNT_CIRCLE);

		for(unsigned int i = 0; i < SEGMENT_COUNT_CIRCLE; ++i){
			m_pickPoints[i] =IBKMK::Vector2D(m_center.m_x + m_radius * cos(2 * IBK::PI * i / SEGMENT_COUNT_CIRCLE),
											 m_center.m_y + m_radius * sin(2 * IBK::PI * i / SEGMENT_COUNT_CIRCLE));
		}

		m_dirtyPoints = false;
	}

	return m_pickPoints;
}


const std::vector<PlaneGeometry> &Drawing::Circle::planeGeometries(const Drawing &drawing) const {
	FUNCID(Drawing::Circle::planeGeometries);

	if (m_dirtyTriangulation) {
		m_planeGeometries.clear();

		std::vector<IBKMK::Vector3D> circlePoints;

		for(unsigned int i = 0; i < SEGMENT_COUNT_CIRCLE; i++){
			IBKMK::Vector3D p = IBKMK::Vector3D(drawing.m_scalingFactor * points2D()[i].m_x,
												drawing.m_scalingFactor * points2D()[i].m_y,
												m_zPosition * Z_MULTIPLYER);

			QVector3D vec = drawing.m_rotationMatrix.toQuaternion() * IBKVector2QVector(p);
			vec += IBKVector2QVector(drawing.m_origin);

			circlePoints.push_back(QVector2IBKVector(vec));
		}

		bool success = generatePlanesFromPolyline(circlePoints, drawing.m_rotationMatrix, true,
												  DEFAULT_LINE_WEIGHT + lineWeight() * DEFAULT_LINE_WEIGHT_SCALING,
												  m_planeGeometries, m_trans);

		if (!success)
			return m_planeGeometries;

		m_dirtyTriangulation = false;
	}

	return m_planeGeometries;
}


TiXmlElement * Drawing::PolyLine::writeXML(TiXmlElement * parent) const {
	if (m_id == VICUS::INVALID_ID)  return nullptr;

	TiXmlElement * e = new TiXmlElement("PolyLine");
	parent->LinkEndChild(e);

	if (!m_polyline.empty()) {

		if (m_id != VICUS::INVALID_ID)
			e->SetAttribute("id", IBK::val2string<unsigned int>(m_id));
		if (m_color.isValid())
			e->SetAttribute("color", m_color.name().toStdString());
		if (m_zPosition != 0.0)
			e->SetAttribute("zPosition", IBK::val2string<unsigned int>(m_zPosition));
		if (m_endConnected)
			e->SetAttribute("connected", IBK::val2string<bool>(m_endConnected));
		if (!m_layerName.isEmpty())
			e->SetAttribute("layer", m_layerName.toStdString());

		std::stringstream vals;
		const std::vector<IBKMK::Vector2D> & polyVertexes = m_polyline;
		for (unsigned int i=0; i<polyVertexes.size(); ++i) {
			vals << polyVertexes[i].m_x << " " << polyVertexes[i].m_y;
			if (i<polyVertexes.size()-1)  vals << ", ";
		}
		TiXmlText * text = new TiXmlText( vals.str() );
		e->LinkEndChild( text );
	}

	return e;
}

void Drawing::PolyLine::readXML(const TiXmlElement *element){
	FUNCID(Drawing::Arc::readXMLPrivate);

	try {
		// search for mandatory attributes
		if (!TiXmlAttribute::attributeByName(element, "id"))
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
									  IBK::FormatString("Missing required 'id' attribute.") ), FUNC_ID);

		const TiXmlAttribute * attrib = element->FirstAttribute();
		while (attrib) {
			const std::string & attribName = attrib->NameStr();
			if (attribName == "id")
				m_id = NANDRAD::readPODAttributeValue<unsigned int>(element, attrib);
			else if (attribName == "color")
				m_color = QColor(QString::fromStdString(attrib->ValueStr()));
			else if (attribName == "zPosition")
				m_zPosition = NANDRAD::readPODAttributeValue<unsigned int>(element, attrib);
			else if (attribName == "connected")
				m_endConnected = NANDRAD::readPODAttributeValue<bool>(element, attrib);
			else if (attribName == "layer")
				m_layerName = QString::fromStdString(attrib->ValueStr());
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ATTRIBUTE).arg(attribName).arg(element->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			attrib = attrib->Next();
		}

		// reading elements
		// read vertexes
		std::string text = element->GetText();
		text = IBK::replace_string(text, ",", " ");
		std::vector<IBKMK::Vector2D> verts;
		try {
			std::vector<double> vals;
			IBK::string2valueVector(text, vals);
			// must have n*2 elements
			if (vals.size() % 2 != 0)
				throw IBK::Exception("Mismatching number of values.", FUNC_ID);
			if (vals.empty())
				throw IBK::Exception("Missing values.", FUNC_ID);
			verts.resize(vals.size() / 2);
			for (unsigned int i=0; i<verts.size(); ++i){
				verts[i].m_x = vals[i*2];
				verts[i].m_y = vals[i*2+1];
			}

			m_polyline = verts;

		} catch (IBK::Exception & ex) {
			throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(element->Row())
								  .arg("Error reading element 'PolyLine'." ), FUNC_ID);
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'Drawing::PolyLine' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'Drawing::PolyLine' element.").arg(ex2.what()), FUNC_ID);
	}
}

const std::vector<IBKMK::Vector2D> &Drawing::PolyLine::points2D() const {
	if (m_dirtyPoints) {
		m_pickPoints.clear();

		// iterateover data.vertlist, insert all vertices of Polyline into vector
		for(size_t i = 0; i < m_polyline.size(); ++i){
			m_pickPoints.push_back(m_polyline[i]);
		}
		m_dirtyPoints = false;
	}
	return m_pickPoints;
}

const std::vector<PlaneGeometry> &Drawing::PolyLine::planeGeometries(const Drawing &drawing) const {
	FUNCID(Drawing::PolyLine::planeGeometries);

	if (m_dirtyTriangulation) {
		m_planeGeometries.clear();

		// Create Vector to store vertices of polyline
		std::vector<IBKMK::Vector3D> polylinePoints;

		// adds z-coordinate to polyline
		for(unsigned int i = 0; i < m_polyline.size(); ++i){
			IBKMK::Vector3D p = IBKMK::Vector3D(drawing.m_scalingFactor * m_polyline[i].m_x,
												drawing.m_scalingFactor * m_polyline[i].m_y,
												m_zPosition * Z_MULTIPLYER);
			QVector3D vec = drawing.m_rotationMatrix.toQuaternion() * IBKVector2QVector(p);
			vec += IBKVector2QVector(drawing.m_origin);

			polylinePoints.push_back(QVector2IBKVector(vec));
		}

		bool success = generatePlanesFromPolyline(polylinePoints, drawing.m_rotationMatrix, true,
												  DEFAULT_LINE_WEIGHT + lineWeight() * DEFAULT_LINE_WEIGHT_SCALING,
												  m_planeGeometries, m_trans);

		if (!success)
			return m_planeGeometries;

		m_dirtyTriangulation = false;
	}

	return m_planeGeometries;
}


TiXmlElement * Drawing::Arc::writeXML(TiXmlElement * parent) const {
	if (m_id == VICUS::INVALID_ID)  return nullptr;

	TiXmlElement * e = new TiXmlElement("Arc");
	parent->LinkEndChild(e);

	if (m_id != VICUS::INVALID_ID)
		e->SetAttribute("id", IBK::val2string<unsigned int>(m_id));
	if (m_color.isValid())
		e->SetAttribute("color", m_color.name().toStdString());
	if (m_zPosition != 0.0)
		e->SetAttribute("zPosition", IBK::val2string<unsigned int>(m_zPosition));
	if (!m_layerName.isEmpty())
		e->SetAttribute("layer", m_layerName.toStdString());

	TiXmlElement::appendSingleAttributeElement(e, "Center", nullptr, std::string(), m_center.toString());
	TiXmlElement::appendSingleAttributeElement(e, "Radius", nullptr, std::string(), IBK::val2string<double>(m_radius));
	TiXmlElement::appendSingleAttributeElement(e, "StartAngle", nullptr, std::string(), IBK::val2string<double>(m_startAngle));
	TiXmlElement::appendSingleAttributeElement(e, "EndAngle", nullptr, std::string(), IBK::val2string<double>(m_endAngle));

	return e;
}

void Drawing::Arc::readXML(const TiXmlElement *element){
	FUNCID(Drawing::Arc::readXMLPrivate);

	try {
		// search for mandatory attributes
		if (!TiXmlAttribute::attributeByName(element, "id"))
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
									  IBK::FormatString("Missing required 'id' attribute.") ), FUNC_ID);

		const TiXmlAttribute * attrib = element->FirstAttribute();
		while (attrib) {
			const std::string & attribName = attrib->NameStr();
			if (attribName == "id")
				m_id = NANDRAD::readPODAttributeValue<unsigned int>(element, attrib);
			else if (attribName == "color")
				m_color = QColor(QString::fromStdString(attrib->ValueStr()));
			else if (attribName == "zPosition")
				m_zPosition = NANDRAD::readPODAttributeValue<unsigned int>(element, attrib);
			else if (attribName == "layer")
				m_layerName = QString::fromStdString(attrib->ValueStr());
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ATTRIBUTE).arg(attribName).arg(element->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			attrib = attrib->Next();
		}

		// reading elements
		const TiXmlElement * c = element->FirstChildElement();
		while (c) {
			const std::string & cName = c->ValueStr();
			if (cName == "Radius")
				m_radius = NANDRAD::readPODElement<double>(c, cName);
			else if (cName == "StartAngle")
				m_startAngle = NANDRAD::readPODElement<double>(c, cName);
			else if (cName == "EndAngle")
				m_endAngle = NANDRAD::readPODElement<double>(c, cName);
			else if (cName == "Center") {
				try {
					m_center = IBKMK::Vector2D::fromString(c->GetText());
				} catch (IBK::Exception & ex) {
					throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(c->Row())
										  .arg("Invalid vector data."), FUNC_ID);
				}
			}
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			c = c->NextSiblingElement();
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'Drawing::Arc' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'Drawing::Arc' element.").arg(ex2.what()), FUNC_ID);
	}
}

const std::vector<IBKMK::Vector2D>& Drawing::Arc::points2D() const {

	if (m_dirtyPoints) {
		double startAngle = m_startAngle;
		double endAngle = m_endAngle;

		double angleDifference;

		if (startAngle > endAngle)
			angleDifference = 2 * IBK::PI - startAngle + endAngle;
		else
			angleDifference = endAngle - startAngle;

		unsigned int n = std::ceil(angleDifference / (2 * IBK::PI) * SEGMENT_COUNT_ARC);
		double stepAngle = angleDifference / n;

		m_pickPoints.resize(n + 1);
		for (unsigned int i = 0; i < n+1; ++i){
			m_pickPoints[i] = IBKMK::Vector2D(m_center.m_x + m_radius * cos(startAngle + i * stepAngle),
											  m_center.m_y + m_radius * sin(startAngle + i * stepAngle));
		}

		m_dirtyPoints = false;
	}
	return m_pickPoints;
}

const std::vector<PlaneGeometry> &Drawing::Arc::planeGeometries(const Drawing &drawing) const {
	FUNCID(Drawing::Arc::planeGeometries);

	if (m_dirtyTriangulation) {
		m_planeGeometries.clear();

		std::vector<IBKMK::Vector3D> arcPoints;
		const std::vector<IBKMK::Vector2D> &arcPoints2D = points2D();
		for (unsigned int i = 0; i < arcPoints2D.size(); ++i){
			IBKMK::Vector3D p = IBKMK::Vector3D(drawing.m_scalingFactor * arcPoints2D[i].m_x,
												drawing.m_scalingFactor * arcPoints2D[i].m_y,
												m_zPosition * Z_MULTIPLYER);
			QVector3D vec = drawing.m_rotationMatrix.toQuaternion() * IBKVector2QVector(p);
			vec += IBKVector2QVector(drawing.m_origin);

			arcPoints.push_back(QVector2IBKVector(vec));
		}

		bool success = generatePlanesFromPolyline(arcPoints, drawing.m_rotationMatrix, false,
												  DEFAULT_LINE_WEIGHT + lineWeight() * DEFAULT_LINE_WEIGHT_SCALING,
												  m_planeGeometries, m_trans);

		if (!success)
			return m_planeGeometries;

		m_dirtyTriangulation = false;
	}

	return m_planeGeometries;
}


TiXmlElement * Drawing::Ellipse::writeXML(TiXmlElement * parent) const {
	if (m_id == VICUS::INVALID_ID)  return nullptr;

	TiXmlElement * e = new TiXmlElement("Ellipse");
	parent->LinkEndChild(e);

	if (m_id != VICUS::INVALID_ID)
		e->SetAttribute("id", IBK::val2string<unsigned int>(m_id));
	if (m_color.isValid())
		e->SetAttribute("color", m_color.name().toStdString());
	if (m_zPosition != 0.0)
		e->SetAttribute("zPosition", IBK::val2string<unsigned int>(m_zPosition));
	if (!m_layerName.isEmpty())
		e->SetAttribute("layer", m_layerName.toStdString());

	TiXmlElement::appendSingleAttributeElement(e, "Center", nullptr, std::string(), m_center.toString());
	TiXmlElement::appendSingleAttributeElement(e, "MajorAxis", nullptr, std::string(), m_majorAxis.toString());
	TiXmlElement::appendSingleAttributeElement(e, "Ratio", nullptr, std::string(), IBK::val2string<double>(m_ratio));
	TiXmlElement::appendSingleAttributeElement(e, "StartAngle", nullptr, std::string(), IBK::val2string<double>(m_startAngle));
	TiXmlElement::appendSingleAttributeElement(e, "EndAngle", nullptr, std::string(), IBK::val2string<double>(m_endAngle));

	return e;
}


void Drawing::Ellipse::readXML(const TiXmlElement *element){
	FUNCID(Drawing::Arc::readXMLPrivate);

	try {
		// search for mandatory attributes
		if (!TiXmlAttribute::attributeByName(element, "id"))
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
									  IBK::FormatString("Missing required 'id' attribute.") ), FUNC_ID);


		const TiXmlAttribute * attrib = element->FirstAttribute();
		while (attrib) {
			const std::string & attribName = attrib->NameStr();
			if (attribName == "id")
				m_id = NANDRAD::readPODAttributeValue<unsigned int>(element, attrib);
			else if (attribName == "color")
				m_color = QColor(QString::fromStdString(attrib->ValueStr()));
			else if (attribName == "zPosition")
				m_zPosition = NANDRAD::readPODAttributeValue<unsigned int>(element, attrib);
			else if (attribName == "layer")
				m_layerName = QString::fromStdString(attrib->ValueStr());
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ATTRIBUTE).arg(attribName).arg(element->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			attrib = attrib->Next();
		}

		// reading elements
		const TiXmlElement * c = element->FirstChildElement();
		while (c) {
			const std::string & cName = c->ValueStr();
			if (cName == "Ratio")
				m_ratio = NANDRAD::readPODElement<double>(c, cName);
			else if (cName == "StartAngle")
				m_startAngle = NANDRAD::readPODElement<double>(c, cName);
			else if (cName == "EndAngle")
				m_endAngle = NANDRAD::readPODElement<double>(c, cName);
			else if (cName == "Center") {
				try {
					m_center = IBKMK::Vector2D::fromString(c->GetText());
				} catch (IBK::Exception & ex) {
					throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(c->Row())
										  .arg("Invalid vector data."), FUNC_ID);
				}
			}
			else if (cName == "MajorAxis") {
				try {
					m_majorAxis = IBKMK::Vector2D::fromString(c->GetText());
				} catch (IBK::Exception & ex) {
					throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(c->Row())
										  .arg("Invalid vector data."), FUNC_ID);
				}
			}

			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			c = c->NextSiblingElement();
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'Drawing::Arc' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'Drawing::Arc' element.").arg(ex2.what()), FUNC_ID);
	}
}

const std::vector<IBKMK::Vector2D> &Drawing::Ellipse::points2D() const {
	if (m_dirtyPoints) {
		m_pickPoints.clear();

		// iterateover data.vertlist, insert all vertices of Polyline into vector
		double startAngle = m_startAngle;
		double endAngle = m_endAngle;

		double angleStep = (endAngle - startAngle) / (SEGMENT_COUNT_ELLIPSE - 1);

		double majorRadius = sqrt(pow(m_majorAxis.m_x, 2) + pow(m_majorAxis.m_y, 2));
		double minorRadius = majorRadius * m_ratio;
		double rotationAngle = atan2(m_majorAxis.m_y, m_majorAxis.m_x);

		double x, y, rotated_x, rotated_y;

		m_pickPoints.resize(SEGMENT_COUNT_ELLIPSE);

		for (unsigned int i = 0; i < SEGMENT_COUNT_ELLIPSE; ++i) {

			double currentAngle = startAngle + i * angleStep;

			x = majorRadius * cos(currentAngle);
			y = minorRadius * sin(currentAngle);

			rotated_x = x * cos(rotationAngle) - y * sin(rotationAngle);
			rotated_y = x * sin(rotationAngle) + y * cos(rotationAngle);

			m_pickPoints[i] = IBKMK::Vector2D(rotated_x + m_center.m_x,
											  rotated_y + m_center.m_y);

		}
		m_dirtyPoints = false;
	}
	return m_pickPoints;
}


const std::vector<PlaneGeometry> &Drawing::Ellipse::planeGeometries(const Drawing &drawing) const {
	FUNCID(Drawing::Ellipse::planeGeometries);

	if (m_dirtyTriangulation) {
		m_planeGeometries.clear();

		const std::vector<IBKMK::Vector2D> &pickPoints = points2D();

		std::vector<IBKMK::Vector3D> ellipsePoints;
		for (unsigned int i = 0; i < SEGMENT_COUNT_ELLIPSE; i++) {

			IBKMK::Vector3D p = IBKMK::Vector3D(drawing.m_scalingFactor * pickPoints[i].m_x,
												drawing.m_scalingFactor * pickPoints[i].m_y,
												m_zPosition * Z_MULTIPLYER);

			QVector3D vec = drawing.m_rotationMatrix.toQuaternion() * IBKVector2QVector(p);
			vec += IBKVector2QVector(drawing.m_origin);

			ellipsePoints.push_back(QVector2IBKVector(vec));
		}

		Q_ASSERT(points2D().size() > 0);

		bool connect = pickPoints[0] == pickPoints.back();
		bool success = generatePlanesFromPolyline(ellipsePoints, drawing.m_rotationMatrix, connect,
												  DEFAULT_LINE_WEIGHT + lineWeight() * DEFAULT_LINE_WEIGHT_SCALING,
												  m_planeGeometries, m_trans);

		if (!success)
			return m_planeGeometries;

		m_dirtyTriangulation = false;
	}

	return m_planeGeometries;
}


const Drawing::AbstractDrawingObject *Drawing::objectByID(unsigned int id) const {
	FUNCID(Drawing::objectByID);

	AbstractDrawingObject *obj = m_objectPtr.at(id);
	if (obj == nullptr)
		throw IBK::Exception(IBK::FormatString("Drawing Object with ID #%1 not found").arg(id), FUNC_ID);

	return obj;
}


void Drawing::updatePointer(){
	m_objectPtr.clear();

	for (unsigned int i=0; i < m_points.size(); ++i){
		m_points[i].m_parentLayer = findLayerPointer(m_points[i].m_layerName);
		m_points[i].m_block = findBlockPointer(m_points[i].m_blockName);
		m_objectPtr[m_points[i].m_id] = &m_points[i];
	}
	for (unsigned int i=0; i < m_lines.size(); ++i){
		m_lines[i].m_parentLayer = findLayerPointer(m_lines[i].m_layerName);
		m_lines[i].m_block = findBlockPointer(m_lines[i].m_blockName);
		m_objectPtr[m_lines[i].m_id] = &m_lines[i];
	}
	for (unsigned int i=0; i < m_polylines.size(); ++i){
		m_polylines[i].m_parentLayer = findLayerPointer(m_polylines[i].m_layerName);
		m_polylines[i].m_block = findBlockPointer(m_polylines[i].m_blockName);
		m_objectPtr[m_polylines[i].m_id] = &m_polylines[i];
	}
	for (unsigned int i=0; i < m_circles.size(); ++i){
		m_circles[i].m_parentLayer = findLayerPointer(m_circles[i].m_layerName);
		m_circles[i].m_block = findBlockPointer(m_circles[i].m_blockName);
		m_objectPtr[m_circles[i].m_id] = &m_circles[i];
	}
	for (unsigned int i=0; i < m_arcs.size(); ++i){
		m_arcs[i].m_parentLayer = findLayerPointer(m_arcs[i].m_layerName);
		m_arcs[i].m_block = findBlockPointer(m_arcs[i].m_blockName);
		m_objectPtr[m_arcs[i].m_id] = &m_arcs[i];
	}
	for (unsigned int i=0; i < m_ellipses.size(); ++i){
		m_ellipses[i].m_parentLayer = findLayerPointer(m_ellipses[i].m_layerName);
		m_ellipses[i].m_block = findBlockPointer(m_ellipses[i].m_blockName);
		m_objectPtr[m_ellipses[i].m_id] = &m_ellipses[i];
	}
	for (unsigned int i=0; i < m_solids.size(); ++i){
		m_solids[i].m_parentLayer = findLayerPointer(m_solids[i].m_layerName);
		m_solids[i].m_block = findBlockPointer(m_solids[i].m_blockName);
		m_objectPtr[m_solids[i].m_id] = &m_solids[i];
	}
	for (unsigned int i=0; i < m_texts.size(); ++i){
		m_texts[i].m_parentLayer = findLayerPointer(m_texts[i].m_layerName);
		m_texts[i].m_block = findBlockPointer(m_texts[i].m_blockName);
		m_objectPtr[m_texts[i].m_id] = &m_texts[i];
	}
	for (unsigned int i=0; i < m_inserts.size(); ++i){
		for(unsigned int j = 0; j < m_blocks.size(); ++j) {
			const QString &blockName = m_blocks[j].m_name;
			const QString &insertBlockName = m_inserts[i].m_currentBlockName;
			if (blockName == insertBlockName) {
				m_inserts[i].m_currentBlock = &m_blocks[j];
			}

			const QString &insertParentBlockName = m_inserts[i].m_parentBlockName;
			if (blockName == insertParentBlockName) {
				m_inserts[i].m_parentBlock = &m_blocks[j];
			}
		}
	}
	for (unsigned int i=0; i < m_linearDimensions.size(); ++i){
		m_linearDimensions[i].m_parentLayer = findLayerPointer(m_linearDimensions[i].m_layerName);
		m_texts[i].m_block = findBlockPointer(m_linearDimensions[i].m_blockName);
		m_objectPtr[m_linearDimensions[i].m_id] = &m_linearDimensions[i];

		for(unsigned int j = 0; j < m_dimensionStyles.size(); ++j) {
			const QString &dimStyleName = m_dimensionStyles[j].m_name;
			const QString &styleName = m_linearDimensions[i].m_styleName;
			if (dimStyleName == styleName) {
				m_linearDimensions[i].m_style = &m_dimensionStyles[j];
				break;
			}
		}

		// In order to be safe
		if (m_linearDimensions[i].m_style == nullptr)
			m_linearDimensions[i].m_style = &m_dimensionStyles.front();
	}

	// Update blocks
//	for (unsigned int i=0; i < m_drawingLayers.size(); ++i) {
//		if (m_drawingLayers[i].m_idBlock != INVALID_ID ) {
//			for (unsigned int j=0; j < m_blocks.size(); ++j) {
//				if (m_blocks[j].m_id == m_drawingLayers[i].m_idBlock) {
//					m_drawingLayers[i].m_currentBlock = &m_blocks[j];
//					break;
//				}
//			}
//		}
//	}
}

template <typename t>
void updateGeometry(std::vector<t> &objects) {
	for (t &obj : objects )
		obj.updateGeometry();
}

void Drawing::updatePlaneGeometries() {
	updateGeometry<Drawing::Line>(m_lines);
	updateGeometry<Drawing::PolyLine>(m_polylines);
	updateGeometry<Drawing::Arc>(m_arcs);
	updateGeometry<Drawing::Circle>(m_circles);
	updateGeometry<Drawing::Ellipse>(m_ellipses);
	updateGeometry<Drawing::Solid>(m_solids);
	updateGeometry<Drawing::LinearDimension>(m_linearDimensions);
	updateGeometry<Drawing::Point>(m_points);
	updateGeometry<Drawing::Text>(m_texts);

	m_dirtyPickPoints = true;
}

template <typename t>
void generateObjectFromInsert(unsigned int &nextId, const Drawing::Block &block,
							  std::vector<t> &objects, const QMatrix4x4 &trans) {
	std::vector<t> newObjects;

	for (const t &obj : objects) {

		if (obj.m_block == nullptr)
			continue;

		if (obj.m_block->m_name != block.m_name)
			continue;

		t newObj(obj);
		newObj.m_id = ++nextId;
		newObj.m_trans = trans;
		newObj.m_blockName = "";
		newObj.m_block = nullptr;

		newObjects.push_back(newObj);
	}

	objects.insert(objects.end(), newObjects.begin(), newObjects.end());
}

void Drawing::transformInsert(QMatrix4x4 &trans, const VICUS::Drawing::Insert &insert, unsigned int &nextId) {

	Q_ASSERT(insert.m_currentBlock != nullptr);
	IBKMK::Vector2D insertPoint = insert.m_insertionPoint - insert.m_currentBlock->m_basePoint;

	trans.translate(QVector3D(insertPoint.m_x,
							  insertPoint.m_y,
							  0.0));

	for (const Insert &i : m_inserts) {
		if (i.m_parentBlock == nullptr)
			continue;

		if (insert.m_currentBlockName == i.m_parentBlock->m_name) {
			transformInsert(trans, i, nextId);
		}
	}

	generateObjectFromInsert(nextId, *insert.m_currentBlock, m_points, trans);
	generateObjectFromInsert(nextId, *insert.m_currentBlock, m_arcs, trans);
	generateObjectFromInsert(nextId, *insert.m_currentBlock, m_circles, trans);
	generateObjectFromInsert(nextId, *insert.m_currentBlock, m_ellipses, trans);
	generateObjectFromInsert(nextId, *insert.m_currentBlock, m_lines, trans);
	generateObjectFromInsert(nextId, *insert.m_currentBlock, m_polylines, trans);
	generateObjectFromInsert(nextId, *insert.m_currentBlock, m_solids, trans);
	generateObjectFromInsert(nextId, *insert.m_currentBlock, m_texts, trans);

}

void Drawing::generateInsertGeometries(unsigned int &nextId) {
	FUNCID(Drawing::generateInsertGeometries);
	updateParents();

	for (const VICUS::Drawing::Insert &insert : m_inserts) {

		if (insert.m_parentBlock != nullptr)
			continue;

		if (insert.m_currentBlock == nullptr)
			throw IBK::Exception(IBK::FormatString("Block with name '%1' was not found").arg(insert.m_currentBlockName.toStdString()), FUNC_ID);

		QMatrix4x4 trans;
		transformInsert(trans, insert, nextId);
	}

	updateParents();
}


template <typename t>
void addPickPoints(const std::vector<t> &objects, const VICUS::Drawing &d, std::map<unsigned int, std::vector<IBKMK::Vector3D>> &verts,
				   const Drawing::Block *block = nullptr) {
	for (const t& obj : objects) {
		bool isBlockDefined = block != nullptr;
		bool hasBlock = obj.m_block != nullptr;

		if ((hasBlock && !isBlockDefined) || (isBlockDefined && !hasBlock))
			continue;

		if (isBlockDefined && hasBlock && block->m_name != obj.m_block->m_name)
			continue;

		const std::vector<IBKMK::Vector3D> &objVerts = d.points3D(obj);
		verts[obj.m_id] = objVerts;
	}
}


const std::map<unsigned int, std::vector<IBKMK::Vector3D>> &Drawing::pickPoints() const {
	FUNCID(Drawing::pickPoints);
	try {
		if (m_dirtyPickPoints) {
			addPickPoints(m_points, *this, m_pickPoints);
			addPickPoints(m_arcs, *this, m_pickPoints);
			addPickPoints(m_circles, *this, m_pickPoints);
			addPickPoints(m_ellipses, *this, m_pickPoints);
			addPickPoints(m_linearDimensions, *this, m_pickPoints);
			addPickPoints(m_lines, *this, m_pickPoints);
			addPickPoints(m_polylines, *this, m_pickPoints);
			addPickPoints(m_solids, *this, m_pickPoints);

			m_dirtyPickPoints = false;
		}
		return m_pickPoints;
	}
	catch (IBK::Exception &ex) {
		throw IBK::Exception(IBK::FormatString("Could not generate pick points.\n%1").arg(ex.what()), FUNC_ID);
	}
}


const std::vector<IBKMK::Vector3D> Drawing::points3D(const AbstractDrawingObject &obj) const {

	const std::vector<IBKMK::Vector2D> &points2D = obj.points2D();
	std::vector<IBKMK::Vector3D> points3D(points2D.size());

	for (unsigned int i=0; i<points2D.size(); ++i) {
		const IBKMK::Vector2D &v2D = points2D[i];
		double zCoordinate = obj.m_zPosition * Z_MULTIPLYER;
		IBKMK::Vector3D v3D = IBKMK::Vector3D(v2D.m_x * m_scalingFactor,
											  v2D.m_y * m_scalingFactor,
											  zCoordinate);

		QVector3D qV3D = m_rotationMatrix.toQuaternion() * IBKVector2QVector(v3D);
		qV3D += IBKVector2QVector(m_origin);

		// Apply block transformation from insert
		qV3D = obj.m_trans * qV3D;
		points3D[i] = QVector2IBKVector(qV3D);
	}

	return points3D;
}

const IBKMK::Vector3D Drawing::normal() const {
	return QVector2IBKVector(m_rotationMatrix.toQuaternion() * QVector3D(0,0,1));
}

const IBKMK::Vector3D Drawing::localX() const {
	return QVector2IBKVector(m_rotationMatrix.toQuaternion() * QVector3D(1,0,0));
}

const IBKMK::Vector3D Drawing::localY() const {
	return QVector2IBKVector(m_rotationMatrix.toQuaternion() * QVector3D(0,1,0));
}


DrawingLayer* Drawing::findLayerPointer(const QString &layername){
	for(unsigned int i = 0; i < m_drawingLayers.size(); ++i) {
		if (m_drawingLayers[i].m_displayName == layername)
			return &m_drawingLayers[i];
	}
	return nullptr;
}

Drawing::Block *Drawing::findBlockPointer(const QString &name){
	for(unsigned int i = 0; i < m_blocks.size(); ++i) {
		if (m_blocks[i].m_name == name)
			return &m_blocks[i];
	}
	return nullptr;
}


bool Drawing::generatePlaneFromLine(const IBKMK::Vector3D &startPoint, const IBKMK::Vector3D &endPoint,
									const RotationMatrix &matrix, double width, VICUS::PlaneGeometry &plane,
									const QMatrix4x4 &trans) {

	// Calculate the line vector and its length
	IBKMK::Vector3D lineVector = endPoint - startPoint;
	double length = lineVector.magnitude();
	if(length <= 0)
		return false;

	// Calculate the line width (1 pixel)
	double halfWidth = width / 2.0;

	// Calculate a perpendicular vector for the line width
	IBKMK::Vector3D normal(matrix.toQuaternion().toRotationMatrix()(0,2),
						   matrix.toQuaternion().toRotationMatrix()(1,2),
						   matrix.toQuaternion().toRotationMatrix()(2,2));

	// calculate perpendicular vector
	IBKMK::Vector3D perpendicularVector(lineVector.crossProduct(normal));
	perpendicularVector.normalize();
	perpendicularVector *= halfWidth;

	// Create an array of 4 vertices to define the box
	std::vector<IBKMK::Vector3D> lineVertices = {
		startPoint - perpendicularVector,
		endPoint - perpendicularVector,
		endPoint + perpendicularVector,
		startPoint + perpendicularVector,
	};


	for (IBKMK::Vector3D &v3D : lineVertices)
		v3D = QVector2IBKVector(trans * IBKVector2QVector(v3D));


	// Call addPlane to create the box geometry twice so visible from both sides
	IBKMK::Polygon3D p(VICUS::Polygon2D::T_Rectangle, lineVertices[0], lineVertices[3], lineVertices[1]);
	plane = VICUS::PlaneGeometry(p);

	return true;
}


bool Drawing::generatePlanesFromPolyline(const std::vector<IBKMK::Vector3D> &polyline, const RotationMatrix &matrix,
										 bool connectEndStart, double width, std::vector<PlaneGeometry> &planes,
										 const QMatrix4x4 &trans) {

	// initialise values
	IBKMK::Vector3D lineVector, previousVector, crossProduct, perpendicularVector;
	std::vector<IBKMK::Vector3D> previousVertices;
	double halfWidth = width / 2;
	double length;

	// if polyline is empty, return
	if(polyline.size() < 2){
		return false;
	}

	for (const IBKMK::Vector3D &v3D : polyline)
		const_cast<IBKMK::Vector3D &>(v3D) = QVector2IBKVector(trans * IBKVector2QVector(v3D));

	// initialise previousVector
	previousVector = polyline[1] - polyline[0];

	auto processSegment = [&](const IBKMK::Vector3D& startPoint, const IBKMK::Vector3D& endPoint)->void {
		// calculate line vector
		lineVector = endPoint - startPoint;
		length = lineVector.magnitude();
		if(length <= 0)
			return;

		IBKMK::Vector3D normal(matrix.toQuaternion().toRotationMatrix()(0,2),
							   matrix.toQuaternion().toRotationMatrix()(1,2),
							   matrix.toQuaternion().toRotationMatrix()(2,2));

		// calculate perpendicular vector
		perpendicularVector = lineVector.crossProduct(normal);
		perpendicularVector.normalize();
		perpendicularVector *= halfWidth;

		// create vertices for the line
		std::vector<IBKMK::Vector3D> lineVertices = {
			startPoint - perpendicularVector,
			endPoint - perpendicularVector,
			endPoint + perpendicularVector,
			startPoint + perpendicularVector,
		};

		// Transformation for block segment
		// Draw the line
		IBKMK::Polygon3D p(VICUS::Polygon2D::T_Rectangle, lineVertices[0], lineVertices[3], lineVertices[1]);
		planes.push_back(VICUS::PlaneGeometry(p));

		// Calculate the cross product between the current line Vector and previous to get the direction of the triangle
		crossProduct = lineVector.crossProduct(previousVector);

		if (previousVertices.size() == lineVertices.size()) {
			// draws the triangle
			if(crossProduct.m_z < -1e-10){
				// line is left
				std::vector<IBKMK::Vector3D> verts(3);
				verts[0] = previousVertices[1];
				verts[1] = startPoint;
				verts[2] = lineVertices[0];

				IBKMK::Polygon3D poly3d(verts);
				planes.push_back(PlaneGeometry(poly3d));
			}
			else if(crossProduct.m_z > 1e-10){
				// line is right

				// line is left
				std::vector<IBKMK::Vector3D> verts(3);
				verts[0] = lineVertices[3];
				verts[1] = previousVertices[2];
				verts[2] = startPoint;

				IBKMK::Polygon3D poly3d(verts);
				planes.push_back(PlaneGeometry(poly3d));
			}
			else {
				// if z coordinate of cross product is 0, lines are parallel, no triangle needed (would crash anyway)
				previousVector = lineVector;
				previousVertices = lineVertices;
				return;
			}
		}

		// update previous values
		previousVector = lineVector;
		previousVertices = lineVertices;
	};

	// loops through all points in polyline, draws a line between every two points, adds a triangle between two lines to fill out the gaps
	for(unsigned int i = 0; i < polyline.size() - 1; i++){
		processSegment(polyline[i], polyline[i+1]);
	}

	// repeats the code of the for loop for the last line and adds two triangles to fill out the lines
	if(connectEndStart){
		unsigned int lastIndex = polyline.size() - 1;
		processSegment(polyline[lastIndex], polyline[0]);
		processSegment(polyline[0], polyline[1]);
	}

	return true;
}


bool isClockwise(const QPolygonF& polygon) {
	double sum = 0.0;
	for (int i = 0; i < polygon.count(); i++) {
		QPointF p1 = polygon[i];
		QPointF p2 = polygon[(i + 1) % polygon.count()]; // next point
		sum += (p2.x() - p1.x()) * (p2.y() + p1.y());
	}
	return sum > 0.0;
}


void Drawing::generatePlanesFromText(const std::string &text, double textSize, Qt::Alignment alignment, const double &rotationAngle,
									 const VICUS::RotationMatrix &matrix, const IBKMK::Vector3D &origin,
									 const IBKMK::Vector2D &basePoint, double scalingFactor, double zScale, std::vector<PlaneGeometry> &planeGeometries,
									 const QMatrix4x4 &trans) {

	if (text.empty())
		return;

	// We choose Arial for now
	QFont font("Arial");
	// Create a QPainterPath object
	QPainterPath path;
	path.addText(0, 0, font, QString::fromStdString(text)); // 50 is roughly the baseline for the text

	double width = path.boundingRect().width();
	double moveX = 0.0;
	if (alignment == Qt::AlignHCenter) {
		moveX = -0.5*width;
	}

	QTransform transformation;
	transformation.rotate(rotationAngle);  // Rotate by 45 degrees
	transformation.translate(moveX, 0.0);

	// Apply the rotation to the path
	QPainterPath rotatedPath = transformation.map(path);

	// Extract polygons from the path
	QList<QPolygonF> polygons = rotatedPath.toSubpathPolygons();

	for (int i=0; i < polygons.size(); ++i) {

		const QPolygonF &polygon = polygons[i];
		std::vector<IBKMK::Vector3D> poly(polygon.size());

		for (unsigned int i=0; i<poly.size(); ++i) {
			const QPointF &point = polygon[i];
			// double zCoordinate = obj->m_zPosition * Z_MULTIPLYER + d->m_origin.m_z;
			IBKMK::Vector3D v3D = IBKMK::Vector3D(  point.x() * textSize * DEFAULT_FONT_SCALING * 2 + basePoint.m_x,
												   -point.y() * textSize * DEFAULT_FONT_SCALING * 2 + basePoint.m_y,
												   zScale);

			v3D *= scalingFactor;
			QVector3D qV3D = matrix.toQuaternion() * IBKVector2QVector(v3D);
			qV3D += IBKVector2QVector(origin);
			qV3D = trans * qV3D;

			poly[i] = QVector2IBKVector(qV3D);
		}

		IBKMK::Polygon3D poly3D(poly);
		if (!poly3D.isValid())
			continue;

		if ( planeGeometries.size() > 0 && isClockwise(polygon) ) {
			/// We need to use the hole triangulation of the plane geometry
			/// in order to add holes to the letters. We now just assume, that
			/// if the polygon is clockwise we have a hole and a parent plane geometry
			/// we convert the coordinates back to plane coordinates
			std::vector<IBKMK::Vector3D> verts = poly3D.vertexes();
			std::vector<IBKMK::Vector2D> verts2D(poly3D.vertexes().size());

			const IBKMK::Vector3D &offset = planeGeometries.back().offset();
			const IBKMK::Vector3D &localX = planeGeometries.back().localX();
			const IBKMK::Vector3D &localY = planeGeometries.back().localY();

			for (unsigned int i=0; i<verts.size(); ++i) {
				const IBKMK::Vector3D v3D = verts[i];
				IBKMK::planeCoordinates(offset, localX, localY, v3D, verts2D[i].m_x, verts2D[i].m_y);
			}

			const std::vector<VICUS::PlaneGeometry::Hole> &holes = planeGeometries.back().holes();
			const_cast<std::vector<VICUS::PlaneGeometry::Hole> &>(holes).push_back(
						VICUS::PlaneGeometry::Hole(VICUS::INVALID_ID, verts2D, false));
			planeGeometries.back().setHoles(holes);
		}
		else
			planeGeometries.push_back(VICUS::PlaneGeometry(poly3D));
	}
}

const QColor & Drawing::AbstractDrawingObject::color() const{
	/* If the object has a color, return it, else use color of parent */
	if (m_color.isValid())
		return m_color;
	else if (m_parentLayer != nullptr) {
		const DrawingLayer *layer = m_parentLayer;
		Q_ASSERT(layer != nullptr);
		return layer->m_color;
	}

	return m_color;
}


double Drawing::AbstractDrawingObject::lineWeight() const{
	/* if -1: use weight of layer */
	const DrawingLayer *dl = m_parentLayer;

	// TODO: how to handle case where there is no parent layer
	if (m_lineWeight < 0) {
		if(dl == nullptr || dl->m_lineWeight < 0){
			return 0;
		}
		else{
			return dl->m_lineWeight;
		}
	}
	/* if -3: default lineWeight is used
		if -2: lineWeight of block is used. Needs to be modified when blocks
		are implemented */
	else if(m_lineWeight == -3 || m_lineWeight == -2)
		return 0;
	else
		return m_lineWeight;
}

TiXmlElement *Drawing::DimStyle::writeXML(TiXmlElement *parent) const {
	if (m_id == VICUS::INVALID_ID)  return nullptr;

	TiXmlElement * e = new TiXmlElement("DimStyle");
	parent->LinkEndChild(e);

	if (m_id != VICUS::INVALID_ID)
		e->SetAttribute("id", IBK::val2string<unsigned int>(m_id));
	if (m_name != QString())
		e->SetAttribute("name", m_name.toStdString());
	if (m_upperLineDistance > 0.0)
		e->SetAttribute("upperLineDistance", IBK::val2string<double>(m_upperLineDistance));
	if (m_extensionLineLowerDistance > 0.0)
		e->SetAttribute("extensionLineLowerDistance", IBK::val2string<double>(m_extensionLineLowerDistance));
	if (m_extensionLineLength > 0.0)
		e->SetAttribute("extensionLineLength", IBK::val2string<double>(m_extensionLineLength));
	if (!m_fixedExtensionLength)
		e->SetAttribute("fixedExtensionLength", IBK::val2string<bool>(m_fixedExtensionLength));
	if (m_textHeight > 0.0)
		e->SetAttribute("textHeight", IBK::val2string<double>(m_textHeight));

	return e;
}

void Drawing::DimStyle::readXML(const TiXmlElement *element) {
	FUNCID(Drawing::DimStyle::readXMLPrivate);

	try {
		// search for mandatory attributes
		if (!TiXmlAttribute::attributeByName(element, "id"))
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
									  IBK::FormatString("Missing required 'id' attribute.") ), FUNC_ID);


		const TiXmlAttribute * attrib = element->FirstAttribute();
		while (attrib) {
			const std::string & attribName = attrib->NameStr();
			if (attribName == "id")
				m_id = NANDRAD::readPODAttributeValue<unsigned int>(element, attrib);
			else if (attribName == "name")
				m_name = QString::fromStdString(attrib->ValueStr());
			else if (attribName == "upperLineDistance")
				m_upperLineDistance = NANDRAD::readPODAttributeValue<double>(element, attrib);
			else if (attribName == "extensionLineLowerDistance")
				m_extensionLineLowerDistance = NANDRAD::readPODAttributeValue<double>(element, attrib);
			else if (attribName == "extensionLineLength")
				m_extensionLineLength = NANDRAD::readPODAttributeValue<double>(element, attrib);
			else if (attribName == "fixedExtensionLength")
				m_fixedExtensionLength = NANDRAD::readPODAttributeValue<bool>(element, attrib);
			else if (attribName == "textHeight")
				m_textHeight = NANDRAD::readPODAttributeValue<double>(element, attrib);

			attrib = attrib->Next();
		}

		// reading elements

	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'Drawing::DimStyle' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'Drawing::DimStyle' element.").arg(ex2.what()), FUNC_ID);
	}
}


TiXmlElement *Drawing::Insert::writeXML(TiXmlElement *parent) const {
	TiXmlElement * e = new TiXmlElement("Insert");
	parent->LinkEndChild(e);

	if (m_currentBlockName != QString())
		e->SetAttribute("blockName", m_currentBlockName.toStdString());
	if (m_angle != 0.0)
		e->SetAttribute("angle", IBK::val2string<double>(m_angle));
	if (m_xScale != 1.0)
		e->SetAttribute("xScale", IBK::val2string<double>(m_xScale));
	if (m_yScale != 1.0)
		e->SetAttribute("yScale", IBK::val2string<double>(m_yScale));
	if (m_zScale != 1.0)
		e->SetAttribute("zScale", IBK::val2string<double>(m_zScale));

	return e;
}


void Drawing::Insert::readXML(const TiXmlElement *element) {
	FUNCID(Drawing::DimStyle::readXMLPrivate);

	try {
		// search for mandatory attributes
		if (!TiXmlAttribute::attributeByName(element, "id"))
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
									  IBK::FormatString("Missing required 'id' attribute.") ), FUNC_ID);

		const TiXmlAttribute * attrib = element->FirstAttribute();
		while (attrib) {
			const std::string & attribName = attrib->NameStr();
			if (attribName == "blockName")
				m_currentBlockName = QString::fromStdString(attrib->ValueStr());
			else if (attribName == "angle")
				m_angle= NANDRAD::readPODAttributeValue<double>(element, attrib);
			else if (attribName == "xScale")
				m_xScale = NANDRAD::readPODAttributeValue<double>(element, attrib);
			else if (attribName == "yScale")
				m_yScale = NANDRAD::readPODAttributeValue<double>(element, attrib);
			else if (attribName == "zScale")
				m_zScale = NANDRAD::readPODAttributeValue<bool>(element, attrib);

			attrib = attrib->Next();
		}

		// reading elements

	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'Drawing::Insert' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'Drawing::Insert' element.").arg(ex2.what()), FUNC_ID);
	}
}

TiXmlElement * Drawing::Block::writeXML(TiXmlElement * parent) const {
	if (m_id == VICUS::INVALID_ID)  return nullptr;

	TiXmlElement * e = new TiXmlElement("Block");
	parent->LinkEndChild(e);

	if (m_id != VICUS::INVALID_ID)
		e->SetAttribute("id", IBK::val2string<unsigned int>(m_id));
	if (m_color.isValid())
		e->SetAttribute("color", m_color.name().toStdString());
	if (!m_name.isEmpty())
		e->SetAttribute("name", m_name.toStdString());
	if (m_lineWeight > 0)
		e->SetAttribute("lineWeight", IBK::val2string<unsigned int>(m_lineWeight));

	return e;
}

void Drawing::Block::readXML(const TiXmlElement *element){
	FUNCID(Drawing::Circle::readXMLPrivate);

	try {
		// search for mandatory attributes
		if (!TiXmlAttribute::attributeByName(element, "id"))
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Missing required 'id' attribute.") ), FUNC_ID);

		// reading attributes
		const TiXmlAttribute * attrib = element->FirstAttribute();
		while (attrib) {
			const std::string & attribName = attrib->NameStr();
			if (attribName == "id")
				m_id = NANDRAD::readPODAttributeValue<unsigned int>(element, attrib);
			else if (attribName == "lineWeight")
				m_lineWeight = NANDRAD::readPODAttributeValue<unsigned int>(element, attrib);
			else if (attribName == "Name")
				m_name = QString::fromStdString(attrib->ValueStr());
			else if (attribName == "color")
				m_color.setNamedColor(QString::fromStdString(attrib->ValueStr()));
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ATTRIBUTE).arg(attribName).arg(element->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			attrib = attrib->Next();
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'ZoneTemplate' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'ZoneTemplate' element.").arg(ex2.what()), FUNC_ID);
	}
}

} // namespace VICUS
