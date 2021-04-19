#ifndef SH_SunShadingAlgorithmH
#define SH_SunShadingAlgorithmH

#include "SH_Polygon.h"

namespace SH {


class SunShadingAlgorithm
{
public:

	struct ShadingObj{

		Polygon					m_grid;						///< Grid of shading object
		bool					m_isGridInitialized = false;///< Indicates whether grid is initialized
		Polygon					m_polygon;					///< Polyline
		double					m_shadingValue;				///< shading value 1 is no shading; 0 surface is completly shaded
	};

	//member functions start
	/*! Calculate the shading value of all polygons
		\param sunNormal normal of the sun beam
		\param gridElementLength Length of the grid in x and y direction in m, standard 0.5 m
	*/
	void calcShading(const IBKMK::Vector3D & sunNormal);

	/*! Initialize Grid for Shading Object.
		1) Rotate polygon to x-y-plane
		2) Create Grid in this plane
		3) Rotate all Grid Elements back to original shading object plane
	*/
	void initializeGrid();

	//member functions end


	//Member variables Start
	std::vector<ShadingObj>					m_shadingObjects;		///< all polygons to be considered for the shading calculation
	std::vector<Polygon>					m_obstacles;			///< all obstacle polygons
	double									m_gridLength = 0.1;		///< Grid Element Length in [m]


private:

	/*! Calculate the shading value of an polygon.
		\param sunNormal normal of the sun beam
	*/
	void calcShadingOneElement( const IBKMK::Vector3D & sunNormal);

};
}


#endif // SH_SunShadingAlgorithmH
