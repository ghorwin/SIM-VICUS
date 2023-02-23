#ifndef RC_ClippingPolygonH
#define RC_ClippingPolygonH

#include <IBKMK_Polygon2D.h>

class ClippingPolygon {
public:
    ClippingPolygon() {}

    ClippingPolygon(const IBKMK::Polygon2D &polygon):
        m_polygon(polygon)
	{
		if(polygon.isValid())
			m_area = polygon.area();
	}

    ClippingPolygon(const IBKMK::Polygon2D &polygon, const std::vector<IBKMK::Polygon2D> &holePolygons):
        m_polygon(polygon),
        m_holePolygons(holePolygons)
	{
		if(polygon.isValid())
			m_area = polygon.area();
	}

    bool operator==(const ClippingPolygon &other) const {
        if(m_polygon != other.m_polygon)
            return false;
        if(m_holePolygons.size() != other.m_holePolygons.size())
            return false;
        for(unsigned int i=0; i<m_holePolygons.size(); ++i) {
            if(m_holePolygons[i] != other.m_holePolygons[i]) {
                return false;
            }
        }
        if(m_haveRealHole != other.m_haveRealHole)
            return false;

        return true;
    }

    IBKMK::Polygon2D					m_polygon;
	double								m_area;
    std::vector<IBKMK::Polygon2D>		m_holePolygons;
    bool								m_haveRealHole = true;
};

#endif // RC_ClippingPolygonH
