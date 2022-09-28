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

#include "NANDRAD_HydraulicFluid.h"

#include "NANDRAD_HydraulicNetwork.h"

#include <IBK_UnitList.h>

namespace NANDRAD {

void HydraulicFluid::defaultFluidWater() {
	m_displayName = "Water";
	m_para[P_Density] = IBK::Parameter("Density", 998, "kg/m3");
	m_para[P_Conductivity] = IBK::Parameter("Conductivity", 0.6, "W/mK");
	m_para[P_HeatCapacity] = IBK::Parameter("HeatCapacity", 4180, "J/kgK");

	m_kinematicViscosity.m_name = "KinematicViscosity";
	m_kinematicViscosity.m_interpolationMethod = LinearSplineParameter::I_LINEAR;
	m_kinematicViscosity.m_xUnit = IBK::Unit("C");
	m_kinematicViscosity.m_yUnit = IBK::Unit("m2/s");
	m_kinematicViscosity.m_values.setValues(std::vector<double>{0,10,20,30,40,50,60,70,80,90},
											std::vector<double>{1.793e-6,1.307e-6,1.004e-6,0.801e-6,0.658e-6,0.554e-6,0.475e-6,0.413e-6,0.365e-6,0.326e-6});
}


void HydraulicFluid::defaultFluidAir()
{
	m_displayName = "Air";
	m_para[P_Density] = IBK::Parameter("Density", 1.205, "kg/m3");
	m_para[P_Conductivity] = IBK::Parameter("Conductivity", 0.0262, "W/mK");
	m_para[P_HeatCapacity] = IBK::Parameter("HeatCapacity", 1006, "J/kgK");

	m_kinematicViscosity.m_name = "KinematicViscosity";
	m_kinematicViscosity.m_interpolationMethod = LinearSplineParameter::I_LINEAR;
	m_kinematicViscosity.m_xUnit = IBK::Unit("C");
	m_kinematicViscosity.m_yUnit = IBK::Unit("m2/s");
	m_kinematicViscosity.m_values.setValues(std::vector<double>{0,90},
											std::vector<double>{17.2e-6,17.2e-6});
}


void HydraulicFluid::checkParameters(int networkModelType) {

	// check for required parameters and meaningful value ranges
	m_para[P_Density].checkedValue("Density", "kg/m3", "kg/m3", 0, false, std::numeric_limits<double>::max(), false,
								   "Density of fluid must be > 0 kg/m3.");

	// kinematic viscosity is always needed, here we check the spline and convert it to base units automatically
	m_kinematicViscosity.checkAndInitialize("KinematicViscosity", IBK::Unit("K"), IBK::Unit("m2/s"),
											IBK::Unit("m2/s"), 0, false, std::numeric_limits<double>::max(), false,
											"Kinematic viscosity must be > 0 m2/s.");

	// check thermal properties, but only when required
	HydraulicNetwork::ModelType modelType = (HydraulicNetwork::ModelType) networkModelType;
	if (modelType == HydraulicNetwork::MT_ThermalHydraulicNetwork) {
		m_para[P_HeatCapacity].checkedValue("HeatCapacity", "J/kgK", "J/kgK", 0, false, std::numeric_limits<double>::max(), false,
									   "Heat capacity of fluid must be > 0 J/kgK.");
		m_para[P_Conductivity].checkedValue("Conductivity", "W/mK", "W/mK", 0, false, std::numeric_limits<double>::max(), false,
									   "Thermal conductivity of fluid must be > 0 W/mK.");
	}

}


bool HydraulicFluid::equal(const HydraulicFluid &other) const {

	//check parameters
	for(unsigned int i=0; i<NUM_P; ++i){
		if(m_para[i] != other.m_para[i])
			return false;
	}

	if(m_kinematicViscosity != other.m_kinematicViscosity)
		return false;

	return true;
}

} // namespace NANDRAD
