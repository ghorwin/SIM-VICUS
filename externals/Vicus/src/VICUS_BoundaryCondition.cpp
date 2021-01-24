#include "VICUS_BoundaryCondition.h"

namespace VICUS {

bool BoundaryCondition::isValid() const
{

	if(m_id == VICUS::INVALID_ID)
		return false;

	if((m_heatConduction.m_modelType == NANDRAD::InterfaceHeatConduction::MT_Constant &&
			m_heatConduction.m_para[NANDRAD::InterfaceHeatConduction::P_HeatTransferCoefficient].value < 0))
		return false;

	if(m_longWaveEmission.m_modelType == NANDRAD::InterfaceLongWaveEmission::MT_Constant &&
			(m_longWaveEmission.m_para[NANDRAD::InterfaceLongWaveEmission::P_Emissivity].value < 0 ||
			 m_longWaveEmission.m_para[NANDRAD::InterfaceLongWaveEmission::P_Emissivity].value > 1 ))
		return false;

	if(m_solarAbsorption.m_modelType == NANDRAD::InterfaceSolarAbsorption::MT_Constant &&
			(m_solarAbsorption.m_para[NANDRAD::InterfaceSolarAbsorption::P_AbsorptionCoefficient].value < 0 ||
			 m_solarAbsorption.m_para[NANDRAD::InterfaceSolarAbsorption::P_AbsorptionCoefficient].value > 1 ))
		return false;

	return true;
}


} // namespace VICUS
