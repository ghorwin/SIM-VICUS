/*	The RoomClipper data model library.

	Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Stephan Hirth     <stephan.hirth -[at]- tu-dresden.de>
	  Dirk Weiß         <dirk.weis     -[at]- tu-dresden.de>

	This library is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This library is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

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
