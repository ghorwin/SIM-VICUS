#include "VICUS_Polygon2D.h"

#include <set>

#include <QLineF>

#include <IBK_Line.h>
#include <IBK_math.h>

#include <IBKMK_Triangulation.h>

#include <VICUS_Constants.h>
#include <VICUS_KeywordList.h>

#include <tinyxml.h>

namespace VICUS {


void Polygon2D::addVertex(const QPointF &v) {
	m_vertexes << v;
	computeGeometry(); // if we have a triangle/rectangle, this is detected here
}

void Polygon2D::removeVertex(unsigned int idx){
	Q_ASSERT(idx < (unsigned int)m_vertexes.size());
	m_vertexes.erase(m_vertexes.begin()+idx);
	m_type = T_Polygon; // assume the worst
	computeGeometry(); // if we have a triangle/rectangle, this is detected here
}

void Polygon2D::computeGeometry(){
	m_triangles.clear();
	eleminateColinearPts();

	// try to simplify polygon to internal rectangle/parallelogram definition
	// this may change m_type to Rectangle or Triangle and subsequently speed up operations
	simplify();
	// we need 3 vertexes (not collinear) to continue
	if (m_vertexes.size() < 3)
		return;
	// determine 2D plane coordinates

	// polygon must not be winding into itself, otherwise triangulation would not be meaningful
	if (isSimplePolygon())
		triangulate();
}

void Polygon2D::flip(){
	std::vector<QPointF> inverseVertexes;

	for(int i=m_vertexes.size()-1; i>=0; --i )
		inverseVertexes.push_back(m_vertexes[i]);

	///TODO Dirk->Andreas das untere funktioniert nicht mehr brauch da hilfe.
//	for (std::vector<QPointF>::const_reverse_iterator rit = m_vertexes.rbegin();
//		 rit != m_vertexes.rend(); ++rit) {
//		inverseVertexes.push_back(*rit);
//	}


//		 setVertexes(inverseVertexes);
}


bool Polygon2D::intersectsLine(const QPointF &p1, const QPointF &p2, QPointF &intersectionPoint) const{
	QLineF otherLine(p1,p2);
	int vertSize=m_vertexes.size();
	for(int i=0; i<vertSize; ++i){
		QLineF line(m_vertexes[i], m_vertexes[(i+1)%vertSize]);
		QPointF *intP = nullptr;
		QLineF::IntersectType type = line.intersect(otherLine, intP);

		if(intP==nullptr || type == QLineF::UnboundedIntersection)
			//no intersection go to next
			continue;

		//no we have an intersection
		intersectionPoint = *intP;
		//stop here
		return true;
	}
	return false;

}

double Polygon2D::area() const{
	double surfArea=0;
	int sizeV = m_vertexes.size();
	for(int i=0; i<sizeV; ++i){
		const QPointF &p0 = m_vertexes[i];
		const QPointF &p1 = m_vertexes[(i+1)%sizeV];
		const QPointF &p2 = m_vertexes[(i+2)%sizeV];
		surfArea += p1.x() * (p2.y() - p0.y());
	}
	surfArea *= 0.5;
	return surfArea;
}

void Polygon2D::simplify() {
	if (m_vertexes.size() == 3) {
		m_type = T_Triangle;
		return;
	}
	if (m_vertexes.size() != 4)
		return;
	const QPointF & a = m_vertexes[0];
	const QPointF & b = m_vertexes[1];
	const QPointF & c = m_vertexes[2];
	const QPointF & d = m_vertexes[3];
	QPointF c2 = b + (d-a);
	c2 -= c;
	// we assume we have zero length for an rectangle
	if (c2.manhattanLength() < 1e-4)
		m_type = T_Rectangle;
}


void Polygon2D::triangulate() {
//	FUNCID(PlaneGeometry::triangulate);
	Q_ASSERT(m_vertexes.size() >= 3);
	//Q_ASSERT(m_polygon.size() == m_vertexes.size());

	bool isDrawMode = true;

	const double eps = 1e-4;
	m_triangles.clear();
	switch (m_type) {

		case T_Triangle :
			m_triangles.push_back( triangle_t(0, 1, 2) );
			break;

		case T_Rectangle :
			m_triangles.push_back( triangle_t(0, 1, 2) );
			m_triangles.push_back( triangle_t(2, 3, 1) );
		break;

		case T_Polygon : {
			//here the index is stored which is already taken into account
			std::set<unsigned int> usedIdx;
			std::vector<std::vector<unsigned int>>	trisIndices;

			IBKMK::Triangulation triangu;

			std::vector<IBK::point2D<double> > points;
			std::vector<std::pair<unsigned int, unsigned int> > edges;

			// fill points vector
			int sizeVert = m_vertexes.size();
			points.resize((size_t)m_vertexes.size());
			for (int i=0; i<sizeVert; ++i) {
				const QPointF  & p = m_vertexes[i];
				points[i] = IBK::point2D<double>(p.x(), p.y());
				edges.push_back(std::make_pair(i, (i+1)%sizeVert));
			}
			edges.back().second = 0;

			triangu.setPoints(points, edges);

			for (auto tri : triangu.m_triangles) {
				// TODO : only add triangles if not degenerated (i.e. area = 0) (may happen in case of windows aligned to outer bounds)
				m_triangles.push_back(triangle_t(tri.i1, tri.i2, tri.i3));
			}

		}
		break;

		case NUM_T : ; // shouldn't happen
		break;
	}
}

bool Polygon2D::isSimplePolygon()
{
	std::vector<IBK::Line>	lines;
	for (int i=0; i<m_vertexes.size(); ++i) {
		lines.emplace_back(
					IBK::Line(
					IBK::point2D<double>(
								  m_vertexes.value(i).x(),
								  m_vertexes.value(i).y()),
					IBK::point2D<double>(
								  m_vertexes.value((i+1)%m_vertexes.size()).x(),
								  m_vertexes.value((i+1)%m_vertexes.size()).y())));
	}
	if(lines.size()<4)
		return true;
	for (unsigned int i=0; i<lines.size();++i) {
		for (unsigned int j=0; j<lines.size()-2; ++j) {
			unsigned int k1 = (i+1)%lines.size();
			unsigned int k2 = (i-1);
			if(i==0)
				k2 = lines.size()-1;
			if(i==j || k1 == j || k2 == j )
				continue;
			//int k = (i+j+2)%lines.size();
			IBK::point2D<double> p;
			if(lines[i].intersects(lines[j], p)){
				return false;
			}
		}
	}

	return true;
}

void Polygon2D::eleminateColinearPts(){

	int sizeVert = m_vertexes.size();
	std::vector<int> erasePointIdx;
	for(unsigned int i=0; i<sizeVert; ++i){
		const QPointF &p0 = m_vertexes[i];
		const QPointF &p1 = m_vertexes[(i+1)%sizeVert];
		const QPointF &p2 = m_vertexes[(i+2)%sizeVert];

		if(p1==p2){
			erasePointIdx.push_back((i+1)%sizeVert);
			continue;
		}

		double dx1 = p1.x() - p0.x();
		double dx2 = p2.x() - p0.x();
		double dy1 = p1.y() - p0.y();
		double dy2 = p2.y() - p0.y();

		if( IBK::near_zero(dx1) && IBK::near_zero(dx2)){
			//eleminate point
			erasePointIdx.push_back((i+1)%sizeVert);
			continue;
		}
		if(IBK::near_zero(dx1) || IBK::near_zero(dx2))
			continue;
		if( IBK::near_zero(std::abs(dy1/dx1) - std::abs(dy2/dx2))){
			//eleminate point
			erasePointIdx.push_back((i+1)%sizeVert);
			continue;
		}
	}

	unsigned int count=0;
	while (count<erasePointIdx.size()) {
		m_vertexes.erase(m_vertexes.begin()+erasePointIdx[count]);
		++count;
	}
}


void Polygon2D::readXMLPrivate(const TiXmlElement * element) {
	FUNCID(Polygon2D::readXMLPrivate);

	try {
		// search for mandatory attributes
		// reading attributes
		const TiXmlAttribute * attrib = element->FirstAttribute();
		while (attrib) {
			const std::string & attribName = attrib->NameStr();
			if (attribName == "type")
				try {
				m_type = (type_t)KeywordList::Enumeration("Polygon2D::type_t", attrib->ValueStr());
			}
			catch (IBK::Exception & ex) {
				throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
										  IBK::FormatString("Invalid or unknown keyword '"+attrib->ValueStr()+"'.") ), FUNC_ID);
			}
			attrib = attrib->Next();
		}

		// read data


		std::string text = element->GetText();
		text = IBK::replace_string(text, ",", " ");
		std::vector<double> vals;
		try {
			IBK::string2valueVector(text, vals);
			// must have n*3 elements
			if (vals.size() % 2 != 0)
				throw IBK::Exception("Mismatching number of values.", FUNC_ID);
			if (vals.empty())
				throw IBK::Exception("Missing values.", FUNC_ID);
			m_vertexes.resize(vals.size() / 2);
			for (unsigned int i=0; i<m_vertexes.size(); ++i){
				m_vertexes[i].setX(vals[i*2]);
				m_vertexes[i].setY(vals[i*2+1]);
			}

		} catch (IBK::Exception & ex) {
			throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Error reading vector element '%1'.").arg("Polygon2D") ), FUNC_ID);
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'Polygon2D' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'Polygon2D' element.").arg(ex2.what()), FUNC_ID);
	}
}


TiXmlElement * Polygon2D::writeXMLPrivate(TiXmlElement * parent) const {
	TiXmlElement * e = new TiXmlElement("Polygon2D");
	parent->LinkEndChild(e);

	if (m_type != NUM_T)
		e->SetAttribute("type", KeywordList::Keyword("Polygon2D::type_t",  m_type));
	std::stringstream vals;
	for (unsigned int i=0; i<m_vertexes.size(); ++i) {
		vals << m_vertexes[i].x() << " " << m_vertexes[i].y();
		if (i<m_vertexes.size()-1)  vals << ", ";
	}
	TiXmlText * text = new TiXmlText( vals.str() );
	e->LinkEndChild( text );
	return e;
}

}
