/*	QtExt - Qt-based utility classes and functions (extends Qt library)

	Copyright (c) 2014-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Heiko Fechner
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 3 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

*/

#ifndef QtExt_ConstructionLayerH
#define QtExt_ConstructionLayerH

#include <QString>
#include <QColor>

#include <IBK_math.h>

#include <QtExt_Constants.h>

namespace QtExt {

/*! Class contains basic parameter for 1D construction layer.*/
class ConstructionLayer {
public:
	/*! Standard constructor. Creates an invalid object.
		Validity can be checked by test m_id != -1.
	*/
	ConstructionLayer();

	/*! Global comparison operator for equal. Necessary for using associative container.*/
	friend bool operator==(const ConstructionLayer& lhs, const ConstructionLayer& rhs) {
		if( !IBK::nearly_equal<4>(lhs.m_width,rhs.m_width))
			return false;
		if(lhs.m_name != rhs.m_name)
			return false;
		if(lhs.m_color != rhs.m_color)
			return false;
		if(lhs.m_id != rhs.m_id)
			return false;
		return true;
	}

	/*! Global comparison operator for not equal. Necessary for using associative container.*/
	friend bool operator!=(const ConstructionLayer& lhs, const ConstructionLayer& rhs) {
		if( !IBK::nearly_equal<4>(lhs.m_width,rhs.m_width))
			return true;
		if(lhs.m_name != rhs.m_name)
			return true;
		if(lhs.m_color != rhs.m_color)
			return true;
		if(lhs.m_id != rhs.m_id)
			return true;
		return false;
	}

	double			m_width;	///< Width of the layer in m.
	QString			m_name;		///< Material name.
	QColor			m_color;	///< Material color.
	int				m_id;		///< Material ID
	HatchingType	m_hatch;	///< Material hatching
};

} // namespace QtExt

#endif // QtExt_ConstructionLayerH
