#ifndef RCClippingSurfaceH
#define RCClippingSurfaceH


#include <VICUS_Surface.h>

namespace RC {

/*! Object that encapsulates all necessairy data for clipping operations. */
struct ClippingObject {

    ClippingObject() {}

    ClippingObject(unsigned int id, const VICUS::Surface &surf, double distance) :
        m_vicusId(id),
        m_vicusSurface(surf),
        m_distance(distance)
    {}

    bool operator<(const ClippingObject &other) const {
        return m_distance < other.m_distance;
    }

    unsigned int					m_vicusId;					///< id of clipping surface
    VICUS::Surface					m_vicusSurface;				///< Pointer to vicus surfaces
    double							m_distance;					///< distance to clipping object

};

class ClippingSurface
{
public:
    ClippingSurface(){}

    ClippingSurface(unsigned int id, const VICUS::Surface &surf):
        m_vicusId(id),
        m_vicusSurface(surf)
    {}

    unsigned int					m_vicusId;					///< id of vicus surface

    VICUS::Surface					m_vicusSurface;				///< Pointer to vicus surface

    std::vector<ClippingObject>		m_clippingObjects;			///< set with clipping objects

    std::vector<VICUS::Surface>		m_intersectedSurfaces;		///< Vector with all new intersected surfaces from clipping
    std::vector<IBKMK::Polygon3D>	m_remainingSurfaces;		///< Vector with all remaining surfaces from clipping as Polygon3D

};

}
#endif // RCClippingSurfaceH
