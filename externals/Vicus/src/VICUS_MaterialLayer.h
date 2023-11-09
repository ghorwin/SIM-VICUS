/*	The SIM-VICUS data model library.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Dirk Weiss  <dirk.weiss -[at]- tu-dresden.de>
	  Stephan Hirth  <stephan.hirth -[at]- tu-dresden.de>
	  Hauke Hirsch  <hauke.hirsch -[at]- tu-dresden.de>

	  ... all the others from the SIM-VICUS team ... :-)

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

#ifndef VICUS_MaterialLayerH
#define VICUS_MaterialLayerH

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"
#include "VICUS_Database.h"
#include "VICUS_Material.h"

#include <QString>

#include <vector>

#include <IBK_Flag.h>
#include <IBK_Parameter.h>
#include <IBK_IntPara.h>

namespace VICUS {

/*! Stores data for a single material layer in a construction. */
class MaterialLayer {
public:

	/*! Basic parameters. */
	enum para_t {
		//thermal paramters
		/*! Dry density of the material. */
		P_Thickness,			// Keyword: Thickness				[m]	'Thickness of the material layer.'
		/*! Specific heat capacity of the material. */
		P_LifeTime,				// Keyword: Lifetime				[a]	'Lifetime of the material layer.'

		NUM_P
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	/*! Default c'tor. */
	MaterialLayer() {}

	/*! Simple Constructor with thickness in [m] and material id. */
	MaterialLayer(double thickness, unsigned int id):
		m_idMaterial(id)
	{
		m_para[P_Thickness].set("Thickness", thickness, IBK::Unit("m"));
		m_para[P_LifeTime].set("Lifetime", 50, IBK::Unit("a")); // We take 50 years as standard
	}

	/*! Simple Constructor with thickness and material id. */
	MaterialLayer(IBK::Parameter thickness, unsigned int id):
		m_idMaterial(id)
	{
		m_para[P_Thickness] = thickness;
		m_para[P_LifeTime].set("Lifetime", 50, IBK::Unit("a")); // We take 50 years as standard
	}


	VICUS_READWRITE

	/*! Checks if all referenced materials exist and if their parameters are valid. */
	bool isValid(const VICUS::Database<VICUS::Material> & materials) const;

	/*! Inequality operator. */
	bool operator!=(const MaterialLayer & other) const {
		return (m_idMaterial != other.m_idMaterial ||
				m_para[P_LifeTime] != other.m_para[P_LifeTime] ||
				m_para[P_Thickness] != other.m_para[P_Thickness] ||
				m_cost != other.m_cost);
	}
	/*! Equality operator. */
	bool operator==(const MaterialLayer & other) const { return !operator!=(other); }

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique ID of material. */
	unsigned int					m_idMaterial = INVALID_ID;	// XML:A:required

	/*! IBK::Parameter. */
	IBK::Parameter					m_para[NUM_P];				// XML:E:required

	/*! Cost of the material layer in Euro Cent. */
	IBK::IntPara					m_cost;						// XML:E

	/*! Holds error string in order to give users a tooltip in db dialog. */
	mutable std::string				m_errorMsg;

};

} // namespace VICUS


#endif // VICUS_MaterialLayerH
