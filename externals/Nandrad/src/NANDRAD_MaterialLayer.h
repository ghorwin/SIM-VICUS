/*	The NANDRAD data model library.

	Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Anne Paepcke     <anne.paepcke -[at]- tu-dresden.de>

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

#ifndef NANDRAD_MaterialLayerH
#define NANDRAD_MaterialLayerH

#include "NANDRAD_CodeGenMacros.h"

namespace NANDRAD {

class Material;

/*! A layer of a multi-layered construction. */
class MaterialLayer {
public:
	/*! Default c'tor. */
	MaterialLayer() {}

	/*! Simple Constructor with thickness in [m] and material id. */
	MaterialLayer(double thickness, unsigned int id):
		m_thickness(thickness),
		m_matId(id)
	{}

	NANDRAD_READWRITE

	/*! Inequality operator. */
	bool operator!=(const MaterialLayer & other) const { return (m_thickness != other.m_thickness || m_matId != other.m_matId);	}
	/*! Equality operator. */
	bool operator==(const MaterialLayer & other) const { return !operator!=(other); }

	/*! Thickness in [m]. */
	double					m_thickness;				// XML:A:required

	/*! Material id. */
	unsigned int			m_matId;					// XML:A:required

	// *** Variables used only during simulation ***

	/*! Quick-access pointer to referenced material. */
	const NANDRAD::Material	*m_material = nullptr;
};

} // namespace NANDRAD

#endif // NANDRAD_MaterialLayerH
