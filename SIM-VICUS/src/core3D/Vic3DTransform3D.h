/*	SIM-VICUS - Building and District Energy Simulation Tool.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Dirk Weiss  <dirk.weiss -[at]- tu-dresden.de>
	  Stephan Hirth  <stephan.hirth -[at]- tu-dresden.de>
	  Hauke Hirsch  <hauke.hirsch -[at]- tu-dresden.de>

	  ... all the others from the SIM-VICUS team ... :-)

	This program is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

#ifndef Vic3DTransform3DH
#define Vic3DTransform3DH

#include <QVector3D>
#include <QQuaternion>
#include <QMatrix4x4>

namespace Vic3D {


/*! A typical 3D transformation class.
	Implements lazy evaluation - calling any of the modification functions
	will only store info about the modification - only by retrieving the
	matrix with toMatrix(), the actual matrix is being returned.
*/
class Transform3D {
public:
	// Constructors
	Transform3D();

	// Transform By (Add/Scale)
	void translate(const QVector3D &dt);
	void translate(float dx, float dy, float dz);
	void scale(const QVector3D &ds);
	void scale(float dx, float dy, float dz);
	void scale(float factor);
	void rotate(const QQuaternion &dr);
	void rotate(float angle, const QVector3D &axis);
	void rotate(float angle, float ax, float ay, float az);
	void grow(const QVector3D &ds);
	void grow(float dx, float dy, float dz);
	void grow(float factor);

	// Transform To (Setters)
	void setTranslation(const QVector3D &t);
	void setTranslation(float x, float y, float z);
	void setScale(const QVector3D &s);
	void setScale(float x, float y, float z);
	void setScale(float k);
	void setRotation(const QQuaternion &r);
	void setRotation(float angle, const QVector3D &axis);
	void setRotation(float angle, float ax, float ay, float az);

	/*! Sets a transformation matrix that scales a translated and rotated object along its local
		coordinate axes.
		\param offset The translation vector from global coordinate system to local coordinate system.
		\param toLocal The rotation matrix from global coordinate system to local coordinate system.
		\param localScaleFactors The scale factors for all local axes.
	*/
	void setLocalScaling(const QVector3D & offset,
						 const QQuaternion & toLocal, const QVector3D & localScaleFactors);

	// Accessors
	const QVector3D& translation() const;
	const QVector3D& scale() const;
	const QQuaternion& rotation() const;
	const QMatrix4x4& toMatrix() const;

protected:
	QVector3D m_translation;
	QVector3D m_scale;
	QQuaternion m_rotation;
	mutable QMatrix4x4 m_world; // is updated in the const toMatrix() function
	mutable bool m_dirty;
//	char _padding[3]; // additional padding characters to align class to 4 byte boundary (if missing, compiler would do this automatically)

#ifndef QT_NO_DATASTREAM
	friend QDataStream &operator<<(QDataStream &out, const Transform3D &transform);
	friend QDataStream &operator>>(QDataStream &in, Transform3D &transform);
#endif
};

inline Transform3D::Transform3D() : m_scale(1.0f, 1.0f, 1.0f), m_dirty(true) {}

// Transform By (Add/Scale)
inline void Transform3D::translate(float dx, float dy,float dz) { translate(QVector3D(dx, dy, dz)); }
inline void Transform3D::scale(float dx, float dy,float dz) { scale(QVector3D(dx, dy, dz)); }
inline void Transform3D::scale(float factor) { scale(QVector3D(factor, factor, factor)); }
inline void Transform3D::rotate(float angle, const QVector3D &axis) { rotate(QQuaternion::fromAxisAndAngle(axis, angle)); }
inline void Transform3D::rotate(float angle, float ax, float ay,float az) { rotate(QQuaternion::fromAxisAndAngle(ax, ay, az, angle)); }
inline void Transform3D::grow(float dx, float dy, float dz) { grow(QVector3D(dx, dy, dz)); }
inline void Transform3D::grow(float factor) { grow(QVector3D(factor, factor, factor)); }

// Transform To (Setters)
inline void Transform3D::setTranslation(float x, float y, float z) { setTranslation(QVector3D(x, y, z)); }
inline void Transform3D::setScale(float x, float y, float z) { setScale(QVector3D(x, y, z)); }
inline void Transform3D::setScale(float k) { setScale(QVector3D(k, k, k)); }
inline void Transform3D::setRotation(float angle, const QVector3D &axis) { setRotation(QQuaternion::fromAxisAndAngle(axis, angle)); }
inline void Transform3D::setRotation(float angle, float ax, float ay, float az) { setRotation(QQuaternion::fromAxisAndAngle(ax, ay, az, angle)); }

// Accessors
inline const QVector3D& Transform3D::translation() const { return m_translation; }
inline const QVector3D& Transform3D::scale() const { return m_scale; }
inline const QQuaternion& Transform3D::rotation() const { return m_rotation; }

// Qt Streams
#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug dbg, const Transform3D &transform);
#endif

#ifndef QT_NO_DATASTREAM
QDataStream &operator<<(QDataStream &out, const Transform3D &transform);
QDataStream &operator>>(QDataStream &in, Transform3D &transform);
#endif

} // namespace Vic3D

Q_DECLARE_TYPEINFO(Vic3D::Transform3D, Q_MOVABLE_TYPE);

#endif // Vic3DTransform3DH
