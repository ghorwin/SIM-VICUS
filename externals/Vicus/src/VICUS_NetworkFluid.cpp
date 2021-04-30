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

bool NetworkFluid::isValid() const
{
	if (! (m_para[P_Density].value > 0 && m_para[P_Conductivity].value > 0 && m_para[P_HeatCapacity].value > 0))
		return false;
//	try {
//		m_kinematicViscosity.checkAndInitialize("KinematicViscosity", IBK::Unit("C"), IBK::Unit("m2/s"), IBK::Unit("m2/s"),
//												std::numeric_limits<double>::lowest(), true,
//												std::numeric_limits<double>::max(), true, nullptr);
//	} catch (IBK::Exception &ex) {
//		return false;
	//	}
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
