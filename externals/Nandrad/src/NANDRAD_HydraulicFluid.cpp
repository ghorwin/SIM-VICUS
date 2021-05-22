#include "NANDRAD_HydraulicFluid.h"

#include "NANDRAD_HydraulicNetwork.h"

#include <IBK_UnitList.h>

namespace NANDRAD {

void HydraulicFluid::defaultFluidWater(unsigned int id){
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

} // namespace NANDRAD
