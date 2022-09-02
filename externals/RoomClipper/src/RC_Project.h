#ifndef RCProjectH
#define RCProjectH

#include <VICUS_Project.h>

#include "RC_ClippingSurface.h"

namespace RC {


const unsigned int SCALE_FACTOR = 1E8;



class Project
{
public:

	Project(){}

	Project(const VICUS::Project &prj, double normalDeviationInDeg, double maxDistanceOfSurfaces):
		m_prjVicus(prj),
		m_normalDeviationInDeg(normalDeviationInDeg),
		m_maxDistanceOfSurfaces(maxDistanceOfSurfaces)
	{
		m_newPrjVicus = m_prjVicus;
		m_newPrjVicus.updatePointers();
	}


	/*! Finds all corresponding parallel surfaces for clipping operations. */
	void findParallelSurfaces();

	/*! Finds all corresponding surfaces in range for clipping. */
	void findSurfacesInRange();

	/*! Surfaces are clipped by their corresponding surfaces sorted by distance. */
	void clipSurfaces();




	const VICUS::Project &newPrjVicus() const;

private:

	ClippingSurface & getClippingSurfaceById(unsigned int id);

	void doClipperClipping(const IBKMK::Polygon2D &surf,
						   const IBKMK::Polygon2D &otherSurf,
						   std::vector<IBKMK::Polygon2D> &mainDiffs,
						   std::vector<IBKMK::Polygon2D> &mainIntersections,
						   IBKMK::Polygon2D &hole, bool normalInterpolation = false);

	const VICUS::Project							m_prjVicus;					///< copy of VICUS project

	VICUS::Project									m_newPrjVicus;				///< VICUS project with added surfaces

	double											m_normalDeviationInDeg;		///< normal deviation in DEG

	double											m_maxDistanceOfSurfaces;	///< maximal distance of the search radius for clipping in m

	/*! holds all parallel surfaces by id; second element in pair is the distance in m. */
	std::vector<ClippingSurface>					m_clippingSurfaces;

	/*! Clipping connections */
	std::map<unsigned int, std::set<unsigned int>>	m_connections;

};
}

#endif // RCProjectH
