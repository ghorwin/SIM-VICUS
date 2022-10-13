#ifndef RCProjectH
#define RCProjectH

#include <VICUS_Project.h>

#include <clipper.hpp>

#include "RC_ClippingSurface.h"

namespace RC {

const unsigned int SCALE_FACTOR = 1E8;
const double MIN_AREA = 1e-4;


class Project
{
public:

	Project(){}

	Project(const VICUS::Project &prj, double normalDeviationInDeg, double maxDistanceOfSurfaces, unsigned int lastUnusedID):
		m_prjVicus(prj),
		m_normalDeviationInDeg(normalDeviationInDeg),
		m_maxDistanceOfSurfaces(maxDistanceOfSurfaces),
		m_lastUnusedVicusID(lastUnusedID)
	{
		m_newPrjVicus = m_prjVicus;
		m_newPrjVicus.updatePointers();
	}

	struct extPolygon {

		extPolygon() {}

		extPolygon(const IBKMK::Polygon2D &polygon):
			m_polygon(polygon)
		{}

		extPolygon(const IBKMK::Polygon2D &polygon, const std::vector<IBKMK::Polygon2D> &holePolygons):
			m_polygon(polygon),
			m_holePolygons(holePolygons)
		{}

		IBKMK::Polygon2D					m_polygon;
		std::vector<IBKMK::Polygon2D>		m_holePolygons;
		bool								m_haveRealHole = true;
	};


	/*! Finds all corresponding parallel surfaces for clipping operations. */
	void findParallelSurfaces();

	/*! Finds all corresponding surfaces in range for clipping. */
	void findSurfacesInRange();

	/*! Surfaces are clipped by their corresponding surfaces sorted by distance. */
	void clipSurfaces();

	const VICUS::Project &newPrjVicus() const;

	void generatePolyWithHole(const IBKMK::Polygon2D &polygon, const std::vector<IBKMK::Polygon2D> &holes,
							  IBKMK::Polygon2D &newPolygon, bool &realHole);

	void testProjectClipping();
private:

	ClippingSurface & getClippingSurfaceById(unsigned int id);

	void doClipperClipping(const extPolygon &surf,
						   const extPolygon &otherSurf,
						   std::vector<extPolygon> &mainDiffs,
						   std::vector<extPolygon> &mainIntersections,
						   bool normalInterpolation = false);
	/*! Create a clipper lib path from a IBKMK polygon. */
	ClipperLib::Path convertVec2DToClipperPath(const std::vector<IBKMK::Vector2D> &vertexes);

	bool isSamePolygon(const ClipperLib::Path &diff, const ClipperLib::Path &intersection);

	bool isIntersectionAnHole(const ClipperLib::Path &pathIntersection, const ClipperLib::PolyNodes &diffs);

	std::vector<IBKMK::Vector2D> convertClipperPathToVec2D(const ClipperLib::Path &path);

	const VICUS::Project							m_prjVicus;					///< copy of VICUS project

	VICUS::Project									m_newPrjVicus;				///< VICUS project with added surfaces

	double											m_normalDeviationInDeg;		///< normal deviation in DEG

	double											m_maxDistanceOfSurfaces;	///< maximal distance of the search radius for clipping in m

	/*! holds all parallel surfaces by id; second element in pair is the distance in m. */
	std::vector<ClippingSurface>					m_clippingSurfaces;

	/*! Clipping connections */
	std::map<unsigned int, std::set<unsigned int>>	m_connections;

	unsigned int									m_lastUnusedVicusID;		///< last unused id in vicus project

};
}

#endif // RCProjectH
