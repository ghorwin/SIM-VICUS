/*	NANDRAD Solver Framework and Model Implementation.

	Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Anne Paepcke     <anne.paepcke -[at]- tu-dresden.de>

	This program is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

#include "NM_HydraulicNetworkFlowElements.h"
#include "NM_Physics.h"

#include <NANDRAD_HydraulicNetworkElement.h>
#include <NANDRAD_HydraulicNetworkPipeProperties.h>
#include <NANDRAD_HydraulicNetworkComponent.h>
#include "NANDRAD_HydraulicNetworkControlElement.h"
#include <NANDRAD_HydraulicFluid.h>

#include "NM_ThermalNetworkFlowElements.h"

#define PI				3.141592653589793238


namespace NANDRAD_MODEL {

// Definition of destructor is here, so that we have the code and virtual function table
// only once.
HydraulicNetworkAbstractFlowElement::~HydraulicNetworkAbstractFlowElement() {
}


// *** HNPipeElement ***

HNPipeElement::HNPipeElement(const NANDRAD::HydraulicNetworkElement & elem,
							const NANDRAD::HydraulicNetworkPipeProperties & pipePara,
							const NANDRAD::HydraulicFluid & fluid):
	m_fluid(&fluid)
{
	m_length = elem.m_para[NANDRAD::HydraulicNetworkElement::P_Length].value;
	m_diameter = pipePara.m_para[NANDRAD::HydraulicNetworkPipeProperties::P_PipeInnerDiameter].value;
	m_roughness = pipePara.m_para[NANDRAD::HydraulicNetworkPipeProperties::P_PipeRoughness].value;
	m_nParallelPipes = (unsigned int) elem.m_intPara[NANDRAD::HydraulicNetworkElement::IP_NumberParallelPipes].value;
}


double  HNPipeElement::systemFunction(double mdot, double p_inlet, double p_outlet) const {
	// In case of multiple parallel pipes, mdot is the mass flux through *all* pipes
	// (but all parallel sections together need the same pressure drop as a single one
	return p_inlet - p_outlet - pressureLossFriction(mdot/m_nParallelPipes);	// this is the system function
}


void HNPipeElement::partials(double mdot, double p_inlet, double p_outlet,
							 double & df_dmdot, double & df_dp_inlet, double & df_dp_outlet) const
{
	// partial derivatives of the system function to pressures are constants
	df_dp_inlet = 1;
	df_dp_outlet = -1;

	// generic DQ approximation of partial derivative
	const double EPS = 1e-5; // in kg/s
	double f_eps = systemFunction(mdot+EPS, p_inlet, p_outlet);
	double f = systemFunction(mdot, p_inlet, p_outlet);
	df_dmdot = (f_eps - f)/EPS;
}


double HNPipeElement::pressureLossFriction(const double &mdot) const {
	// for negative mass flow: Reynolds number is positive, velocity and pressure loss are negative
	double fluidDensity = m_fluid->m_para[NANDRAD::HydraulicFluid::P_Density].value;
	double velocity = mdot / (fluidDensity * m_diameter * m_diameter * PI / 4);
	double Re = std::abs(velocity) * m_diameter / m_fluid->m_kinematicViscosity.m_values.value(*m_fluidTemperatureRef);
	double zeta = m_length / m_diameter * FrictionFactorSwamee(Re, m_diameter, m_roughness);
	return zeta * fluidDensity / 2 * std::abs(velocity) * velocity;
}



// *** HNFixedPressureLossCoeffElement ***

HNPressureLossCoeffElement::HNPressureLossCoeffElement(unsigned int flowElementId,
														const NANDRAD::HydraulicNetworkComponent &component,
														const NANDRAD::HydraulicFluid &fluid,
														const NANDRAD::HydraulicNetworkControlElement *controlElement):
	m_flowElementId(flowElementId),
	m_controlElement(controlElement)
{
	m_fluidDensity = fluid.m_para[NANDRAD::HydraulicFluid::P_Density].value;
	m_fluidHeatCapacity = fluid.m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value;
	m_zeta = component.m_para[NANDRAD::HydraulicNetworkComponent::P_PressureLossCoefficient].value;
	m_diameter = component.m_para[NANDRAD::HydraulicNetworkComponent::P_HydraulicDiameter].value;
}


void HNPressureLossCoeffElement::inputReferences(std::vector<InputReference> & inputRefs) const
{
	// in the case of control add heat exchange spline value to input references
	if(m_controlElement != nullptr) {
		switch (m_controlElement->m_controlledProperty) {

			case NANDRAD::HydraulicNetworkControlElement::CP_TemperatureDifference: {
				InputReference ref;
				ref.m_id = m_flowElementId;
				ref.m_referenceType = NANDRAD::ModelInputReference::MRT_NETWORKELEMENT;
				ref.m_name.m_name = "HeatExchangeHeatLoss";
				ref.m_required = true;
				inputRefs.push_back(ref);
			} break;
			default: ;
		}
	}
}


void HNPressureLossCoeffElement::setInputValueRefs(std::vector<const double*>::const_iterator & resultValueRefs)
{
	IBK_ASSERT(m_controlElement != nullptr && m_controlElement->m_controlledProperty ==
			NANDRAD::HydraulicNetworkControlElement::CP_TemperatureDifference);
	// now store the pointer returned for our input ref request and advance the iterator by one
	m_heatExchangeHeatLossRef = *(resultValueRefs++); // Heat exchange value reference
}


double HNPressureLossCoeffElement::systemFunction(double mdot, double p_inlet, double p_outlet) const {
	// for negative mass flow: dp is negative
	double area = PI / 4 * m_diameter * m_diameter;
	double velocity = mdot / (m_fluidDensity * area); // signed!
	double zeta = m_zeta;
	if (m_controlElement != nullptr) {
		zeta += zetaControlled(mdot); // no clipping necessary here, function zetaControlled() takes care of that!
	}
	double dp = zeta * m_fluidDensity / 2 * std::abs(velocity) * velocity;
	return p_inlet - p_outlet - dp;
}


double HNPressureLossCoeffElement::zetaControlled(double mdot) const {
	FUNCID(TNElementWithExternalHeatLoss::zetaControlled);

	// NOTE: When solving the hydraulic network equations, we already have a new value stored in
	//       m_heatExchangeValueRef. However, the m_heatLoss member is only set much later, when
	//       internalDerivatives() is called as part of ThermalNetworkBalanceModel::update().

	// calculate zetaControlled value for valve
	switch (m_controlElement->m_controlledProperty) {

		case NANDRAD::HydraulicNetworkControlElement::CP_TemperatureDifference: {

			IBK_ASSERT(m_heatExchangeHeatLossRef != nullptr);
			// compute current temperature for given heat loss and mass flux
			// Mind: access m_heatExchangeValueRef and not m_heatLoss here!
			const double temperatureDifference = *m_heatExchangeHeatLossRef/(mdot*m_fluidHeatCapacity);
			// if temperature difference is larger than the set point (negative e), we want maximum mass flux -> zeta = 0
			// if temperature difference is smaller than the set point (positive e), we decrease mass flow by increasing zeta
			const double e = m_controlElement->m_para[NANDRAD::HydraulicNetworkControlElement::P_TemperatureDifferenceSetpoint].value - temperatureDifference;
			double zetaControlled = 0.0;
			if (e <= 0) {
				zetaControlled = 0;
			}
			else {
				switch (m_controlElement->m_controllerType) {
					case NANDRAD::HydraulicNetworkControlElement::CT_PController: {
						// relate controller error e to zeta
						const double y = m_controlElement->m_para[NANDRAD::HydraulicNetworkControlElement::P_Kp].value * e;
						const double zetaMax = m_controlElement->m_maximumControllerResultValue;
						// apply clipping
						if (zetaMax > 0 && y > zetaMax)
							zetaControlled = zetaMax; // Note: this is problematic inside a Newton method without relaxation!
						else {
							zetaControlled = y;
						}
					} break;

					case NANDRAD::HydraulicNetworkControlElement::CT_PIController:
						throw IBK::Exception("PIController not implemented, yet.", FUNC_ID);

					case NANDRAD::HydraulicNetworkControlElement::NUM_CT: break; // just to make compiler happy
				}

			}
			return zetaControlled;
		}

		case NANDRAD::HydraulicNetworkControlElement::CP_ThermostatValue: // not a possible combination
		case NANDRAD::HydraulicNetworkControlElement::NUM_CP: ; // nothing todo - we return 0
	}
//	IBK::IBK_Message(IBK::FormatString("zeta = %1, m_heatLoss = %4 W, dT = %2 K, mdot = %3 kg/s, heatExchangeValueRef = %5 W\n")
//					 .arg(m_zetaControlled).arg(m_temperatureDifference).arg(mdot).arg(m_heatLoss).arg(*m_heatExchangeValueRef));
	return 0.0;
}



void HNPressureLossCoeffElement::partials(double mdot, double p_inlet, double p_outlet,
							 double & df_dmdot, double & df_dp_inlet, double & df_dp_outlet) const
{
	// partial derivatives of the system function to pressures are constants
	df_dp_inlet = 1;
	df_dp_outlet = -1;
	// generic DQ approximation of partial derivative
	const double EPS = 1e-5; // in kg/s
	double f_eps = systemFunction(mdot+EPS, p_inlet, p_outlet);
	double f = systemFunction(mdot, p_inlet, p_outlet);
	df_dmdot = (f_eps - f)/EPS;
}


// *** HNConstantPressurePump ***

HNConstantPressurePump::HNConstantPressurePump(unsigned int id, const NANDRAD::HydraulicNetworkComponent &component)  :
	m_id(id)
{
	m_pressureHead = component.m_para[NANDRAD::HydraulicNetworkComponent::P_PressureHead].value;
}


double HNConstantPressurePump::systemFunction(double /*mdot*/, double p_inlet, double p_outlet) const {
	if (m_pressureHeadRef != nullptr)
		return p_inlet - p_outlet + *m_pressureHeadRef;
	else
		return p_inlet - p_outlet + m_pressureHead;
}


void HNConstantPressurePump::partials(double /*mdot*/, double /*p_inlet*/, double /*p_outlet*/,
							 double & df_dmdot, double & df_dp_inlet, double & df_dp_outlet) const
{
	// partial derivatives of the system function to pressures are constants
	df_dp_inlet = 1;
	df_dp_outlet = -1;
	df_dmdot = 0;
}


void HNConstantPressurePump::inputReferences(std::vector<InputReference> & inputRefs) const {
	InputReference inputRef;
	inputRef.m_referenceType = NANDRAD::ModelInputReference::MRT_NETWORKELEMENT;
	inputRef.m_name = std::string("PumpPressureHead");
	inputRef.m_required = false;
	inputRef.m_id = m_id;
	inputRefs.push_back(inputRef);
}


void HNConstantPressurePump::setInputValueRefs(std::vector<const double *>::const_iterator & resultValueRefIt) {
	m_pressureHeadRef = *resultValueRefIt; // optional, may be nullptr
	++resultValueRefIt;
}


// *** HNConstantMassFluxPump ***

HNConstantMassFluxPump::HNConstantMassFluxPump(unsigned int id, const NANDRAD::HydraulicNetworkComponent & component) :
	m_id(id)
{
	m_massFlux = component.m_para[NANDRAD::HydraulicNetworkComponent::P_MassFlux].value;
}


double HNConstantMassFluxPump::systemFunction(double mdot, double /*p_inlet*/, double /*p_outlet*/) const {
	if (m_massFluxRef != nullptr)
		return mdot - *m_massFluxRef;
	else
		return mdot - m_massFlux;
}


void HNConstantMassFluxPump::partials(double /*mdot*/, double /*p_inlet*/, double /*p_outlet*/,
									  double & df_dmdot, double & df_dp_inlet, double & df_dp_outlet) const
{
	df_dmdot = 1;
	df_dp_inlet = 0;
	df_dp_outlet = 0;
}


void HNConstantMassFluxPump::inputReferences(std::vector<InputReference> & inputRefs) const {
	// we allow an optional PumpMassFlux input reference to override our parameter
	InputReference inputRef;
	inputRef.m_referenceType = NANDRAD::ModelInputReference::MRT_NETWORKELEMENT;
	inputRef.m_name = std::string("PumpMassFlux");
	inputRef.m_required = false;
	inputRef.m_id = m_id;
	inputRefs.push_back(inputRef);
}


void HNConstantMassFluxPump::setInputValueRefs(std::vector<const double *>::const_iterator & resultValueRefIt) {
	m_massFluxRef = *resultValueRefIt; // optional, may be nullptr
	++resultValueRefIt;
}


} // namespace NANDRAD_MODEL
