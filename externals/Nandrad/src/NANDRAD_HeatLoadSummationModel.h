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

#ifndef NANDRAD_HeatLoadSummationModelH
#define NANDRAD_HeatLoadSummationModelH

#include <IBK_Parameter.h>

#include "NANDRAD_Constants.h"
#include "NANDRAD_CodeGenMacros.h"

namespace NANDRAD {

/*! A model for summation of heat load from different zones, construction instances or networks.
	The resulting heat load summation value represents the energy flux towards (positive) or from (negative)
	the considered balance variables.

	The object list, or rather the reference type used by the object list, uniquely determines
	the model result variables that are accumulated:

	ConstructionInstance:
	- ConstructionInstance.ActiveLayerThermalLoad

	NetworkElement:
	- NetworkElement.HeatLoss

	Zone:
	- Zone.IdealHeatingLoad
	- Zone.IdealCoolingLoad  (defined as positive quantity, even though it is a negative energy constribution
							  to the zone energy balance)

	The signs of the values for NetworkElement.HeatLoss and Zone.IdealCoolingLoad are inverted so that positive
	summation values represet a net gain of energy in the group of considered objects.

	As can be seen, in the case of Zone as reference type, there are two variables, which may or
	may not be used in the model. This is distinguished by an additional variable: useZoneCoolingLoad.

	Note: with one model parametrization block you may not sum up *both* IdealHeatingLoad and IdealCoolingLoad.
*/
class HeatLoadSummationModel {
public:

	NANDRAD_READWRITE
	NANDRAD_COMPARE_WITH_ID

	/*! Checks parameters for valid values. */
	void checkParameters();

	/*! Unique ID-number for this model. */
	unsigned int		m_id = NANDRAD::INVALID_ID;					// XML:A:required
	/*! Some display/comment name for this model (optional). */
	std::string			m_displayName;								// XML:A

	/*! If true, and if the object list defines 'Zone' as reference type, then the variable
		'IdealHeatingLoad' is requested, otherwise 'IdealHeatingLoad' is used (the default).
	*/
	bool				m_zoneCoolingLoad = false;					// XML:A
	/*! Object list with zones that this model is to be apply to. */
	std::string			m_objectList;								// XML:E:required
};

} // namespace NANDRAD

#endif // NANDRAD_HeatLoadSummationModelH
