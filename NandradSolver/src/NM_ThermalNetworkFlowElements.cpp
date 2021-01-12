#include "NM_ThermalNetworkFlowElements.h"

#include "NANDRAD_HydraulicFluid.h"
#include "NANDRAD_HydraulicNetworkElement.h"
#include "NANDRAD_HydraulicNetworkPipeProperties.h"
#include "NANDRAD_HydraulicNetworkComponent.h"

#define PI				3.141592653589793238

namespace NANDRAD_MODEL {

// *** HNPipeElement ***

TNPipeElement::TNPipeElement(const NANDRAD::HydraulicNetworkElement & elem,
							 const NANDRAD::HydraulicNetworkComponent & comp,
							const NANDRAD::HydraulicNetworkPipeProperties & pipePara,
							const NANDRAD::HydraulicFluid & fluid):
	m_fluid(&fluid)
{
	FUNCID(TNPipeElement::TNPipeElement);

	m_length = elem.m_para[NANDRAD::HydraulicNetworkElement::P_Length].value;
	m_innerDiameter = pipePara.m_para[NANDRAD::HydraulicNetworkPipeProperties::P_PipeInnerDiameter].value;
	m_outerDiameter = pipePara.m_para[NANDRAD::HydraulicNetworkPipeProperties::P_PipeOuterDiameter].value;
	m_UValuePipeWall = pipePara.m_para[NANDRAD::HydraulicNetworkPipeProperties::P_UValuePipeWall].value;
	m_outerHeatTransferCoefficient =
			pipePara.m_para[NANDRAD::HydraulicNetworkComponent::P_ExternalHeatTransferCoefficient].value;

	// TODO : perform parameter checking inide NANDRAD data structure, so that we avoid
	// exceptions at this place
	if (m_length<=0)
		throw IBK::Exception(IBK::FormatString("HydraulicNetworkElement with id %1 has length <= 0").arg(elem.m_id),FUNC_ID);
	if (m_innerDiameter<=0)
		throw IBK::Exception(IBK::FormatString("HydraulicNetworkElement with id %1 has inner diameter <= 0").arg(elem.m_id),FUNC_ID);
	if (m_outerDiameter<=0)
		throw IBK::Exception(IBK::FormatString("HydraulicNetworkElement with id %1 has outer diameter <= 0").arg(elem.m_id),FUNC_ID);
	if (m_UValuePipeWall<=0)
		throw IBK::Exception(IBK::FormatString("HydraulicNetworkElement with id %1 has U-Value <= 0").arg(elem.m_id),FUNC_ID);
	if (m_outerHeatTransferCoefficient<=0)
		throw IBK::Exception(IBK::FormatString("HydraulicNetworkElement with id %1 has outer heat transfer coefficient <= 0").arg(elem.m_id),FUNC_ID);

	// calculate fluid volume inside the pipe
	m_volume = PI/4. * m_innerDiameter * m_innerDiameter * m_length;
}

TNPipeElement::~TNPipeElement()
{

}

unsigned int TNPipeElement::nInternalStates() const
{
	return 1;
}

void TNPipeElement::initialTemperatures(double *T0) const {
	// TODO: retrieve initial temperature from fluid
	T0[0] = 293.15 ;
}

void TNPipeElement::setInternalStates(const double * y)
{
	// calculate specific enthalpy
	m_specificEnthalpy = y[0] / (m_volume * m_fluid->m_para[NANDRAD::HydraulicFluid::P_Density].value);
}

void TNPipeElement::internalDerivatives(double * ydot)
{
	// heat fluxes into the fluid and enthalpy change are heat sources
	ydot[0] = -m_heatLoss + m_massFlux * (m_inletSpecificEnthalpy - m_specificEnthalpy);
}

void TNPipeElement::setInletFluxes(double mdot, double Hdot)
{
	// claculate inlet specific enthalpy
	m_inletSpecificEnthalpy = Hdot/mdot;
	// calculate inlet temperature
	m_inletTemperature = m_inletSpecificEnthalpy/m_fluid->m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value;
	// copy mass flux
	m_massFlux = mdot;

	// calculate inner heat transfer and for simpicity use laminar flow equations
	// TODO Hauke: add turbulent flow equations
	// first calculate reynolds number
	const double velocity = mdot/(m_volume * m_fluid->m_para[NANDRAD::HydraulicFluid::P_Density].value);
	// TODO Anne: we use the kinematic viscosity based on inlet temperature? Should be ok.
	const double reynolds = velocity * m_innerDiameter/m_fluid->m_kinematicViscosity.m_values.value(m_inletTemperature);
	// calculate prandtl number
	const double prandtl = velocity * m_fluid->m_para[NANDRAD::HydraulicFluid::P_Density].value *
			m_fluid->m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value/
			m_fluid->m_para[NANDRAD::HydraulicFluid::P_Conductivity].value;
	// calculate laminar nusselt number
	double val = 1.615 * (reynolds*prandtl * m_innerDiameter/m_length-0.7);
	const double nusselt = std::pow(49.37 + val*val*val,0.33333);
	// calculate convection coefficient for the inner side
	m_innerHeatTransferCoefficient = nusselt * m_fluid->m_para[NANDRAD::HydraulicFluid::P_Conductivity].value/m_innerDiameter;

	// calculate heat transfer
	const double totalUValuePipe = (PI*m_length) / (1.0/(m_innerHeatTransferCoefficient * m_innerDiameter)
													+ 1.0/(m_outerHeatTransferCoefficient * m_innerDiameter)
													+ 1.0/(2.0*m_UValuePipeWall) );

	// calculate heat loss with given parameters
	m_heatLoss = std::fabs(m_massFlux) * m_fluid->m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value *
			(m_inletTemperature - m_ambientTemperature) *
			(1. - std::exp(-totalUValuePipe / (std::fabs(m_massFlux) * m_fluid->m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value )));
}

void TNPipeElement::setAmbientConditions(double Tamb, double alphaAmb)
{
	//copy ambient temperature
	m_ambientTemperature = Tamb;
}

double TNPipeElement::outletSpecificEnthalpy() const
{
	// constant interpolation of specific enthlpy
	return m_specificEnthalpy;
}

double TNPipeElement::heatLoss() const  {
	return m_heatLoss;
}

double TNPipeElement::volume() const  {
	return m_volume;
}



// *** HeatExchanger ***

TNHeatExchanger::TNHeatExchanger(const NANDRAD::HydraulicNetworkElement & elem,
							 const NANDRAD::HydraulicNetworkComponent & comp,
							const NANDRAD::HydraulicFluid & fluid):
	m_fluid(&fluid)
{
	FUNCID(TNHeatExchanger::TNHeatExchanger);

	m_volume = comp.m_para[NANDRAD::HydraulicNetworkComponent::P_Volume].value;
	m_UAValue = comp.m_para[NANDRAD::HydraulicNetworkComponent::P_UAValue].value;
	m_heatFluxToAmbient = elem.m_para[NANDRAD::HydraulicNetworkElement::P_HeatFlux].value;

	// TODO : perform parameter checking inide NANDRAD data structure, so that we avoid
	// exceptions at this place
	if (m_volume<=0)
		throw IBK::Exception(IBK::FormatString("HydraulicNetworkElement with id %1 has volume <= 0").arg(elem.m_id),FUNC_ID);
	if (m_UAValue<=0)
		throw IBK::Exception(IBK::FormatString("HydraulicNetworkElement with id %1 has outer heat transfer coefficient <= 0").arg(elem.m_id),FUNC_ID);
}

TNHeatExchanger::~TNHeatExchanger()
{

}

unsigned int TNHeatExchanger::nInternalStates() const
{
	return 1;
}

void TNHeatExchanger::initialTemperatures(double *T0) const {
	// TODO: retrieve initial temperature from fluid
	T0[0] = 293.15 ;
}

void TNHeatExchanger::setInternalStates(const double * y)
{
	m_specificEnthalpy = y[0] / (m_volume * m_fluid->m_para[NANDRAD::HydraulicFluid::P_Density].value);
}

void TNHeatExchanger::internalDerivatives(double * ydot)
{
	// heat fluxes into the fluid and enthalpy change are heat sources
	ydot[0] = -heatLoss() + m_massFlux * (m_inletSpecificEnthalpy - m_specificEnthalpy);
}

void TNHeatExchanger::setInletFluxes(double mdot, double Hdot)
{
	// claculate inlet specific enthalpy
	m_inletSpecificEnthalpy = Hdot/mdot;
	// calculate inlet temperature
	m_inletTemperature = m_inletSpecificEnthalpy/m_fluid->m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value;
	// copy mass flux
	m_massFlux = mdot;
}

void TNHeatExchanger::setAmbientConditions(double /*Tamb*/, double /*alphaAmb*/)
{
}

double TNHeatExchanger::outletSpecificEnthalpy() const
{
	// constant interpolation of specific enthlpy
	return m_specificEnthalpy;
}

double TNHeatExchanger::heatLoss() const
{
	return m_heatFluxToAmbient;
}

double TNHeatExchanger::volume() const  {
	return m_volume;
}



// *** Pump ***

TNPump::TNPump(const NANDRAD::HydraulicNetworkElement & elem,
							 const NANDRAD::HydraulicNetworkComponent & comp,
							const NANDRAD::HydraulicFluid & fluid):
	m_fluid(&fluid)
{
	FUNCID(TNPump::TNPump);

	m_volume = comp.m_para[NANDRAD::HydraulicNetworkComponent::P_Volume].value;
	// TODO : perform parameter checking inide NANDRAD data structure, so that we avoid
	// exceptions at this place
	if (m_volume<=0)
		throw IBK::Exception(IBK::FormatString("HydraulicNetworkElement with id %1 has volume <= 0").arg(elem.m_id),FUNC_ID);
}

TNPump::~TNPump()
{

}

unsigned int TNPump::nInternalStates() const
{
	return 1;
}

void TNPump::initialTemperatures(double *T0) const {
	// TODO: retrieve initial temperature from fluid
	T0[0] = 293.15 ;
}

void TNPump::setInternalStates(const double * y)
{
	m_specificEnthalpy = y[0] / (m_volume * m_fluid->m_para[NANDRAD::HydraulicFluid::P_Density].value);
}

void TNPump::internalDerivatives(double * ydot)
{
	// heat fluxes into the fluid and enthalpy change are heat sources
	ydot[0] = -heatLoss() + m_massFlux * (m_inletSpecificEnthalpy - m_specificEnthalpy);
}

void TNPump::setInletFluxes(double mdot, double Hdot)
{
	// claculate inlet specific enthalpy
	m_inletSpecificEnthalpy = Hdot/mdot;
	// calculate inlet temperature
	m_inletTemperature = m_inletSpecificEnthalpy/m_fluid->m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value;
	// copy mass flux
	m_massFlux = mdot;
}

void TNPump::setAmbientConditions(double /*Tamb*/, double /*alphaAmb*/)
{
}

double TNPump::outletSpecificEnthalpy() const
{
	// constant interpolation of specific enthlpy
	return m_specificEnthalpy;
}

double TNPump::heatLoss() const
{
	return 0.0;
}

double TNPump::volume() const  {
	return m_volume;
}

} // namespace NANDRAD_MODEL
