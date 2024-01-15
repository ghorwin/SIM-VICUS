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

static int PRECISION = 15;  // precision of floating point values for output writing

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


TiXmlElement * Drawing::Text::writeXMLPrivate(TiXmlElement * parent) const {
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

	TiXmlElement::appendSingleAttributeElement(e, "BasePoint", nullptr, std::string(), m_basePoint.toString(PRECISION));

	return e;
}

void Drawing::Text::readXMLPrivate(const TiXmlElement *element){
	FUNCID(Drawing::Text::readXMLPrivate);

	try {
		// search for mandatory attributes
		if (!TiXmlAttribute::attributeByName(element, "id")) {
			IBK::IBK_Message( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
								 IBK::FormatString("Missing required 'id' attribute.") ), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			return;
		}

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
			else if (attribName == "rotationAngle")
				m_rotationAngle = NANDRAD::readPODAttributeValue<double>(element, attrib);
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

const std::vector<PlaneGeometry> &Drawing::Text::planeGeometries() const {
	FUNCID(Drawing::Text::planeGeometries);
	try {
		if (m_dirtyTriangulation) {
			m_planeGeometries.clear();

			drawing()->generatePlanesFromText(m_text.toStdString(), m_height, m_alignment, m_rotationAngle, m_basePoint,
											  m_zPosition * Z_MULTIPLYER, m_planeGeometries);

			m_dirtyTriangulation = false;
		}

		return m_planeGeometries;
	} catch (IBK::Exception &ex) {
		throw IBK::Exception(IBK::FormatString("Could not generate plane geometries of 'Drawing::Text'\n%1").arg(ex.what()), FUNC_ID);
	}

}


TiXmlElement * Drawing::Solid::writeXMLPrivate(TiXmlElement * parent) const {
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

	TiXmlElement::appendSingleAttributeElement(e, "Point1", nullptr, std::string(), m_point1.toString(PRECISION));
	TiXmlElement::appendSingleAttributeElement(e, "Point2", nullptr, std::string(), m_point2.toString(PRECISION));
	TiXmlElement::appendSingleAttributeElement(e, "Point3", nullptr, std::string(), m_point3.toString(PRECISION));
	TiXmlElement::appendSingleAttributeElement(e, "Point4", nullptr, std::string(), m_point4.toString(PRECISION));

	return e;
}

void Drawing::Solid::readXMLPrivate(const TiXmlElement *element){
	FUNCID(Drawing::Solid::readXMLPrivate);

	try {
		// search for mandatory attributes
		if (!TiXmlAttribute::attributeByName(element, "id")) {
			IBK::IBK_Message( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
								 IBK::FormatString("Missing required 'id' attribute.") ), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			return;
		}

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
}

const std::vector<IBKMK::Vector2D> &Drawing::Solid::points2D() const {
	return m_pickPoints;
}

const std::vector<PlaneGeometry> &Drawing::Solid::planeGeometries() const {
	FUNCID(Drawing::Line::planeGeometries);
	try {
		if (m_dirtyTriangulation) {
			m_planeGeometries.clear();

			const VICUS::Drawing *drawing = this->drawing();
			Q_ASSERT(drawing);

			std::vector<IBKMK::Vector2D> points2D(4);
			points2D[0] = IBKMK::Vector2D(drawing->m_scalingFactor * m_point1.m_x,
										  drawing->m_scalingFactor * m_point1.m_y);
			points2D[0] = IBKMK::Vector2D(drawing->m_scalingFactor * m_point2.m_x,
										  drawing->m_scalingFactor * m_point2.m_y);
			points2D[0] = IBKMK::Vector2D(drawing->m_scalingFactor * m_point3.m_x,
										  drawing->m_scalingFactor * m_point3.m_y);
			points2D[0] = IBKMK::Vector2D(drawing->m_scalingFactor * m_point4.m_x,
										  drawing->m_scalingFactor * m_point4.m_y);

			const std::vector<IBKMK::Vector3D> &verts = drawing->points3D(points2D, m_zPosition);
			m_planeGeometries.push_back(VICUS::PlaneGeometry(verts));

			m_dirtyTriangulation = false;
		}

		return m_planeGeometries;
	} catch (IBK::Exception &ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error generating plane geometries for 'Drawing::Solid' element.\n%1").arg(ex.what()), FUNC_ID);
	}
}

TiXmlElement * Drawing::LinearDimension::writeXMLPrivate(TiXmlElement * parent) const {
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
	if (m_measurement != "")
		e->SetAttribute("measurement", m_measurement.toStdString());
	if (!m_layerName.isEmpty())
		e->SetAttribute("layer", m_layerName.toStdString());
	if (!m_styleName.isEmpty())
		e->SetAttribute("styleName", m_styleName.toStdString());

	TiXmlElement::appendSingleAttributeElement(e, "Point1", nullptr, std::string(), m_point1.toString(PRECISION));
	TiXmlElement::appendSingleAttributeElement(e, "Point2", nullptr, std::string(), m_point2.toString(PRECISION));
	TiXmlElement::appendSingleAttributeElement(e, "DimensionPoint", nullptr, std::string(), m_dimensionPoint.toString(PRECISION));
	TiXmlElement::appendSingleAttributeElement(e, "LeftPoint", nullptr, std::string(), m_leftPoint.toString(PRECISION));
	TiXmlElement::appendSingleAttributeElement(e, "RightPoint", nullptr, std::string(), m_rightPoint.toString(PRECISION));
	TiXmlElement::appendSingleAttributeElement(e, "TextPoint", nullptr, std::string(), m_textPoint.toString(PRECISION));

	return e;
}

void Drawing::LinearDimension::readXMLPrivate(const TiXmlElement *element){
	FUNCID(Drawing::LinearDimension::readXMLPrivate);

	try {
		// search for mandatory attributes
		if (!TiXmlAttribute::attributeByName(element, "id")) {
			IBK::IBK_Message( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
								 IBK::FormatString("Missing required 'id' attribute.") ), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			return;
		}

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
				m_angle = NANDRAD::readPODAttributeValue<double>(element, attrib);
			else if (attribName == "measurement")
				m_measurement = QString::fromStdString(attrib->ValueStr());
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

const std::vector<PlaneGeometry> &Drawing::LinearDimension::planeGeometries() const {
	FUNCID(Drawing::Text::planeGeometries);

	if (m_dirtyTriangulation) {
		m_planeGeometries.clear();

		if ((m_point1 - m_point2).magnitudeSquared() < 1E-2)
			return m_planeGeometries;

		const VICUS::Drawing *drawing = this->drawing();

		// Create Vector from start and end point of the line, add point of origin to each coordinate and calculate z value
		double zCoordinate = m_zPosition * Z_MULTIPLYER;

		m_pickPoints.push_back(m_leftPoint);
		m_pickPoints.push_back(m_rightPoint);

		// Dimension LINE ================================================================

		IBKMK::Vector3D p1 = IBKMK::Vector3D(drawing->m_scalingFactor * m_leftPoint.m_x,
											 drawing->m_scalingFactor * m_leftPoint.m_y,
											 zCoordinate);

		IBKMK::Vector3D p2 = IBKMK::Vector3D(drawing->m_scalingFactor * m_rightPoint.m_x,
											 drawing->m_scalingFactor * m_rightPoint.m_y,
											 zCoordinate);


		// rotate Vectors
		QVector3D vec1 = drawing->m_rotationMatrix.toQuaternion() * IBKVector2QVector(p1);
		QVector3D vec2 = drawing->m_rotationMatrix.toQuaternion() * IBKVector2QVector(p2);

		// move by origin
		vec1 += IBKVector2QVector(drawing->m_origin);
		vec2 += IBKVector2QVector(drawing->m_origin);

		m_planeGeometries.push_back(PlaneGeometry());
		bool success = drawing->generatePlaneFromLine(QVector2IBKVector(vec1), QVector2IBKVector(vec2),
													  drawing->m_lineWeightOffset + lineWeight() * drawing->m_lineWeightScaling,
													  m_planeGeometries.back());

		if (!success)
			return m_planeGeometries;

		// LINE LEFT  ================================================================

		IBKMK::Vector3D l (m_leftPoint - m_point1);
		IBKMK::Vector3D point;
		if (m_style->m_fixedExtensionLength) {
			IBKMK::Vector3D ext = drawing->m_scalingFactor * m_style->m_fixedExtensionLength * l.normalized();
			point.m_x = m_leftPoint.m_x - ext.m_x;
			point.m_y = m_leftPoint.m_y - ext.m_y;
		}
		else {
			IBKMK::Vector3D ext = drawing->m_scalingFactor * m_style->m_extensionLineLowerDistance * l.normalized();
			point.m_x = m_point1.m_x + ext.m_x;
			point.m_y = m_point1.m_y + ext.m_y;
		}

		IBKMK::Vector3D p1Left = IBKMK::Vector3D(drawing->m_scalingFactor * point.m_x,
												 drawing->m_scalingFactor * point.m_y,
												 zCoordinate);
		IBKMK::Vector3D lowerExtension = drawing->m_scalingFactor * m_style->m_upperLineDistance * l.normalized();
		IBKMK::Vector3D p2Left = IBKMK::Vector3D(drawing->m_scalingFactor * (m_leftPoint.m_x + lowerExtension.m_x),
												 drawing->m_scalingFactor * (m_leftPoint.m_y + lowerExtension.m_y),
												 zCoordinate);

		// rotate Vectors
		QVector3D vec1Left = drawing->m_rotationMatrix.toQuaternion() * IBKVector2QVector(p1Left);
		QVector3D vec2Left = drawing->m_rotationMatrix.toQuaternion() * IBKVector2QVector(p2Left);

		// move by origin
		vec1Left += IBKVector2QVector(drawing->m_origin);
		vec2Left += IBKVector2QVector(drawing->m_origin);

		//		QQuaternion q = QQuaternion::fromAxisAndAngle(QVector3D(vec1Left.x(), vec1Left.y(), 1), linDem.m_angle);
		//		vec2Left = q * vec2Left;

		m_planeGeometries.push_back(PlaneGeometry());
		success = drawing->generatePlaneFromLine(QVector2IBKVector(vec1Left), QVector2IBKVector(vec2Left),
												 drawing->m_lineWeightOffset + lineWeight() * drawing->m_lineWeightScaling,
												 m_planeGeometries.back());

		if (!success)
			return m_planeGeometries;

		// LINE RIGHT ================================================================

		IBKMK::Vector3D r (m_rightPoint - m_point2);
		if (m_style->m_fixedExtensionLength) {
			IBKMK::Vector3D ext = drawing->m_scalingFactor * m_style->m_fixedExtensionLength * r.normalized();
			point.m_x = m_rightPoint.m_x - ext.m_x;
			point.m_y = m_rightPoint.m_y - ext.m_y;
		}
		else {
			IBKMK::Vector3D ext = drawing->m_scalingFactor * m_style->m_extensionLineLowerDistance * r.normalized();
			point.m_x = m_point2.m_x + ext.m_x;
			point.m_y = m_point2.m_y + ext.m_y;
		}

		IBKMK::Vector3D p1Right = IBKMK::Vector3D(drawing->m_scalingFactor * point.m_x,
												  drawing->m_scalingFactor * point.m_y,
												  zCoordinate);
		lowerExtension = drawing->m_scalingFactor * m_style->m_upperLineDistance * l.normalized();
		IBKMK::Vector3D p2Right = IBKMK::Vector3D(drawing->m_scalingFactor * (m_rightPoint.m_x + lowerExtension.m_x),
												  drawing->m_scalingFactor * (m_rightPoint.m_y + lowerExtension.m_y),
												  zCoordinate);

		// rotate Vectors
		QVector3D vec1Right = drawing->m_rotationMatrix.toQuaternion() * IBKVector2QVector(p1Right);
		QVector3D vec2Right = drawing->m_rotationMatrix.toQuaternion() * IBKVector2QVector(p2Right);

		// move by origin
		vec1Right += IBKVector2QVector(drawing->m_origin);
		vec2Right += IBKVector2QVector(drawing->m_origin);

		m_planeGeometries.push_back(PlaneGeometry());
		success = drawing->generatePlaneFromLine(QVector2IBKVector(vec1Right), QVector2IBKVector(vec2Right),
												 drawing->m_lineWeightOffset + lineWeight() * drawing->m_lineWeightScaling,
												 m_planeGeometries.back());

		if (!success)
			return m_planeGeometries;

		// Text ======================================================================

		double length = (m_leftPoint - m_rightPoint).magnitude();
		m_pickPoints.push_back(m_textPoint);

		QString measurementText;
		if (m_measurement == "") {
			measurementText = QString("%1").arg(length / m_style->m_textLinearFactor,
												m_style->m_textDecimalPlaces, 'g');
		}
		else
			measurementText = m_measurement;

		drawing->generatePlanesFromText(measurementText.toStdString(),
										m_style->m_textHeight * m_style->m_globalScalingFactor * 2,
										Qt::AlignHCenter, m_angle, m_textPoint,
										m_zPosition * Z_MULTIPLYER, m_planeGeometries);

		m_dirtyTriangulation = false;
		m_dirtyPoints = false;
	}

	return m_planeGeometries;
}


TiXmlElement * Drawing::Point::writeXMLPrivate(TiXmlElement * parent) const {
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

	TiXmlElement::appendSingleAttributeElement(e, "Point", nullptr, std::string(), m_point.toString(PRECISION));

	return e;
}

void Drawing::Point::readXMLPrivate(const TiXmlElement *element){
	FUNCID(Drawing::Circle::readXMLPrivate);

	try {
		// search for mandatory attributes
		if (!TiXmlAttribute::attributeByName(element, "id")) {
			IBK::IBK_Message( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
								 IBK::FormatString("Missing required 'id' attribute.") ), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			return;
		}


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

const std::vector<PlaneGeometry> &Drawing::Point::planeGeometries() const {
	FUNCID(Drawing::Line::planeGeometries);
	try {
		if (m_dirtyTriangulation) {
			m_planeGeometries.clear();

			// Create Vector from point, add point of origin to each coordinate and calculate z value
			IBKMK::Vector3D p(drawing()->m_scalingFactor * m_point.m_x,
							  drawing()->m_scalingFactor * m_point.m_y,
							  m_zPosition * Z_MULTIPLYER);

			// scale Vector with selected unit
			double pointWeight = (drawing()->m_lineWeightOffset + lineWeight() * drawing()->m_lineWeightScaling) / 2;

			// rotation
			QVector3D vec = drawing()->m_rotationMatrix.toQuaternion() * IBKVector2QVector(p);
			vec += IBKVector2QVector(drawing()->m_origin);

			IBKMK::Vector3D p1 = QVector2IBKVector(vec);

			IBKMK::Vector3D pExt0 = IBKMK::Vector3D(p1.m_x - pointWeight, p1.m_y - pointWeight, p1.m_z);
			IBKMK::Vector3D pExt1 = IBKMK::Vector3D(p1.m_x + pointWeight, p1.m_y - pointWeight, p1.m_z);
			IBKMK::Vector3D pExt2 = IBKMK::Vector3D(p1.m_x - pointWeight, p1.m_y + pointWeight, p1.m_z);

			IBKMK::Polygon3D po(VICUS::Polygon2D::T_Rectangle, pExt0, pExt2, pExt1);

			for (const IBKMK::Vector3D &v3D : po.vertexes())
				const_cast<IBKMK::Vector3D &>(v3D) = QVector2IBKVector(QMatrix4x4() * IBKVector2QVector(v3D));


			m_planeGeometries.push_back(VICUS::PlaneGeometry(po));
			m_dirtyTriangulation = false;
		}

		return m_planeGeometries;
	} catch (IBK::Exception &ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error generating plane geometries for 'Drawing::Point' element.\n%1").arg(ex.what()), FUNC_ID);
	}
}

void Drawing::Line::readXMLPrivate(const TiXmlElement *element){
	FUNCID(Drawing::Circle::readXMLPrivate);

	try {
		// search for mandatory attributes
		if (!TiXmlAttribute::attributeByName(element, "id")) {
			IBK::IBK_Message( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
								 IBK::FormatString("Missing required 'id' attribute.") ), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			return;
		}

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

const std::vector<PlaneGeometry> &Drawing::Line::planeGeometries() const {
	FUNCID(Drawing::Line::planeGeometries);

	if (m_dirtyTriangulation) {
		m_planeGeometries.clear();

		const VICUS::Drawing *drawing = this->drawing();

		VICUS::PlaneGeometry plane;
		// Create Vector from start and end point of the line, add point of origin to each coordinate and calculate z value
		double zCoordinate = m_zPosition * Z_MULTIPLYER;
		IBKMK::Vector3D p1 = IBKMK::Vector3D(drawing->m_scalingFactor * m_point1.m_x,
											 drawing->m_scalingFactor * m_point1.m_y,
											 zCoordinate);
		IBKMK::Vector3D p2 = IBKMK::Vector3D(drawing->m_scalingFactor * m_point2.m_x,
											 drawing->m_scalingFactor * m_point2.m_y,
											 zCoordinate);

		// rotate Vectors
		QVector3D vec1 = drawing->m_rotationMatrix.toQuaternion() * IBKVector2QVector(p1);
		QVector3D vec2 = drawing->m_rotationMatrix.toQuaternion() * IBKVector2QVector(p2);

		// scale Vector with selected unit
		vec1 += IBKVector2QVector(drawing->m_origin);
		vec2 += IBKVector2QVector(drawing->m_origin);

		bool success = drawing->generatePlaneFromLine(QVector2IBKVector(vec1), QVector2IBKVector(vec2),
													  drawing->m_lineWeightOffset + lineWeight() * drawing->m_lineWeightScaling,
													  plane);

		if (!success)
			IBK::IBK_Message(IBK::FormatString("Could not generate plane from line #%1").arg(m_id), IBK::MSG_WARNING);

		m_planeGeometries.push_back(plane);
		m_dirtyTriangulation = false;
	}

	return m_planeGeometries;
}

TiXmlElement * Drawing::Line::writeXMLPrivate(TiXmlElement * parent) const {
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

	TiXmlElement::appendSingleAttributeElement(e, "Point1", nullptr, std::string(), m_point1.toString(PRECISION));
	TiXmlElement::appendSingleAttributeElement(e, "Point2", nullptr, std::string(), m_point2.toString(PRECISION));

	return e;
}


TiXmlElement * Drawing::Circle::writeXMLPrivate(TiXmlElement * parent) const {
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

	TiXmlElement::appendSingleAttributeElement(e, "Center", nullptr, std::string(), m_center.toString(PRECISION));
	TiXmlElement::appendSingleAttributeElement(e, "Radius", nullptr, std::string(), IBK::val2string<double>(m_radius, PRECISION));

	return e;
}

void Drawing::Circle::readXMLPrivate(const TiXmlElement *element){
	FUNCID(Drawing::Circle::readXMLPrivate);

	try {
		// search for mandatory attributes
		if (!TiXmlAttribute::attributeByName(element, "id")) {
			IBK::IBK_Message( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
								 IBK::FormatString("Missing required 'id' attribute.") ), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			return;
		}

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


const std::vector<PlaneGeometry> &Drawing::Circle::planeGeometries() const {
	FUNCID(Drawing::Circle::planeGeometries);
	try {
		if (m_dirtyTriangulation) {
			m_planeGeometries.clear();

			std::vector<IBKMK::Vector3D> circlePoints;

			const VICUS::Drawing *drawing = this->drawing();

			for(unsigned int i = 0; i < SEGMENT_COUNT_CIRCLE; i++){
				IBKMK::Vector3D p = IBKMK::Vector3D(drawing->m_scalingFactor * points2D()[i].m_x,
													drawing->m_scalingFactor * points2D()[i].m_y,
													m_zPosition * Z_MULTIPLYER);

				QVector3D vec = drawing->m_rotationMatrix.toQuaternion() * IBKVector2QVector(p);
				vec += IBKVector2QVector(drawing->m_origin);

				circlePoints.push_back(QVector2IBKVector(vec));
			}

			bool success = drawing->generatePlanesFromPolyline(circlePoints, true,
															   drawing->m_lineWeightOffset + lineWeight() * drawing->m_lineWeightScaling,
															   m_planeGeometries);

			if (!success)
				return m_planeGeometries;

			m_dirtyTriangulation = false;
		}

		return m_planeGeometries;
	} catch (IBK::Exception &ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error generating plane geometries for 'Drawing::Circle' element.\n%1").arg(ex.what()), FUNC_ID);

	}
}


TiXmlElement * Drawing::PolyLine::writeXMLPrivate(TiXmlElement * parent) const {
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
			vals << polyVertexes[i].toString(PRECISION);
			if (i<polyVertexes.size()-1)  vals << ", ";
		}
		TiXmlText * text = new TiXmlText( vals.str() );
		e->LinkEndChild( text );
	}

	return e;
}

void Drawing::PolyLine::readXMLPrivate(const TiXmlElement *element){
	FUNCID(Drawing::Arc::readXMLPrivate);

	try {
		// search for mandatory attributes
		if (!TiXmlAttribute::attributeByName(element, "id")) {
			IBK::IBK_Message( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
								 IBK::FormatString("Missing required 'id' attribute.") ), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			return;
		}

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

const std::vector<PlaneGeometry> &Drawing::PolyLine::planeGeometries() const {
	FUNCID(Drawing::PolyLine::planeGeometries);

	if (m_dirtyTriangulation) {
		m_planeGeometries.clear();

		// Create Vector to store vertices of polyline
		std::vector<IBKMK::Vector3D> polylinePoints;

		const VICUS::Drawing *drawing = this->drawing();

		// adds z-coordinate to polyline
		for(unsigned int i = 0; i < m_polyline.size(); ++i){
			IBKMK::Vector3D p = IBKMK::Vector3D(drawing->m_scalingFactor * m_polyline[i].m_x,
												drawing->m_scalingFactor * m_polyline[i].m_y,
												m_zPosition * Z_MULTIPLYER);
			QVector3D vec = drawing->m_rotationMatrix.toQuaternion() * IBKVector2QVector(p);
			vec += IBKVector2QVector(drawing->m_origin);

			polylinePoints.push_back(QVector2IBKVector(vec));
		}

		bool success = drawing->generatePlanesFromPolyline(polylinePoints, m_endConnected,
														   drawing->m_lineWeightOffset + lineWeight() * drawing->m_lineWeightScaling,
														   m_planeGeometries);

		if (!success)
			return m_planeGeometries;

		m_dirtyTriangulation = false;
	}

	return m_planeGeometries;
}


TiXmlElement * Drawing::Arc::writeXMLPrivate(TiXmlElement * parent) const {
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

	TiXmlElement::appendSingleAttributeElement(e, "Center", nullptr, std::string(), m_center.toString(PRECISION));
	TiXmlElement::appendSingleAttributeElement(e, "Radius", nullptr, std::string(), IBK::val2string<double>(m_radius, PRECISION));
	TiXmlElement::appendSingleAttributeElement(e, "StartAngle", nullptr, std::string(), IBK::val2string<double>(m_startAngle, PRECISION));
	TiXmlElement::appendSingleAttributeElement(e, "EndAngle", nullptr, std::string(), IBK::val2string<double>(m_endAngle, PRECISION));

	return e;
}

void Drawing::Arc::readXMLPrivate(const TiXmlElement *element){
	FUNCID(Drawing::Arc::readXMLPrivate);

	try {
		// search for mandatory attributes
		if (!TiXmlAttribute::attributeByName(element, "id")) {
			IBK::IBK_Message( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
								 IBK::FormatString("Missing required 'id' attribute.") ), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			return;
		}

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

const std::vector<PlaneGeometry> &Drawing::Arc::planeGeometries() const {
	FUNCID(Drawing::Arc::planeGeometries);

	if (m_dirtyTriangulation) {
		m_planeGeometries.clear();

		std::vector<IBKMK::Vector3D> arcPoints;
		const VICUS::Drawing *drawing = this->drawing();

		const std::vector<IBKMK::Vector2D> &arcPoints2D = points2D();
		for (unsigned int i = 0; i < arcPoints2D.size(); ++i){
			IBKMK::Vector3D p = IBKMK::Vector3D(drawing->m_scalingFactor * arcPoints2D[i].m_x,
												drawing->m_scalingFactor * arcPoints2D[i].m_y,
												m_zPosition * Z_MULTIPLYER);
			QVector3D vec = drawing->m_rotationMatrix.toQuaternion() * IBKVector2QVector(p);
			vec += IBKVector2QVector(drawing->m_origin);

			arcPoints.push_back(QVector2IBKVector(vec));
		}

		bool success = drawing->generatePlanesFromPolyline(arcPoints, false,
														   drawing->m_lineWeightOffset + lineWeight() * drawing->m_lineWeightScaling,
														   m_planeGeometries);

		if (!success)
			return m_planeGeometries;

		m_dirtyTriangulation = false;
	}

	return m_planeGeometries;
}


TiXmlElement * Drawing::Ellipse::writeXMLPrivate(TiXmlElement * parent) const {
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

	TiXmlElement::appendSingleAttributeElement(e, "Center", nullptr, std::string(), m_center.toString(PRECISION));
	TiXmlElement::appendSingleAttributeElement(e, "MajorAxis", nullptr, std::string(), m_majorAxis.toString(PRECISION));
	TiXmlElement::appendSingleAttributeElement(e, "Ratio", nullptr, std::string(), IBK::val2string<double>(m_ratio));
	TiXmlElement::appendSingleAttributeElement(e, "StartAngle", nullptr, std::string(), IBK::val2string<double>(m_startAngle, PRECISION));
	TiXmlElement::appendSingleAttributeElement(e, "EndAngle", nullptr, std::string(), IBK::val2string<double>(m_endAngle, PRECISION));

	return e;
}


void Drawing::Ellipse::readXMLPrivate(const TiXmlElement *element){
	FUNCID(Drawing::Arc::readXMLPrivate);

	try {
		// search for mandatory attributes
		if (!TiXmlAttribute::attributeByName(element, "id")) {
			IBK::IBK_Message( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
								 IBK::FormatString("Missing required 'id' attribute.") ), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			return;
		}

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


const std::vector<PlaneGeometry> &Drawing::Ellipse::planeGeometries() const {
	FUNCID(Drawing::Ellipse::planeGeometries);

	if (m_dirtyTriangulation) {
		m_planeGeometries.clear();

		const std::vector<IBKMK::Vector2D> &pickPoints = points2D();
		const VICUS::Drawing *drawing = this->drawing();

		std::vector<IBKMK::Vector3D> ellipsePoints;
		for (unsigned int i = 0; i < SEGMENT_COUNT_ELLIPSE; i++) {

			IBKMK::Vector3D p = IBKMK::Vector3D(drawing->m_scalingFactor * pickPoints[i].m_x,
												drawing->m_scalingFactor * pickPoints[i].m_y,
												m_zPosition * Z_MULTIPLYER);

			QVector3D vec = drawing->m_rotationMatrix.toQuaternion() * IBKVector2QVector(p);
			vec += IBKVector2QVector(drawing->m_origin);

			ellipsePoints.push_back(QVector2IBKVector(vec));
		}

		Q_ASSERT(points2D().size() > 0);

		bool connect = pickPoints[0] == pickPoints.back();
		bool success = drawing->generatePlanesFromPolyline(ellipsePoints, connect,
												  drawing->m_lineWeightOffset + lineWeight() * drawing->m_lineWeightScaling,
												  m_planeGeometries);

		if (!success)
			return m_planeGeometries;

		m_dirtyTriangulation = false;
	}

	return m_planeGeometries;
}


const Drawing::AbstractDrawingObject *Drawing::objectByID(unsigned int id) const {
	FUNCID(Drawing::objectByID);

	Q_ASSERT(m_objectPtr.find(id) != m_objectPtr.end());
	AbstractDrawingObject *obj = m_objectPtr.at(id);
	if (obj == nullptr)
		throw IBK::Exception(IBK::FormatString("Drawing Object with ID #%1 not found").arg(id), FUNC_ID);

	return obj;
}


Drawing::Block *Drawing::findBlockPointer(const QString &name, const std::map<QString, Block*> &blockRefs){
	const auto it = blockRefs.find(name);
	if (it == blockRefs.end())
		return nullptr;
	else
		return it->second;
}

void Drawing::updatePointer(){
	FUNCID(Drawing::updatePointer);
	m_objectPtr.clear();

	// map layer name to reference, this avoids nested loops
	std::map<QString, const DrawingLayer*> layerRefs;
	for (const DrawingLayer &dl: m_drawingLayers) {
		layerRefs[dl.m_displayName] = &dl;
	}

	// map block name to reference, also avoids nested loops
	std::map<QString, Block*> blockRefs;
	blockRefs[""] = nullptr; // This is just in case. But actually blocks without name should not exist
	for (Block &b: m_blocks) {
		blockRefs[b.m_name] = &b;
	}

	/* Note: Layer references must always be valid. Hence, when "layerRefs.at()" throws an exception, this is due to an invalid DXF.
	 * Block references are optional, therefore we use the access function which returns a nullptr if there is no block ref
	*/
	try {

		for (unsigned int i=0; i < m_points.size(); ++i){
			m_points[i].m_layerRef = layerRefs.at(m_points[i].m_layerName);
			m_points[i].m_block = findBlockPointer(m_points[i].m_blockName, blockRefs);
			m_objectPtr[m_points[i].m_id] = &m_points[i];
		}
		for (unsigned int i=0; i < m_lines.size(); ++i){
			m_lines[i].m_layerRef = layerRefs.at(m_lines[i].m_layerName);
			m_lines[i].m_block = findBlockPointer(m_lines[i].m_blockName, blockRefs);
			m_objectPtr[m_lines[i].m_id] = &m_lines[i];
		}
		for (unsigned int i=0; i < m_polylines.size(); ++i){
			m_polylines[i].m_layerRef = layerRefs.at(m_polylines[i].m_layerName);
			m_polylines[i].m_block = findBlockPointer(m_polylines[i].m_blockName, blockRefs);
			m_objectPtr[m_polylines[i].m_id] = &m_polylines[i];
		}
		for (unsigned int i=0; i < m_circles.size(); ++i){
			m_circles[i].m_layerRef = layerRefs.at(m_circles[i].m_layerName);
			m_circles[i].m_block = findBlockPointer(m_circles[i].m_blockName, blockRefs);
			m_objectPtr[m_circles[i].m_id] = &m_circles[i];
		}
		for (unsigned int i=0; i < m_arcs.size(); ++i){
			m_arcs[i].m_layerRef = layerRefs.at(m_arcs[i].m_layerName);
			m_arcs[i].m_block = findBlockPointer(m_arcs[i].m_blockName, blockRefs);
			m_objectPtr[m_arcs[i].m_id] = &m_arcs[i];
		}
		for (unsigned int i=0; i < m_ellipses.size(); ++i){
			m_ellipses[i].m_layerRef = layerRefs.at(m_ellipses[i].m_layerName);
			m_ellipses[i].m_block = findBlockPointer(m_ellipses[i].m_blockName, blockRefs);
			m_objectPtr[m_ellipses[i].m_id] = &m_ellipses[i];
		}
		for (unsigned int i=0; i < m_solids.size(); ++i){
			m_solids[i].m_layerRef = layerRefs.at(m_solids[i].m_layerName);
			m_solids[i].m_block = findBlockPointer(m_solids[i].m_blockName, blockRefs);
			m_objectPtr[m_solids[i].m_id] = &m_solids[i];
		}
		for (unsigned int i=0; i < m_texts.size(); ++i){
			m_texts[i].m_layerRef = layerRefs.at(m_texts[i].m_layerName);
			m_texts[i].m_block = findBlockPointer(m_texts[i].m_blockName, blockRefs);
			m_objectPtr[m_texts[i].m_id] = &m_texts[i];
		}

		// For inserts there must be a valid currentBlock reference!
		for (unsigned int i=0; i < m_inserts.size(); ++i){
			m_inserts[i].m_currentBlock = findBlockPointer(m_inserts[i].m_currentBlockName, blockRefs);
			Q_ASSERT(m_inserts[i].m_currentBlock);
			m_inserts[i].m_parentBlock = findBlockPointer(m_inserts[i].m_parentBlockName, blockRefs);
		}
		for (unsigned int i=0; i < m_linearDimensions.size(); ++i){
			m_linearDimensions[i].m_layerRef = layerRefs.at(m_linearDimensions[i].m_layerName);
			m_texts[i].m_block = findBlockPointer(m_linearDimensions[i].m_blockName, blockRefs);
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
	}
	catch (std::exception &ex) {
		throw IBK::Exception(IBK::FormatString("Error during initialization of DXF file. "
											   "Might be du to invalid layer references.\n%1").arg(ex.what()), FUNC_ID);
	}
}

template <typename t>
void updateGeometry(std::vector<t> &objects) {
	for (t &obj : objects )
		obj.updatePlaneGeometry();
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
		newObj.m_isInsertObject = true;

		newObjects.push_back(newObj);
	}

	objects.insert(objects.end(), newObjects.begin(), newObjects.end());
}

void Drawing::transformInsert(QMatrix4x4 trans, const VICUS::Drawing::Insert &insert, unsigned int &nextId) {

	Q_ASSERT(insert.m_currentBlock != nullptr);
	IBKMK::Vector2D insertPoint = insert.m_insertionPoint - insert.m_currentBlock->m_basePoint;

	trans.translate(QVector3D(float(insertPoint.m_x),
							  float(insertPoint.m_y),
							  0.0));
	trans.rotate(float(insert.m_angle/IBK::DEG2RAD), QVector3D(0,0,1));
	trans.scale(float(insert.m_xScale), float(insert.m_yScale), 1);

	for (const Insert &i : m_inserts) {
		if (i.m_parentBlock == nullptr)
			continue;

		if (insert.m_currentBlockName == i.m_parentBlock->m_name) {
			transformInsert(trans, i, nextId); // we pass "trans" by value, to keep our own transformation untouched
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
	generateObjectFromInsert(nextId, *insert.m_currentBlock, m_linearDimensions, trans);
}


void transformPoint(IBKMK::Vector2D &vec, const QMatrix4x4 &trans) {
	IBKMK::Vector3D v3 = QVector2IBKVector(trans * QVector3D((float)vec.m_x, (float)vec.m_y, 0));
	vec = v3.point2D();
}


void Drawing::generateInsertGeometries(unsigned int nextId) {
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

	// apply the transformation to each object and then reset it
	for (Point &p: m_points) {
		if (p.m_isInsertObject) {
			transformPoint(p.m_point, p.m_trans);
			p.m_trans = QMatrix4x4();
		}
	}
	for (Arc &a: m_arcs) {
		if (a.m_isInsertObject) {
			transformPoint(a.m_center, a.m_trans);
			a.m_trans = QMatrix4x4();
		}
	}
	for (Circle &c: m_circles) {
		if (c.m_isInsertObject) {
			transformPoint(c.m_center, c.m_trans);
			c.m_trans = QMatrix4x4();
		}
	}
	for (Ellipse &e: m_ellipses) {
		if (e.m_isInsertObject) {
			transformPoint(e.m_center, e.m_trans);
			e.m_trans = QMatrix4x4();
		}
	}
	for (Line &l: m_lines) {
		if (l.m_isInsertObject) {
			transformPoint(l.m_point1, l.m_trans);
			transformPoint(l.m_point2, l.m_trans);
			l.m_trans = QMatrix4x4();
		}
	}
	for (PolyLine &pl: m_polylines) {
		if (pl.m_isInsertObject) {
			for (IBKMK::Vector2D &v: pl.m_polyline) {
				transformPoint(v, pl.m_trans);
			}
			pl.m_trans = QMatrix4x4();
		}
	}
	for (Solid &s: m_solids) {
		if (s.m_isInsertObject) {
			transformPoint(s.m_point1, s.m_trans);
			transformPoint(s.m_point2, s.m_trans);
			transformPoint(s.m_point3, s.m_trans);
			transformPoint(s.m_point4, s.m_trans);
			s.m_trans = QMatrix4x4();
		}
	}
	for (Text &t: m_texts) {
		if (t.m_isInsertObject) {
			transformPoint(t.m_basePoint, t.m_trans);
			t.m_trans = QMatrix4x4();
		}
	}
	for (LinearDimension &ld: m_linearDimensions) {
		if (ld.m_isInsertObject) {
			transformPoint(ld.m_point1, ld.m_trans);
			transformPoint(ld.m_point2, ld.m_trans);
			transformPoint(ld.m_dimensionPoint, ld.m_trans);
			transformPoint(ld.m_leftPoint, ld.m_trans);
			transformPoint(ld.m_rightPoint, ld.m_trans);
			transformPoint(ld.m_textPoint, ld.m_trans);
			ld.m_trans = QMatrix4x4();
		}
	}

	updateParents();
}


template <typename T>
void updateGeometryForAll(std::vector<T>& objects) {
	for (T& obj : objects) {
		obj.updatePlaneGeometry();
	}
}

void Drawing::updateAllGeometries() {
	updateGeometryForAll(m_points);
	updateGeometryForAll(m_lines);
	updateGeometryForAll(m_polylines);
	updateGeometryForAll(m_circles);
	updateGeometryForAll(m_ellipses);
	updateGeometryForAll(m_arcs);
	updateGeometryForAll(m_solids);
	updateGeometryForAll(m_texts);
	updateGeometryForAll(m_linearDimensions);
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

		const std::vector<IBKMK::Vector3D> &objVerts = d.points3D(obj.points2D(), obj.m_zPosition);
		verts[obj.m_id] = objVerts;
	}
}


const std::map<unsigned int, std::vector<IBKMK::Vector3D>> &Drawing::pickPoints() const {
	FUNCID(Drawing::pickPoints);
	try {
		if (m_dirtyPickPoints) {
			m_pickPoints.clear();
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


const std::vector<IBKMK::Vector3D> Drawing::points3D(const std::vector<IBKMK::Vector2D> &verts, unsigned int zPosition) const {

	std::vector<IBKMK::Vector3D> points3D(verts.size());

	for (unsigned int i=0; i < verts.size(); ++i) {
		const IBKMK::Vector2D &v2D = verts[i];
		double zCoordinate = zPosition * Z_MULTIPLYER;
		IBKMK::Vector3D v3D = IBKMK::Vector3D(v2D.m_x * m_scalingFactor,
											  v2D.m_y * m_scalingFactor,
											  zCoordinate);

		QVector3D qV3D = m_rotationMatrix.toQuaternion() * IBKVector2QVector(v3D);
		qV3D += IBKVector2QVector(m_origin);
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


const DrawingLayer * Drawing::layerPointer(const QString &layername){
	for(unsigned int i = 0; i < m_drawingLayers.size(); ++i) {
		if (m_drawingLayers[i].m_displayName == layername)
			return &m_drawingLayers[i];
	}
	return nullptr;
}

const Drawing::Block *Drawing::blockPointer(const QString &name){
	for(unsigned int i = 0; i < m_blocks.size(); ++i) {
		if (m_blocks[i].m_name == name)
			return &m_blocks[i];
	}
	return nullptr;
}


bool Drawing::generatePlaneFromLine(const IBKMK::Vector3D &startPoint, const IBKMK::Vector3D &endPoint, double width, VICUS::PlaneGeometry &plane) const {

	// Calculate the line vector and its length
	IBKMK::Vector3D lineVector = endPoint - startPoint;
	double length = lineVector.magnitude();
	if(length < 1e-4)
		return false;

	// Calculate the line width (1 pixel)
	double halfWidth = width / 2.0;

	// Calculate a perpendicular vector for the line width
	IBKMK::Vector3D normal((double)m_rotationMatrix.toQuaternion().toRotationMatrix()(0,2),
						   (double)m_rotationMatrix.toQuaternion().toRotationMatrix()(1,2),
						   (double)m_rotationMatrix.toQuaternion().toRotationMatrix()(2,2));

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

	// Call addPlane to create the box geometry twice so visible from both sides
	IBKMK::Polygon3D p(VICUS::Polygon2D::T_Rectangle, lineVertices[0], lineVertices[3], lineVertices[1]);
	plane = VICUS::PlaneGeometry(p);

	return true;
}


bool Drawing::generatePlanesFromPolyline(const std::vector<IBKMK::Vector3D> &polyline,
										 bool connectEndStart, double width, std::vector<PlaneGeometry> &planes) const {

	// initialise values
	IBKMK::Vector3D lineVector, previousVector, crossProduct, perpendicularVector;
	std::vector<IBKMK::Vector3D> previousVertices;
	double halfWidth = width / 2;
	double length;

	// if polyline is empty, return
	if(polyline.size() < 2){
		return false;
	}

	// initialise previousVector
	previousVector = polyline[1] - polyline[0];

	auto processSegment = [&](const IBKMK::Vector3D& startPoint, const IBKMK::Vector3D& endPoint)->void {
		// calculate line vector
		lineVector = endPoint - startPoint;
		length = lineVector.magnitude();
		if(length <= 0)
			return;

		IBKMK::Vector3D normal(m_rotationMatrix.toQuaternion().toRotationMatrix()(0,2),
							   m_rotationMatrix.toQuaternion().toRotationMatrix()(1,2),
							   m_rotationMatrix.toQuaternion().toRotationMatrix()(2,2));

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
	for (unsigned int i = 0; i < polyline.size() - 1; i++) {
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


void Drawing::generatePlanesFromText(const std::string &text, double textHeight, Qt::Alignment alignment,
									 const double &rotationAngle, const IBKMK::Vector2D &basePoint, double zPositon,
									 std::vector<VICUS::PlaneGeometry> &planeGeometries) const {

	if (text.empty())
		return;

	// We choose Arial for now
	QFont font("Arial");
	font.setPointSize(2);
	// Create a QPainterPath object
	QPainterPath path;
	path.addText(0, 0, font, QString::fromStdString(text)); // 50 is roughly the baseline for the text

	double width = path.boundingRect().width();
	double moveX = 0.0;

	// Adjust
	if (alignment == Qt::AlignHCenter)
		moveX = -0.5*width;


	QTransform transformation;
	transformation.rotate(rotationAngle);  // Rotate by 45 degrees
	transformation.translate(moveX, 0.0);

	// Apply the rotation to the path
	QPainterPath rotatedPath = transformation.map(path);

	// Extract polygons from the path
	QList<QPolygonF> polygons = rotatedPath.toSubpathPolygons();

	double scalingFactorFonts = DEFAULT_FONT_SCALING * m_scalingFactor ;
//	std::max(textHeight * DEFAULT_FONT_SCALING * m_scalingFactor,
//										 DEFAULT_FONT_SIZE);

	qDebug() << "Text size: " << scalingFactorFonts;
	qDebug() << "Rotation angle: " << rotationAngle;

	if (polygons.empty()) {
		IBK::IBK_Message(IBK::FormatString("Could not render text '%1'. Skipping").arg(text), IBK::MSG_WARNING);
		return;
	}

	for (int i=0; i < polygons.size(); ++i) {

		const QPolygonF &polygon = polygons[i];
		std::vector<IBKMK::Vector2D> poly(polygon.size());

		for (unsigned int i=0; i<poly.size(); ++i) {
			const QPointF &point = polygon[i];
			// double zCoordinate = obj->m_zPosition * Z_MULTIPLYER + d->m_origin.m_z;
			poly[i] = IBKMK::Vector2D(  point.x() * scalingFactorFonts + basePoint.m_x,
									   -point.y() * scalingFactorFonts + basePoint.m_y);
		}

		IBKMK::Polygon3D poly3D;
		try {
			poly3D = IBKMK::Polygon3D (points3D(poly, zPositon));
		} catch (IBK::Exception &) {
			IBK::IBK_Message(IBK::FormatString("Could not render text '%1'. Skipping").arg(text), IBK::MSG_WARNING);
			return;
		}

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
	else if (m_layerRef != nullptr) {
		const DrawingLayer *layer = m_layerRef;
		Q_ASSERT(layer != nullptr);
		return layer->m_color;
	}

	return m_color;
}


double Drawing::AbstractDrawingObject::lineWeight() const{
	// ToDo Stephan: Improve function

	/*! if -1: use weight of layer */
	const DrawingLayer *dl = m_layerRef;

	if (dl == nullptr)
		return 0;

	if (m_lineWeight <= 0)
		return dl->m_lineWeight;
	else
		return m_lineWeight;
//	/*! if -3: default lineWeight is used
//		if -2: lineWeight of block is used. Needs to be modified when blocks
//		are implemented
//	*/
//	else if (m_lineWeight == -3 || m_lineWeight == -2)
//		return 0;
//	else
//		return m_lineWeight;
}



// *** XML Read/Write



TiXmlElement *Drawing::DimStyle::writeXML(TiXmlElement *parent) const {
	if (m_id == VICUS::INVALID_ID)  return nullptr;

	TiXmlElement * e = new TiXmlElement("DimStyle");
	parent->LinkEndChild(e);

	if (m_id != VICUS::INVALID_ID)
		e->SetAttribute("id", IBK::val2string<unsigned int>(m_id));
	if (!m_name.isEmpty())
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
	if (m_globalScalingFactor != 1.0)
		e->SetAttribute("globalScalingFactor", IBK::val2string<double>(m_globalScalingFactor));
	if (m_globalScalingFactor != 1.0)
		e->SetAttribute("textScalingFactor", IBK::val2string<double>(m_textScalingFactor));
	if (m_textLinearFactor != 1.0)
		e->SetAttribute("textLinearFactor", IBK::val2string<double>(m_textLinearFactor));
	if (m_textDecimalPlaces != 1.0)
		e->SetAttribute("textDecimalPlaces", IBK::val2string<int>(m_textDecimalPlaces));

	return e;
}

void Drawing::DimStyle::readXML(const TiXmlElement *element) {
	FUNCID(Drawing::DimStyle::readXMLPrivate);

	try {
		// search for mandatory attributes
		if (!TiXmlAttribute::attributeByName(element, "id")) {
			IBK::IBK_Message( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
								 IBK::FormatString("Missing required 'id' attribute.") ), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			return;
		}


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
			else if (attribName == "globalScalingFactor")
				m_globalScalingFactor = NANDRAD::readPODAttributeValue<double>(element, attrib);
			else if (attribName == "textLinearFactor")
				m_textLinearFactor = NANDRAD::readPODAttributeValue<double>(element, attrib);
			else if (attribName == "textDecimalPlaces")
				m_textDecimalPlaces = NANDRAD::readPODAttributeValue<int>(element, attrib);

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


TiXmlElement * Drawing::Block::writeXML(TiXmlElement * parent) const {
	if (m_id == INVALID_ID)  return nullptr;

	TiXmlElement * e = new TiXmlElement("Block");
	parent->LinkEndChild(e);

	if (m_id != INVALID_ID)
		e->SetAttribute("id", IBK::val2string<unsigned int>(m_id));
	if (m_color.isValid())
		e->SetAttribute("color", m_color.name().toStdString());
	if (!m_name.isEmpty())
		e->SetAttribute("name", m_name.toStdString());
	if (m_lineWeight > 0)
		e->SetAttribute("lineWeight", IBK::val2string<int>(m_lineWeight));

	TiXmlElement::appendSingleAttributeElement(e, "basePoint", nullptr, std::string(), m_basePoint.toString(PRECISION));

	return e;
}


void Drawing::Block::readXML(const TiXmlElement *element){
	FUNCID(Drawing::Circle::readXMLPrivate);

	try {
		// search for mandatory attributes
		if (!TiXmlAttribute::attributeByName(element, "id")) {
			IBK::IBK_Message( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
								 IBK::FormatString("Missing required 'id' attribute.") ), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			return;
		}

		// reading attributes
		const TiXmlAttribute * attrib = element->FirstAttribute();
		while (attrib) {
			const std::string & attribName = attrib->NameStr();
			if (attribName == "id")
				m_id = NANDRAD::readPODAttributeValue<unsigned int>(element, attrib);
			else if (attribName == "lineWeight")
				m_lineWeight = NANDRAD::readPODAttributeValue<int>(element, attrib);
			else if (attribName == "name")
				m_name = QString::fromStdString(attrib->ValueStr());
			else if (attribName == "color")
				m_color.setNamedColor(QString::fromStdString(attrib->ValueStr()));
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ATTRIBUTE).arg(attribName).arg(element->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			attrib = attrib->Next();
		}

		// reading elements
		const TiXmlElement * c = element->FirstChildElement();
		while (c) {
			const std::string & cName = c->ValueStr();
			if (cName == "basePoint") {
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
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'ZoneTemplate' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'ZoneTemplate' element.").arg(ex2.what()), FUNC_ID);
	}
}


TiXmlElement *Drawing::Insert::writeXML(TiXmlElement *parent) const {
	TiXmlElement * e = new TiXmlElement("Insert");
	parent->LinkEndChild(e);

	if (!m_currentBlockName.isEmpty())
		e->SetAttribute("blockName", m_currentBlockName.toStdString());
	if (!m_parentBlockName.isEmpty())
		e->SetAttribute("parentBlockName", m_parentBlockName.toStdString());
	if (m_angle != 0.0)
		e->SetAttribute("angle", IBK::val2string<double>(m_angle));
	if (m_xScale != 1.0)
		e->SetAttribute("xScale", IBK::val2string<double>(m_xScale));
	if (m_yScale != 1.0)
		e->SetAttribute("yScale", IBK::val2string<double>(m_yScale));
	if (m_zScale != 1.0)
		e->SetAttribute("zScale", IBK::val2string<double>(m_zScale));

	TiXmlElement::appendSingleAttributeElement(e, "insertionPoint", nullptr, std::string(), m_insertionPoint.toString(PRECISION));

	return e;
}


void Drawing::Insert::readXML(const TiXmlElement *element) {
	FUNCID(Drawing::DimStyle::readXMLPrivate);

	try {
		const TiXmlAttribute * attrib = element->FirstAttribute();
		while (attrib) {
			const std::string & attribName = attrib->NameStr();
			if (attribName == "blockName")
				m_currentBlockName = QString::fromStdString(attrib->ValueStr());
			else if (attribName == "parentBlockName")
				m_parentBlockName = QString::fromStdString(attrib->ValueStr());
			else if (attribName == "angle")
				m_angle= NANDRAD::readPODAttributeValue<double>(element, attrib);
			else if (attribName == "xScale")
				m_xScale = NANDRAD::readPODAttributeValue<double>(element, attrib);
			else if (attribName == "yScale")
				m_yScale = NANDRAD::readPODAttributeValue<double>(element, attrib);
			else if (attribName == "zScale")
				m_zScale = NANDRAD::readPODAttributeValue<double>(element, attrib);

			attrib = attrib->Next();
		}

		// reading elements
		const TiXmlElement * c = element->FirstChildElement();
		while (c) {
			const std::string & cName = c->ValueStr();
			if (cName == "insertionPoint") {
				try {
					m_insertionPoint = IBKMK::Vector2D::fromString(c->GetText());
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
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'Drawing::Insert' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'Drawing::Insert' element.").arg(ex2.what()), FUNC_ID);
	}
}


void Drawing::readXML(const TiXmlElement * element) {
	FUNCID(Drawing::readXML);

	try {
		// search for mandatory attributes
		if (!TiXmlAttribute::attributeByName(element, "id")) {
			IBK::IBK_Message( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
								 IBK::FormatString("Missing required 'id' attribute.") ), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			return;
		}

		// reading attributes
		const TiXmlAttribute * attrib = element->FirstAttribute();
		while (attrib) {
			const std::string & attribName = attrib->NameStr();
			if (attribName == "id")
				m_id = NANDRAD::readPODAttributeValue<unsigned int>(element, attrib);
			else if (attribName == "displayName")
				m_displayName = QString::fromStdString(attrib->ValueStr());
			else if (attribName == "visible")
				m_visible = NANDRAD::readPODAttributeValue<bool>(element, attrib);
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ATTRIBUTE).arg(attribName).arg(element->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			attrib = attrib->Next();
		}
		// search for mandatory elements
		// reading elements
		const TiXmlElement * c = element->FirstChildElement();
		while (c) {
			const std::string & cName = c->ValueStr();
			if (cName == "Origin") {
				try {
					m_origin = IBKMK::Vector3D::fromString(c->GetText());
				} catch (IBK::Exception & ex) {
					throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(c->Row())
												 .arg("Invalid vector data."), FUNC_ID);
				}
			}
			else if (cName == "ScalingFactor")
				m_scalingFactor = NANDRAD::readPODElement<double>(c, cName);
			else if (cName == "LineWeightScaling")
				m_lineWeightScaling = NANDRAD::readPODElement<double>(c, cName);
			else if (cName == "Blocks") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "Block")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					Drawing::Block obj;
					obj.readXML(c2);
					m_blocks.push_back(obj);
					c2 = c2->NextSiblingElement();
				}
			}
			else if (cName == "DrawingLayers") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "DrawingLayer")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					DrawingLayer obj;
					obj.readXML(c2);
					m_drawingLayers.push_back(obj);
					c2 = c2->NextSiblingElement();
				}
			}
			else if (cName == "Points") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "Point")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					Point obj;
					obj.readXML(c2);
					m_points.push_back(obj);
					c2 = c2->NextSiblingElement();
				}
			}
			else if (cName == "Lines") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "Line")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					Line obj;
					obj.readXML(c2);
					m_lines.push_back(obj);
					c2 = c2->NextSiblingElement();
				}
			}
			else if (cName == "Polylines") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "PolyLine")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					PolyLine obj;
					obj.readXML(c2);
					m_polylines.push_back(obj);
					c2 = c2->NextSiblingElement();
				}
			}
			else if (cName == "Circles") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "Circle")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					Circle obj;
					obj.readXML(c2);
					m_circles.push_back(obj);
					c2 = c2->NextSiblingElement();
				}
			}
			else if (cName == "Ellipses") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "Ellipse")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					Ellipse obj;
					obj.readXML(c2);
					m_ellipses.push_back(obj);
					c2 = c2->NextSiblingElement();
				}
			}
			else if (cName == "Arcs") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "Arc")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					Arc obj;
					obj.readXML(c2);
					m_arcs.push_back(obj);
					c2 = c2->NextSiblingElement();
				}
			}
			else if (cName == "Solids") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "Solid")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					Solid obj;
					obj.readXML(c2);
					m_solids.push_back(obj);
					c2 = c2->NextSiblingElement();
				}
			}
			else if (cName == "Texts") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "Text")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					Text obj;
					obj.readXML(c2);
					m_texts.push_back(obj);
					c2 = c2->NextSiblingElement();
				}
			}
			else if (cName == "LinearDimensions") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "LinearDimension")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					LinearDimension obj;
					obj.readXML(c2);
					m_linearDimensions.push_back(obj);
					c2 = c2->NextSiblingElement();
				}
			}
			else if (cName == "DimensionStyles") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "DimStyle")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					DimStyle obj;
					obj.readXML(c2);
					m_dimensionStyles.push_back(obj);
					c2 = c2->NextSiblingElement();
				}
			}
			else if (cName == "Inserts") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "Insert")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					Insert obj;
					obj.readXML(c2);
					m_inserts.push_back(obj);
					c2 = c2->NextSiblingElement();
				}
			}
			else if (cName == "ZCounter")
				m_zCounter = NANDRAD::readPODElement<unsigned int>(c, cName);
			else if (cName == "DefaultColor")
				m_defaultColor.setNamedColor(QString::fromStdString(c->GetText()));
			else if (cName == "RotationMatrix")
				m_rotationMatrix.readXML(c);
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			c = c->NextSiblingElement();
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'Drawing' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'Drawing::Text' element.").arg(ex2.what()), FUNC_ID);
	}
}


TiXmlElement * Drawing::writeXML(TiXmlElement * parent) const {
	if (m_id == VICUS::INVALID_ID)  return nullptr;
	TiXmlElement * e = new TiXmlElement("Drawing");
	parent->LinkEndChild(e);

	if (m_id != VICUS::INVALID_ID)
		e->SetAttribute("id", IBK::val2string<unsigned int>(m_id));
	if (!m_displayName.isEmpty())
		e->SetAttribute("displayName", m_displayName.toStdString());
	if (m_visible != Drawing().m_visible)
		e->SetAttribute("visible", IBK::val2string<bool>(m_visible));
	TiXmlElement::appendSingleAttributeElement(e, "Origin", nullptr, std::string(), m_origin.toString(PRECISION));
	m_rotationMatrix.writeXML(e);
	TiXmlElement::appendSingleAttributeElement(e, "ScalingFactor", nullptr, std::string(), IBK::val2string<double>(m_scalingFactor));
	TiXmlElement::appendSingleAttributeElement(e, "LineWeightScaling", nullptr, std::string(), IBK::val2string<double>(m_lineWeightScaling));

	if (!m_blocks.empty()) {
		TiXmlElement * child = new TiXmlElement("Blocks");
		e->LinkEndChild(child);

		for (std::vector<Drawing::Block>::const_iterator it = m_blocks.begin();
			 it != m_blocks.end(); ++it)
		{
			it->writeXML(child);
		}
	}

	if (!m_drawingLayers.empty()) {
		TiXmlElement * child = new TiXmlElement("DrawingLayers");
		e->LinkEndChild(child);

		for (std::vector<DrawingLayer>::const_iterator it = m_drawingLayers.begin();
			 it != m_drawingLayers.end(); ++it)
		{
			it->writeXML(child);
		}
	}

	if (!m_points.empty()) {
		TiXmlElement * child = new TiXmlElement("Points");
		e->LinkEndChild(child);

		for (std::vector<Point>::const_iterator it = m_points.begin();
			 it != m_points.end(); ++it)
		{
			it->writeXML(child);
		}
	}

	if (!m_lines.empty()) {
		TiXmlElement * child = new TiXmlElement("Lines");
		e->LinkEndChild(child);

		for (std::vector<Line>::const_iterator it = m_lines.begin();
			 it != m_lines.end(); ++it)
		{
			it->writeXML(child);
		}
	}

	if (!m_polylines.empty()) {
		TiXmlElement * child = new TiXmlElement("Polylines");
		e->LinkEndChild(child);

		for (std::vector<PolyLine>::const_iterator it = m_polylines.begin();
			 it != m_polylines.end(); ++it)
		{
			it->writeXML(child);
		}
	}

	if (!m_circles.empty()) {
		TiXmlElement * child = new TiXmlElement("Circles");
		e->LinkEndChild(child);

		for (std::vector<Circle>::const_iterator it = m_circles.begin();
			 it != m_circles.end(); ++it)
		{
			it->writeXML(child);
		}
	}

	if (!m_ellipses.empty()) {
		TiXmlElement * child = new TiXmlElement("Ellipses");
		e->LinkEndChild(child);

		for (std::vector<Ellipse>::const_iterator it = m_ellipses.begin();
			 it != m_ellipses.end(); ++it)
		{
			it->writeXML(child);
		}
	}

	if (!m_arcs.empty()) {
		TiXmlElement * child = new TiXmlElement("Arcs");
		e->LinkEndChild(child);

		for (std::vector<Arc>::const_iterator it = m_arcs.begin();
			 it != m_arcs.end(); ++it)
		{
			it->writeXML(child);
		}
	}

	if (!m_solids.empty()) {
		TiXmlElement * child = new TiXmlElement("Solids");
		e->LinkEndChild(child);

		for (std::vector<Solid>::const_iterator it = m_solids.begin();
			 it != m_solids.end(); ++it)
		{
			it->writeXML(child);
		}
	}

	if (!m_texts.empty()) {
		TiXmlElement * child = new TiXmlElement("Texts");
		e->LinkEndChild(child);

		for (std::vector<Text>::const_iterator it = m_texts.begin();
			 it != m_texts.end(); ++it)
		{
			it->writeXML(child);
		}
	}

	if (!m_linearDimensions.empty()) {
		TiXmlElement * child = new TiXmlElement("LinearDimensions");
		e->LinkEndChild(child);

		for (std::vector<LinearDimension>::const_iterator it = m_linearDimensions.begin();
			 it != m_linearDimensions.end(); ++it)
		{
			it->writeXML(child);
		}
	}

	if (!m_dimensionStyles.empty()) {
		TiXmlElement * child = new TiXmlElement("DimensionStyles");
		e->LinkEndChild(child);

		for (std::vector<DimStyle>::const_iterator it = m_dimensionStyles.begin();
			 it != m_dimensionStyles.end(); ++it)
		{
			it->writeXML(child);
		}
	}


	if (!m_inserts.empty()) {
		TiXmlElement * child = new TiXmlElement("Inserts");
		e->LinkEndChild(child);

		for (std::vector<Insert>::const_iterator it = m_inserts.begin();
			 it != m_inserts.end(); ++it)
		{
			it->writeXML(child);
		}
	}

	if (m_zCounter != VICUS::INVALID_ID)
		TiXmlElement::appendSingleAttributeElement(e, "ZCounter", nullptr, std::string(), IBK::val2string<unsigned int>(m_zCounter));
	if (m_defaultColor.isValid())
		TiXmlElement::appendSingleAttributeElement(e, "DefaultColor", nullptr, std::string(), m_defaultColor.name().toStdString());
	return e;
}

} // namespace VICUS
