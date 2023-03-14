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

#ifndef RCProjectH
#define RCProjectH

#include <VICUS_Project.h>

#include <clipper.hpp>

#include <IBK_NotificationHandler.h>

#include "RC_ClippingSurface.h"
#include "RC_ClippingPolygon.h"
#include "RC_Constants.h"

namespace RC {

class Notification : public IBK::NotificationHandler {
public:
    bool	m_aborted = false;
};

/*! Class for VICUS Clipping. Contains all necessairy data and functionality needed to perform smart clipping. */
class VicusClipper {
public:

    VicusClipper(const std::vector<VICUS::Building> &buildings, const std::vector<VICUS::ComponentInstance> &cis,
				 double normalDeviationInDeg, double maxDistanceOfSurfaces, unsigned int lastUnusedID, bool onlySelected = false):
        m_vicusBuildings(buildings),
        m_vicusCompInstances(cis),
		m_normalDeviationInDeg(normalDeviationInDeg),
		m_maxDistanceOfSurfaces(maxDistanceOfSurfaces),
		m_onlySelected(onlySelected),
		m_nextVicusId(lastUnusedID)
	{
        for(VICUS::Building &b : m_vicusBuildings)
            b.updateParents();

        m_vicusBuildingsClipped = m_vicusBuildings;
    }

    /*! Add Polygons to Main Diffs or Intersections. */
    void addClipperPolygons(const std::vector<ClippingPolygon> &polysTemp, std::vector<ClippingPolygon> &mainDiffsTemp);

	/*! Finds all corresponding parallel surfaces for clipping operations. */
	void findParallelSurfaces(Notification * notify);

	/*! Finds all corresponding surfaces in range for clipping. */
    void findSurfacesInRange(Notification * notify);

	/*! Surfaces are clipped by their corresponding surfaces sorted by distance. */
    void clipSurfaces(Notification *notify);

    /*! Component Instances are beeing created by cutting all produced surfaces by clipper lib. */
	void createComponentInstances(Notification * notify, bool createConnections = true);

    /*! Finds the corresponding component instance of specified surface by id. */
	unsigned int findComponentInstanceForSurface(unsigned int id);

	/*! Generates a unique name for every Surface
		Surf01 is cut into 3 pieces --> name will be Surf01 [1] Surf01 [2] Surf01 [3] */
	QString generateUniqueName(QString name);

    const std::vector<VICUS::Building> vicusBuildings() const;

    const std::vector<VICUS::ComponentInstance> *vicusCompInstances() const;

    const std::vector<VICUS::Building> vicusBuildingsClipped() const;

private:

    /*! Returns the containing Clipping Surface with VICUS Surface from m_clippingSurfaces. */
    ClippingSurface & findClippingSurface(unsigned int id, const std::vector<VICUS::Building> &buildings);

    const VICUS::Surface &findVicusSurface(unsigned int id, const std::vector<VICUS::Building> &buildings);

    /*! Performs the Clipping of the surfaces 'surf' and 'otherSurf' and returns intersection and difference polygons. */
    void doClipperClipping(const ClippingPolygon &surf,
                           const ClippingPolygon &otherSurf,
                           std::vector<ClippingPolygon> &mainDiffs,
                           std::vector<ClippingPolygon> &mainIntersections,
						   bool normalInterpolation = false);


	/*! Create a clipper lib path from a IBKMK polygon. */
	ClipperLib::Path convertVec2DToClipperPath(const std::vector<IBKMK::Vector2D> &vertexes);

    /*! Check whether the clipper polygon is the same. */
	bool isSamePolygon(const ClipperLib::Path &diff, const ClipperLib::Path &intersection);

    /*! Check whether an intersection is an hole. */
	bool isIntersectionAnHole(const ClipperLib::Path &pathIntersection, const ClipperLib::PolyNodes &diffs);

    /*! Convert Clipper path ti Verctor 2D. */
	std::vector<IBKMK::Vector2D> convertClipperPathToVec2D(const ClipperLib::Path &path);

    /*! Add Surfaces to Clipping Polygons. */
    void addSurfaceToClippingPolygons(const VICUS::Surface &surf, std::vector<ClippingPolygon> &clippingPolygons);

    // ***** PUBLIC MEMBER VARIABLES *****


    std::vector<VICUS::Building>                    m_vicusBuildings;           ///< Original VICUS buildings
    std::vector<VICUS::Building>                    m_vicusBuildingsClipped;    ///< VICUS buildings with newly added data


    std::vector<VICUS::ComponentInstance>           m_vicusCompInstances;       ///< VICUS component instances

	double											m_normalDeviationInDeg;		///< normal deviation in DEG

	double											m_maxDistanceOfSurfaces;	///< maximal distance of the search radius for clipping in m

	/*! holds all parallel surfaces by id; second element in pair is the distance in m. */
	std::vector<ClippingSurface>					m_clippingSurfaces;

	/*! Take only selected polygons. */
	bool											m_onlySelected = false;

	/*! Name map of surfaces. ID is root-name of surface stripper of any [x] and value is its count. */
	std::map<QString, unsigned int>					m_nameMap;

	/*! Clipping connections
		key is vicus surface id
		values ids of all other possible vicus surfaces for clipping
	*/
    std::map<unsigned int, std::set<unsigned int>>	m_surfaceConnections;

    unsigned int									m_nextVicusId;              ///< last unused id in vicus project

    std::map<unsigned int, unsigned int>			m_compInstOriginSurfId;		///< key is new created surface id, value is old surface ci id

};
}

#endif // RCProjectH
