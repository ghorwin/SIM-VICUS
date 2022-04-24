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

#include "Vic3DTransform3D.h"
#include <QDebug>

namespace Vic3D {

// Transform By (Add/Scale)
void Transform3D::translate(const QVector3D &dt) {
	m_dirty = true;
	m_translation += dt;
}

void Transform3D::scale(const QVector3D &ds) {
	m_dirty = true;
	m_scale *= ds;
}

void Transform3D::rotate(const QQuaternion &dr) {
	m_dirty = true;
	m_rotation = dr * m_rotation;
}

void Transform3D::grow(const QVector3D &ds) {
	m_dirty = true;
	m_scale += ds;
}

// Transform To (Setters)
void Transform3D::setTranslation(const QVector3D &t) {
	m_dirty = true;
	m_translation = t;
}

void Transform3D::setScale(const QVector3D &s) {
	m_dirty = true;
	m_scale = s;
}

void Transform3D::setRotation(const QQuaternion &r) {
	m_dirty = true;
	m_rotation = r;
}

void Transform3D::setLocalScaling(const QVector3D & offset, const QQuaternion & toLocal, const QVector3D & localScaleFactors) {
	m_world.setToIdentity();
	m_world.translate(offset);
	m_world.rotate(toLocal);
	m_world.scale(localScaleFactors);
	m_world.rotate(toLocal.inverted());
	m_world.translate(-offset);
	m_dirty = false;
}

// Accessors
const QMatrix4x4 &Transform3D::toMatrix() const {
	if (m_dirty) {
		m_dirty = false;
		m_world.setToIdentity();
		m_world.translate(m_translation);
		m_world.rotate(m_rotation);
		m_world.scale(m_scale);
	}
	return m_world;
}

// Qt Streams
QDebug operator<<(QDebug dbg, const Transform3D &transform) {
	dbg << "Transform3D\n{\n";
	dbg << "Position: <" << transform.translation().x() << ", " << transform.translation().y() << ", " << transform.translation().z() << ">\n";
	dbg << "Scale: <" << transform.scale().x() << ", " << transform.scale().y() << ", " << transform.scale().z() << ">\n";
	dbg << "Rotation: <" << transform.rotation().x() << ", " << transform.rotation().y() << ", " << transform.rotation().z() << " | " << transform.rotation().scalar() << ">\n}";
	return dbg;
}

QDataStream &operator<<(QDataStream &out, const Transform3D &transform) {
	out << transform.m_translation;
	out << transform.m_scale;
	out << transform.m_rotation;
	return out;
}

QDataStream &operator>>(QDataStream &in, Transform3D &transform) {
	in >> transform.m_translation;
	in >> transform.m_scale;
	in >> transform.m_rotation;
	transform.m_dirty = true;
	return in;
}

} // namespace Vic3D
