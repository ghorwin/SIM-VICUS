#include "VICUS_Drawing.h"
#include "IBK_MessageHandler.h"
#include "IBK_messages.h"
#include "NANDRAD_Utilities.h"
#include "tinyxml.h"



namespace VICUS {

Drawing::Drawing() :
	m_blocks(std::vector<Block>()),
	m_layers(std::vector<DrawingLayer>()),
	m_points(std::vector<Point>()),
	m_lines(std::vector<Line>()),
	m_polylines(std::vector<PolyLine>()),
	m_circles(std::vector<Circle>()),
	m_ellipses(std::vector<Ellipse>()),
	m_arcs(std::vector<Arc>()),
	m_solids(std::vector<Solid>()),
	m_texts(std::vector<Text>())
{}

TiXmlElement * Drawing::Arc::writeXML(TiXmlElement * parent) const {
	if (m_id == VICUS::INVALID_ID)  return nullptr;

	TiXmlElement * e = new TiXmlElement("Drawing");
	parent->LinkEndChild(e);

	if (m_id != VICUS::INVALID_ID)
		e->SetAttribute("id", IBK::val2string<unsigned int>(m_id));
	if (m_color.isValid())
		e->SetAttribute("color", m_color.name().toStdString());

	TiXmlElement::appendSingleAttributeElement(e, "Center", nullptr, std::string(), m_center.toString());
	TiXmlElement::appendSingleAttributeElement(e, "Radius", nullptr, std::string(), IBK::val2string<double>(m_radius));
	TiXmlElement::appendSingleAttributeElement(e, "StartAngle", nullptr, std::string(), IBK::val2string<double>(m_startAngle));
	TiXmlElement::appendSingleAttributeElement(e, "EndAngle", nullptr, std::string(), IBK::val2string<double>(m_endAngle));

	return e;
}

void Drawing::Arc::readXMLPrivate(const TiXmlElement *element){
	FUNCID(Drawing::Arc::readXMLPrivate);

	try {
		// search for mandatory attributes
		if (!TiXmlAttribute::attributeByName(element, "id"))
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
									  IBK::FormatString("Missing required 'id' attribute.") ), FUNC_ID);

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
					m_center = IBKMK::Vector3D::fromString(c->GetText());
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

const Drawing::AbstractDrawingObject *Drawing::objectByID(unsigned int id) const {
	FUNCID(Drawing::objectByID);

	AbstractDrawingObject *obj = m_objectPtr.at(id);
	if (obj == nullptr)
		throw IBK::Exception(IBK::FormatString("Drawing Object with ID #%1 not found").arg(id), FUNC_ID);

	return obj;
}


void Drawing::updatePointer(){
	m_objectPtr.clear();

	for(unsigned int i=0; i < m_points.size(); i++){
		m_points[i].m_parentLayer = findLayerPointer(m_points[i].m_layerName);
		m_objectPtr[m_points[i].m_id] = &m_points[i];
	}
	for(unsigned int i=0; i < m_lines.size(); i++){
		m_lines[i].m_parentLayer = findLayerPointer(m_lines[i].m_layerName);
		m_objectPtr[m_lines[i].m_id] = &m_lines[i];
	}
	for(unsigned int i=0; i < m_polylines.size(); i++){
		m_polylines[i].m_parentLayer = findLayerPointer(m_polylines[i].m_layerName);
		m_objectPtr[m_polylines[i].m_id] = &m_polylines[i];
	}
	for(unsigned int i=0; i < m_circles.size(); i++){
		m_circles[i].m_parentLayer = findLayerPointer(m_circles[i].m_layerName);
		m_objectPtr[m_circles[i].m_id] = &m_circles[i];
	}
	for(unsigned int i=0; i < m_arcs.size(); i++){
		m_arcs[i].m_parentLayer = findLayerPointer(m_arcs[i].m_layerName);
		m_objectPtr[m_arcs[i].m_id] = &m_arcs[i];
	}
	for(unsigned int i=0; i < m_ellipses.size(); i++){
		m_ellipses[i].m_parentLayer = findLayerPointer(m_ellipses[i].m_layerName);
		m_objectPtr[m_ellipses[i].m_id] = &m_ellipses[i];
	}
	for(unsigned int i=0; i < m_solids.size(); i++){
		m_solids[i].m_parentLayer = findLayerPointer(m_solids[i].m_layerName);
		m_objectPtr[m_solids[i].m_id] = &m_solids[i];
	}
	for(unsigned int i=0; i < m_texts.size(); i++){
		m_texts[i].m_parentLayer = findLayerPointer(m_texts[i].m_layerName);
		m_objectPtr[m_texts[i].m_id] = &m_solids[i];
	}
}


Drawing::DrawingLayer* Drawing::findLayerPointer(const QString &layername){
	for(unsigned int i = 0; i < m_layers.size(); i++) {
		if (m_layers[i].m_displayName == layername)
			return &m_layers[i];
	}
	return nullptr;
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

void Drawing::readXML(const TiXmlElement * element) {
	FUNCID(Drawing::readXML);
	// read 3D geometry

}


TiXmlElement * Drawing::writeXML(TiXmlElement * parent) const {
	TiXmlElement * e = writeXMLPrivate(parent);
	// now add Polygon3D
}

TiXmlElement *Drawing::PolyLine::writeXML(TiXmlElement *element) const
{

}


} // namespace VICUS
