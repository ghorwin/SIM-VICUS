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
	m_length = elem.m_para[NANDRAD::HydraulicNetworkElement::P_Length].value;
	m_innerDiameter = pipePara.m_para[NANDRAD::HydraulicNetworkPipeProperties::P_PipeInnerDiameter].value;
	m_outerDiameter = pipePara.m_para[NANDRAD::HydraulicNetworkPipeProperties::P_PipeOuterDiameter].value;
	m_UValuePipeWall = pipePara.m_para[NANDRAD::HydraulicNetworkPipeProperties::P_UValuePipeWall].value;
	m_outerHeatTransferCoefficient =
			comp.m_para[NANDRAD::HydraulicNetworkComponent::P_ExternalHeatTransferCoefficient].value;

	// check thermal parameters only if heat transfer is enabled
	if (comp.m_heatExchangeType != NANDRAD::HydraulicNetworkComponent::NUM_HT) {
		m_haveHeatExchange = true;
	}
	else {
		m_haveHeatExchange = false;
	}

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

void TNPipeElement::setInitialTemperature(double T0) {
	m_temperature = T0;
	m_specificEnthalpy = T0 * m_fluid->m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value;
}

void TNPipeElement::initialInternalStates(double * y0) {
	// heat fluxes into the fluid and enthalpy change are heat sources
	y0[0] = m_specificEnthalpy * m_volume * m_fluid->m_para[NANDRAD::HydraulicFluid::P_Density].value;
}

void TNPipeElement::setInternalStates(const double * y)
{
	// calculate specific enthalpy
	m_specificEnthalpy = y[0] / (m_volume * m_fluid->m_para[NANDRAD::HydraulicFluid::P_Density].value);
	m_temperature = m_specificEnthalpy / m_fluid->m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value;
}

void TNPipeElement::internalDerivatives(double * ydot)
{
	// heat fluxes into the fluid and enthalpy change are heat sources
	ydot[0] = -m_heatLoss + m_massFlux * (m_inletSpecificEnthalpy - m_outletSpecificEnthalpy);
}


void TNPipeElement::setNodalConditions(double mdot, double hInlet, double hOutlet)
{
	// copy mass flux
	m_massFlux = mdot;

	// mass flux from inlet to outlet
	if(mdot >= 0) {
		// retrieve inlet specific enthalpy
		m_inletSpecificEnthalpy = hInlet;
		// calculate inlet temperature
		m_inletTemperature = m_inletSpecificEnthalpy/m_fluid->m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value;
		// set outlet specific enthalpy
		m_outletSpecificEnthalpy = m_specificEnthalpy;
		// set outlet temperature
		m_outletTemperature = m_temperature;
	}
	// reverse direction
	else {
		// retrieve outlet specific enthalpy
		m_outletSpecificEnthalpy = hOutlet;
		// calculate inlet temperature
		m_outletTemperature = m_outletSpecificEnthalpy/m_fluid->m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value;
		// set outlet specific enthalpy
		m_inletSpecificEnthalpy = m_specificEnthalpy;
		// set outlet temperature
		m_inletTemperature = m_temperature;
	}

	if (m_haveHeatExchange) {

		// calculate inner heat transfer and for simpicity use laminar flow equations
		// TODO Hauke: add turbulent flow equations
		// first calculate reynolds number
		const double velocity = std::fabs(mdot)/(m_volume * m_fluid->m_para[NANDRAD::HydraulicFluid::P_Density].value);
		// TODO Anne: we use the kinematic viscosity based on inlet temperature? Should be ok.
		const double reynolds = velocity * m_innerDiameter/m_fluid->m_kinematicViscosity.m_values.value(m_temperature);
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
														+ 1.0/(m_outerHeatTransferCoefficient * m_outerDiameter)
														+ 1.0/(2.0*m_UValuePipeWall) );

		if(mdot >= 0) {
			// calculate heat loss with given parameters
			m_heatLoss = m_massFlux * m_fluid->m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value *
					(m_inletTemperature - m_ambientTemperature) *
					(1. - std::exp(-totalUValuePipe / (std::fabs(m_massFlux) * m_fluid->m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value )));
		}
		else {
			// calculate heat loss with given parameters
			m_heatLoss = std::fabs(m_massFlux) * m_fluid->m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value *
					(m_outletTemperature - m_ambientTemperature) *
					(1. - std::exp(-totalUValuePipe / (std::fabs(m_massFlux) * m_fluid->m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value )));
		}
	}
	else {
		m_heatLoss = 0;
	}
}

void TNPipeElement::setAmbientConditions(double Tamb, double /*alphaAmb*/)
{
	//copy ambient temperature
	m_ambientTemperature = Tamb;
}

double TNPipeElement::meanTemperature() const {
	return m_temperature;
}

double TNPipeElement::inletSpecificEnthalpy() const
{
	// constant interpolation of specific enthlpy
	return m_inletSpecificEnthalpy;
}

double TNPipeElement::outletSpecificEnthalpy() const
{
	// constant interpolation of specific enthlpy
	return m_outletSpecificEnthalpy;
}

double TNPipeElement::inletTemperature() const
{
	// constant interpolation of specific enthlpy
	return m_inletTemperature;
}

double TNPipeElement::outletTemperature() const
{
	// constant interpolation of specific enthlpy
	return m_outletTemperature;
}

double TNPipeElement::heatLoss() const  {
	return m_heatLoss;
}



// *** HeatExchanger ***

TNHeatExchanger::TNHeatExchanger(const NANDRAD::HydraulicNetworkElement & elem,
							 const NANDRAD::HydraulicNetworkComponent & comp,
							const NANDRAD::HydraulicFluid & fluid):
	m_fluid(&fluid)
{
	m_volume = comp.m_para[NANDRAD::HydraulicNetworkComponent::P_Volume].value;
	m_UAValue = comp.m_para[NANDRAD::HydraulicNetworkComponent::P_UAValue].value;
	m_heatFluxToAmbient = elem.m_para[NANDRAD::HydraulicNetworkElement::P_HeatFlux].value;
}

TNHeatExchanger::~TNHeatExchanger()
{

}

unsigned int TNHeatExchanger::nInternalStates() const
{
	return 1;
}

void TNHeatExchanger::setInitialTemperature(double T0) {
	m_temperature = T0;
	m_specificEnthalpy = T0 * m_fluid->m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value;
}

void TNHeatExchanger::initialInternalStates(double * y0) {
	// heat fluxes into the fluid and enthalpy change are heat sources
	y0[0] = m_specificEnthalpy * m_volume * m_fluid->m_para[NANDRAD::HydraulicFluid::P_Density].value;
}

void TNHeatExchanger::setInternalStates(const double * y)
{
	m_specificEnthalpy = y[0] / (m_volume * m_fluid->m_para[NANDRAD::HydraulicFluid::P_Density].value);
	m_temperature = m_specificEnthalpy / m_fluid->m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value;
}

void TNHeatExchanger::internalDerivatives(double * ydot)
{
	// heat fluxes into the fluid and enthalpy change are heat sources
	ydot[0] = -heatLoss() + m_massFlux * (m_inletSpecificEnthalpy - m_outletSpecificEnthalpy);
}

void TNHeatExchanger::setNodalConditions(double mdot, double hInlet, double hOutlet)
{
	// copy mass flux
	m_massFlux = mdot;

	// mass flux from inlet to outlet
	if(mdot >= 0) {
		// retrieve inlet specific enthalpy
		m_inletSpecificEnthalpy = hInlet;
		// calculate inlet temperature
		m_inletTemperature = m_inletSpecificEnthalpy/m_fluid->m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value;
		// set outlet specific enthalpy
		m_outletSpecificEnthalpy = m_specificEnthalpy;
		// set outlet temperature
		m_outletTemperature = m_temperature;
	}
	// reverse direction
	else {
		// retrieve outlet specific enthalpy
		m_outletSpecificEnthalpy = hOutlet;
		// calculate inlet temperature
		m_outletTemperature = m_outletSpecificEnthalpy/m_fluid->m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value;
		// set outlet specific enthalpy
		m_inletSpecificEnthalpy = m_specificEnthalpy;
		// set outlet temperature
		m_inletTemperature = m_temperature;
	}
}

void TNHeatExchanger::setAmbientConditions(double /*Tamb*/, double /*alphaAmb*/)
{
}

double TNHeatExchanger::meanTemperature() const {
	return m_temperature;
}

double TNHeatExchanger::inletSpecificEnthalpy() const
{
	// constant interpolation of specific enthlpy
	return m_inletSpecificEnthalpy;
}

double TNHeatExchanger::outletSpecificEnthalpy() const
{
	// constant interpolation of specific enthlpy
	return m_outletSpecificEnthalpy;
}

double TNHeatExchanger::inletTemperature() const
{
	// constant interpolation of specific enthlpy
	return m_inletTemperature;
}

double TNHeatExchanger::outletTemperature() const
{
	// constant interpolation of specific enthlpy
	return m_outletTemperature;
}

double TNHeatExchanger::heatLoss() const
{
	return m_heatFluxToAmbient;
}



// *** Pump ***

TNPump::TNPump(const NANDRAD::HydraulicNetworkElement & /*elem*/,
							 const NANDRAD::HydraulicNetworkComponent & comp,
							const NANDRAD::HydraulicFluid & fluid):
	m_fluid(&fluid)
{
	m_volume = comp.m_para[NANDRAD::HydraulicNetworkComponent::P_Volume].value;
}

TNPump::~TNPump()
{

}

unsigned int TNPump::nInternalStates() const
{
	return 1;
}

void TNPump::initialInternalStates(double * y0) {
	// heat fluxes into the fluid and enthalpy change are heat sources
	y0[0] = m_specificEnthalpy * m_volume * m_fluid->m_para[NANDRAD::HydraulicFluid::P_Density].value;
}

void TNPump::setInternalStates(const double * y)
{
	m_specificEnthalpy = y[0] / (m_volume * m_fluid->m_para[NANDRAD::HydraulicFluid::P_Density].value);
	m_temperature = m_specificEnthalpy / m_fluid->m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value;
}

void TNPump::internalDerivatives(double * ydot)
{
	// heat fluxes into the fluid and enthalpy change are heat sources
	ydot[0] = -heatLoss() + m_massFlux * (m_inletSpecificEnthalpy - m_outletSpecificEnthalpy);
}

void TNPump::setInitialTemperature(double T0) {
	m_temperature = T0;
	m_specificEnthalpy = T0 * m_fluid->m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value;
}

void TNPump::setNodalConditions(double mdot, double hInlet, double hOutlet)
{
	// copy mass flux
	m_massFlux = mdot;

	// mass flux from inlet to outlet
	if(mdot >= 0) {
		// retrieve inlet specific enthalpy
		m_inletSpecificEnthalpy = hInlet;
		// calculate inlet temperature
		m_inletTemperature = m_inletSpecificEnthalpy/m_fluid->m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value;
		// set outlet specific enthalpy
		m_outletSpecificEnthalpy = m_specificEnthalpy;
		// set outlet temperature
		m_outletTemperature = m_temperature;
	}
	// reverse direction
	else {
		// retrieve outlet specific enthalpy
		m_outletSpecificEnthalpy = hOutlet;
		// calculate inlet temperature
		m_outletTemperature = m_outletSpecificEnthalpy/m_fluid->m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value;
		// set outlet specific enthalpy
		m_inletSpecificEnthalpy = m_specificEnthalpy;
		// set outlet temperature
		m_inletTemperature = m_temperature;
	}
}

void TNPump::setAmbientConditions(double /*Tamb*/, double /*alphaAmb*/)
{
}

double TNPump::meanTemperature() const {
	return m_temperature;
}

double TNPump::inletSpecificEnthalpy() const
{
	// constant interpolation of specific enthlpy
	return m_inletSpecificEnthalpy;
}

double TNPump::outletSpecificEnthalpy() const
{
	// constant interpolation of specific enthlpy
	return m_outletSpecificEnthalpy;
}

double TNPump::inletTemperature() const
{
	// constant interpolation of specific enthlpy
	return m_inletTemperature;
}

double TNPump::outletTemperature() const
{
	// constant interpolation of specific enthlpy
	return m_outletTemperature;
}

double TNPump::heatLoss() const
{
	return 0.0;
}

} // namespace NANDRAD_MODEL
