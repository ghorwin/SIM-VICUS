#include "NM_ThermalNetworkFlowElements.h"

#include "NANDRAD_HydraulicFluid.h"
#include "NANDRAD_HydraulicNetworkElement.h"
#include "NANDRAD_HydraulicNetworkPipeProperties.h"
#include "NANDRAD_HydraulicNetworkComponent.h"

#define PI				3.141592653589793238

namespace NANDRAD_MODEL {

// *** HNPipeElement ***

TNPipeElement::TNPipeElement(const NANDRAD::HydraulicNetworkElement & elem,
							const NANDRAD::HydraulicNetworkPipeProperties & pipePara,
							const NANDRAD::HydraulicFluid & fluid):
	m_fluid(&fluid)
{
	FUNCID(TNPipeElement::TNPipeElement);

	m_length = elem.m_para[NANDRAD::HydraulicNetworkElement::P_Length].value;
	m_diameter = pipePara.m_para[NANDRAD::HydraulicNetworkPipeProperties::P_HydraulicDiameter].value;
	const double wallSpecificUValue =
			pipePara.m_para[NANDRAD::HydraulicNetworkPipeProperties::P_LengthSpecificUValue].value;
	// TODO : add correponding parameeter to pipe properties
	const double wallThickness = 0.01;
	// TODO : perform parameter checking inide NANDRAD data structure, so that we avoid
	// exceptions at this place
	if (m_length<=0)
		throw IBK::Exception(IBK::FormatString("HydraulicNetworkElement with id %1 has length <= 0").arg(elem.m_id),FUNC_ID);
	if (m_diameter<=0)
		throw IBK::Exception(IBK::FormatString("HydraulicNetworkElement with id %1 has diameter <= 0").arg(elem.m_id),FUNC_ID);
	if (wallSpecificUValue<=0)
		throw IBK::Exception(IBK::FormatString("HydraulicNetworkElement with id %1 has specific UValue <= 0").arg(elem.m_id),FUNC_ID);

	// calculate thermal resstance
	m_thermalResistanceWall = 1.0/(2.0 * wallSpecificUValue)
			* std::log((m_diameter + wallThickness)/m_diameter);

	// calculate fluid volume inside the pipe
	m_volume = PI/4. * m_diameter * m_diameter * m_length;
}

TNPipeElement::~TNPipeElement()
{

}

unsigned int TNPipeElement::nInternalStates() const
{
	return 1;
}

void TNPipeElement::setInternalStates(const double * y)
{
	// calculate specific enthalpy
	m_specificEnthalpy = y[0] / (m_volume * m_fluid->m_para[NANDRAD::HydraulicFluid::P_Density].value);
}

void TNPipeElement::internalDerivatives(double * ydot)
{
	// calculate heat transfer
	const double thermalResistance = 1.0/(m_innerHeatTransfer * m_diameter)
			+ 1.0/(m_ambientHeatTransfer * m_diameter)
			+ m_thermalResistanceWall;
	// calculate thermal transmittance
	const double UValue = PI/thermalResistance;
	// calculate dimensionless heat transfer number
	const double NTU = UValue * m_length/
			(m_massFlux *m_fluid->m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value);
	// calculate heat loss with gven parameters
	m_heatLoss = m_massFlux * m_fluid->m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value *
			(m_inletTemperature - m_ambientTemperature) *
			(1. - std::exp(-NTU));
	const double specificInletEnthalpy = m_fluid->m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value *
			m_inletTemperature;
	// heat fluxes into the fluid and enthalpy change are heat sources
	ydot[0] = -m_heatLoss + m_massFlux * (specificInletEnthalpy - m_specificEnthalpy);
}

void TNPipeElement::setInletFluxes(double mdot, double Hdot)
{
	// calculate inlet temperature
	m_inletTemperature = Hdot/(mdot * m_fluid->m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value);
	// copy mass flux
	m_massFlux = mdot;
	// calculate inner heat transfer and for simpicity use laminar flow equations
	// TODO: add turbulent flow equations
	// first calculate reynolds number
	const double velocity = mdot/(m_volume * m_fluid->m_para[NANDRAD::HydraulicFluid::P_Density].value);
	// TODO: add dynamic viscosity to fluid porperties
	const double viscosity = 0.7e-06;
	const double reynolds = velocity * m_diameter/viscosity;
	// calculate prandtl number
	const double prandtl = velocity * m_fluid->m_para[NANDRAD::HydraulicFluid::P_Density].value *
			m_fluid->m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value/
			m_fluid->m_para[NANDRAD::HydraulicFluid::P_Conductivity].value;
	// calculate laminar nusselt number
	double val = 1.615 * (reynolds*prandtl * m_diameter/m_length-0.7);
	const double nusselt = std::pow(49.37 + val*val*val,0.33333);
	// calculate convection coefficient for the inner side
	m_innerHeatTransfer = nusselt * m_fluid->m_para[NANDRAD::HydraulicFluid::P_Conductivity].value/m_diameter;
}

void TNPipeElement::setAmbientConditions(double Tamb, double alphaAmb)
{
	//copy ambient temperature
	m_ambientTemperature = Tamb;
	//copy ambient heat transfer coefficient
	m_ambientHeatTransfer = alphaAmb;
}

void TNPipeElement::outletSpecificEnthalpy(double & h) const
{
	// constant interpolation of specific enthlpy
	h = m_specificEnthalpy;
}



} // namespace NANDRAD_MODEL
