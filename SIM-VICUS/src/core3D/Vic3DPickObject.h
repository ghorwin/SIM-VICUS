#ifndef PICKOBJECT_H
#define PICKOBJECT_H

#include <QVector3D>

namespace Vic3D {

/*! An Object to hold information on the clicked-on object. */
struct PickObject {
	PickObject(float dist, unsigned int id) :
		m_dist(dist), m_objectId(id), m_faceId(0)
	{
	}
	PickObject(float dist, unsigned int id, unsigned int faceId) :
		m_dist(dist), m_objectId(id), m_faceId(faceId)
	{
	}

	float m_dist; // the normalized distance of the intersection point from starting point of pick line
	unsigned int m_objectId; // the object clicked on
	unsigned int m_faceId; // the actual triangle/plane clicked on
};


} // namespace Vic3D

#endif // PICKOBJECT_H
