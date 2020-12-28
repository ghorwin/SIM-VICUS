#ifndef PickObjectH
#define PickObjectH

#include <IBKMK_Vector3D.h>
#include <IBK_bitfield.h>

#include <QPoint>

namespace Vic3D {

/*! An object to hold information on what to pick and also store the result.
	Basically, whenever you ask the scene to pick a point (after a mouse click, or if the mouse
	cursor has moved), configure such a pick object and call Scene::pick(obj).
	Afterwards you can access the data stored in the object, most importantly m_pickPoint
	and depending on the object selection choice also the m_uniqueObjectID.
*/
struct PickObject {
	enum PickOption {
		P_XY_Plane			= 0x0001,
		P_Surface			= 0x0002,
		/*! Check network objects (node spheres and edge cylinders). */
		P_Network			= 0x0004
	};

	PickObject(const QPoint & localMousePos, unsigned int pickMask) :
		m_localMousePos(localMousePos), m_pickMask(pickMask)
	{
	}

	/*! The local mouse position. */
	QPoint			m_localMousePos;

	/*! The bitmask, signalling which stuff to pick on the scene. */
	unsigned int	m_pickMask = P_XY_Plane;

	/*! Stores the normalized distance of the intersection point from starting point of pick line. */
	double			m_dist = 2.0; // save distance, 0..1 is valid range

	/*! Stores the actual point in 3D space. */
	IBKMK::Vector3D	m_pickPoint;

	/*! Stores the unique ID of the clicked-on object (0 = not picked). */
	unsigned int	m_uniqueObjectID = 0;
};


} // namespace Vic3D

#endif // PickObjectH
