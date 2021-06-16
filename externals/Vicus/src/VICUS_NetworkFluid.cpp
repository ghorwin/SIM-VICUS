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

#include "VICUS_NetworkFluid.h"


namespace VICUS {


void NetworkFluid::defaultFluidWater(unsigned int id){
	m_id = id;
	m_displayName = IBK::MultiLanguageString("Water");
	m_para[P_Density] = IBK::Parameter("Density", 998, "kg/m3");
	m_para[P_Conductivity] = IBK::Parameter("Conductivity", 0.6, "W/mK");
	m_para[P_HeatCapacity] = IBK::Parameter("HeatCapacity", 4180, "J/kgK");

	m_kinematicViscosity.m_name = "KinematicViscosity";
	m_kinematicViscosity.m_interpolationMethod = NANDRAD::LinearSplineParameter::I_LINEAR;
	m_kinematicViscosity.m_xUnit = IBK::Unit("C");
	m_kinematicViscosity.m_yUnit = IBK::Unit("m2/s");
	m_kinematicViscosity.m_values.setValues(std::vector<double>{0,10,20,30,40,50,60,70,80,90},
											std::vector<double>{1.793e-6,1.307e-6,1.004e-6,0.801e-6,0.658e-6,0.554e-6,0.475e-6,0.413e-6,0.365e-6,0.326e-6});
}


bool NetworkFluid::isValid() const {
	if (m_para[P_Density].value <= 0)
		return false;
	if (m_para[P_Conductivity].value <= 0)
		return false;
	if (m_para[P_HeatCapacity].value <= 0)
		return false;

	try {
		NANDRAD::LinearSplineParameter kinVisc = m_kinematicViscosity;
		kinVisc.checkAndInitialize("KinematicViscosity", IBK::Unit("K"), IBK::Unit("m2/s"), IBK::Unit("m2/s"),
									std::numeric_limits<double>::lowest(), true,
									std::numeric_limits<double>::max(), true, nullptr);
	}
	catch (IBK::Exception &) {
		return false;
	}
	return true;
}


AbstractDBElement::ComparisonResult NetworkFluid::equal(const AbstractDBElement *other) const {
	const NetworkFluid * otherNetFluid = dynamic_cast<const NetworkFluid*>(other);
	if (otherNetFluid == nullptr)
		return Different;

	//first check critical data

	//check parameters
	for(unsigned int i=0; i<NUM_P; ++i){
		if(m_para[i] != otherNetFluid->m_para[i])
			return Different;
	}
	if(m_kinematicViscosity != otherNetFluid->m_kinematicViscosity)
		return Different;

	//check meta data

	if(m_displayName != otherNetFluid->m_displayName ||
			m_color != otherNetFluid->m_color)
		return OnlyMetaDataDiffers;

	return Equal;
}

} // namespace VICUS
