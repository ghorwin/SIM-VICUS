#include "NM_ThermalNetworkFlowElements.h"
#include "NM_Physics.h"

#include "NANDRAD_HydraulicFluid.h"
#include "NANDRAD_HydraulicNetworkElement.h"
#include "NANDRAD_HydraulicNetworkPipeProperties.h"
#include "NANDRAD_HydraulicNetworkComponent.h"

#include "numeric"



namespace NANDRAD_MODEL {

// *** TNSimplePipeElement ***

TNSimplePipeElement::TNSimplePipeElement(const NANDRAD::HydraulicNetworkElement & elem,
							 const NANDRAD::HydraulicNetworkComponent & comp,
							const NANDRAD::HydraulicNetworkPipeProperties & pipePara,
							const NANDRAD::HydraulicFluid & fluid,
							const double &TExt)
{
	m_length = elem.m_para[NANDRAD::HydraulicNetworkElement::P_Length].value;
	m_innerDiameter = pipePara.m_para[NANDRAD::HydraulicNetworkPipeProperties::P_PipeInnerDiameter].value;
	m_outerDiameter = pipePara.m_para[NANDRAD::HydraulicNetworkPipeProperties::P_PipeOuterDiameter].value;
	m_UValuePipeWall = pipePara.m_para[NANDRAD::HydraulicNetworkPipeProperties::P_UValuePipeWall].value;
	m_outerHeatTransferCoefficient =
			comp.m_para[NANDRAD::HydraulicNetworkComponent::P_ExternalHeatTransferCoefficient].value;
	// compute fluid volume
	m_fluidVolume = PI/4. * m_innerDiameter * m_innerDiameter * m_length;

	// copy fluid properties
	m_fluidDensity = fluid.m_para[NANDRAD::HydraulicFluid::P_Density].value;
	m_fluidHeatCapacity = fluid.m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value;
	m_fluidConductivity = fluid.m_para[NANDRAD::HydraulicFluid::P_Conductivity].value;
	m_fluidViscosity = fluid.m_kinematicViscosity.m_values;
	// set reference to external temperature
	m_externalTemperatureRef = &TExt;
}


void TNSimplePipeElement::setInflowTemperature(double Tinflow) {
	m_inflowTemperature = Tinflow;

	// check heat transfer type
	if (m_heatExchangeType != (int) NANDRAD::HydraulicNetworkComponent::HT_HeatFluxConstant &&
		m_heatExchangeType != (int) NANDRAD::HydraulicNetworkComponent::HT_HeatFluxDataFile)
	{

		// calculate inner heat transfer coefficient
		const double velocity = std::fabs(m_massFlux)/(m_fluidVolume * m_fluidDensity);
		const double viscosity = m_fluidViscosity.value(m_meanTemperature);
		const double reynolds = ReynoldsNumber(velocity, viscosity, m_innerDiameter);
		const double prandtl = PrandtlNumber(viscosity, m_fluidHeatCapacity, m_fluidConductivity, m_fluidDensity);
		double nusselt = NusseltNumber(reynolds, prandtl, m_length, m_innerDiameter);
		double innerHeatTransferCoefficient = nusselt * m_fluidConductivity /
												m_innerDiameter;

		// calculate heat transfer

		// UAValueTotal has W/K, basically the u-value per length pipe (including transfer coefficients) x pipe length.
		const double UAValueTotal = m_length / (
					  1.0/(innerHeatTransferCoefficient * m_innerDiameter * PI
					+ 1.0/(m_outerHeatTransferCoefficient * m_outerDiameter * PI)
					+ 1.0/m_UValuePipeWall )
			);

		// Q in [W] = DeltaT * UAValueTotal
		const double ambientTemperature = *m_externalTemperatureRef;
		// calculate heat loss with given parameters
		m_heatLoss = UAValueTotal * (m_meanTemperature - ambientTemperature);
	}
}


// *** TNStaticPipeElement ***

TNStaticPipeElement::TNStaticPipeElement(const NANDRAD::HydraulicNetworkElement & elem,
							 const NANDRAD::HydraulicNetworkComponent & comp,
							const NANDRAD::HydraulicNetworkPipeProperties & pipePara,
							const NANDRAD::HydraulicFluid & fluid,
							const double &TExt)
{
	m_length = elem.m_para[NANDRAD::HydraulicNetworkElement::P_Length].value;
	m_innerDiameter = pipePara.m_para[NANDRAD::HydraulicNetworkPipeProperties::P_PipeInnerDiameter].value;
	m_outerDiameter = pipePara.m_para[NANDRAD::HydraulicNetworkPipeProperties::P_PipeOuterDiameter].value;
	m_UValuePipeWall = pipePara.m_para[NANDRAD::HydraulicNetworkPipeProperties::P_UValuePipeWall].value;
	m_outerHeatTransferCoefficient =
			comp.m_para[NANDRAD::HydraulicNetworkComponent::P_ExternalHeatTransferCoefficient].value;
	// compute fluid volume
	m_fluidVolume = 0.01;

	// copy fluid properties
	m_fluidDensity = fluid.m_para[NANDRAD::HydraulicFluid::P_Density].value;
	m_fluidHeatCapacity = fluid.m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value;
	m_fluidConductivity = fluid.m_para[NANDRAD::HydraulicFluid::P_Conductivity].value;
	m_fluidViscosity = fluid.m_kinematicViscosity.m_values;
	// set reference to external temperature
	m_externalTemperatureRef = &TExt;
}


void TNStaticPipeElement::setInflowTemperature(double Tinflow) {
	m_inflowTemperature = Tinflow;

	// check heat transfer type
	if (m_heatExchangeType != (int) NANDRAD::HydraulicNetworkComponent::HT_HeatFluxConstant &&
		m_heatExchangeType != (int) NANDRAD::HydraulicNetworkComponent::HT_HeatFluxDataFile)
	{

		// calculate inner heat transfer coefficient
		const double velocity = std::fabs(m_massFlux)/(m_fluidVolume * m_fluidDensity);
		const double viscosity = m_fluidViscosity.value(m_meanTemperature);
		const double reynolds = ReynoldsNumber(velocity, viscosity, m_innerDiameter);
		const double prandtl = PrandtlNumber(viscosity, m_fluidHeatCapacity, m_fluidConductivity, m_fluidDensity);
		double nusselt = NusseltNumber(reynolds, prandtl, m_length, m_innerDiameter);
		double innerHeatTransferCoefficient = nusselt * m_fluidConductivity /
												m_innerDiameter;

		// calculate heat transfer

		// UAValueTotal has W/K, basically the u-value per length pipe (including transfer coefficients) x pipe length.
		const double UAValueTotal = m_length / (
					  1.0/(innerHeatTransferCoefficient * m_innerDiameter * PI
					+ 1.0/(m_outerHeatTransferCoefficient * m_outerDiameter * PI)
					+ 1.0/m_UValuePipeWall )
			);

		// Q in [W] = DeltaT * UAValueTotal
		const double ambientTemperature = *m_externalTemperatureRef;
		// calculate heat loss with given (for steady state model we interpret mean temperature as
		// outflow temperature and calculate a corresponding heat flux)
		m_heatLoss = m_massFlux * m_fluidHeatCapacity *
				(m_inflowTemperature - ambientTemperature) *
				(1. - std::exp(-UAValueTotal / (std::fabs(m_massFlux) * m_fluidHeatCapacity )));
	}
}


// *** DynamicPipeElement ***

TNDynamicPipeElement::TNDynamicPipeElement(const NANDRAD::HydraulicNetworkElement & elem,
							 const NANDRAD::HydraulicNetworkComponent & comp,
							const NANDRAD::HydraulicNetworkPipeProperties & pipePara,
							const NANDRAD::HydraulicFluid & fluid,
							const double &TExt)
{
	m_length = elem.m_para[NANDRAD::HydraulicNetworkElement::P_Length].value;
	m_innerDiameter = pipePara.m_para[NANDRAD::HydraulicNetworkPipeProperties::P_PipeInnerDiameter].value;
	m_outerDiameter = pipePara.m_para[NANDRAD::HydraulicNetworkPipeProperties::P_PipeOuterDiameter].value;
	m_UValuePipeWall = pipePara.m_para[NANDRAD::HydraulicNetworkPipeProperties::P_UValuePipeWall].value;
	m_outerHeatTransferCoefficient =
			comp.m_para[NANDRAD::HydraulicNetworkComponent::P_ExternalHeatTransferCoefficient].value;
	// calculate fluid volume inside the pipe
	m_fluidVolume = PI/4. * m_innerDiameter * m_innerDiameter * m_length;
	// copy fluid properties
	m_fluidDensity = fluid.m_para[NANDRAD::HydraulicFluid::P_Density].value;
	m_fluidHeatCapacity = fluid.m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value;
	m_fluidConductivity = fluid.m_para[NANDRAD::HydraulicFluid::P_Conductivity].value;
	m_fluidViscosity = fluid.m_kinematicViscosity.m_values;

	// caluclate discretization
	double minDiscLength = comp.m_para[NANDRAD::HydraulicNetworkComponent::P_PipeMaxDiscretizationWidth].value;
	// in case given discretization length is larger than pipe length:
	// set discretization length to pipe length, so we have just one volume
	if (minDiscLength > m_length)
		minDiscLength = m_length;

	// claculte number of discretization elements
	m_nVolumes = (unsigned int) (m_length/minDiscLength);
	// resize all vectors
	m_temperatures.resize(m_nVolumes, 273.15);
	m_heatLosses.resize(m_nVolumes, 0.0);

	// calculate segment specific quantities
	m_discLength = m_length/(double) m_nVolumes;
	m_discVolume = m_fluidVolume/(double) m_nVolumes;
	// set reference to external temperature
	m_externalTemperatureRef = &TExt;
}


void TNDynamicPipeElement::setInitialTemperature(double T0) {
	// use standard implementation
	ThermalNetworkAbstractFlowElementWithHeatLoss::setInitialTemperature(T0);
	// fill vector valued quantiteis
	std::fill(m_temperatures.begin(), m_temperatures.end(), T0);
}


void TNDynamicPipeElement::setInflowTemperature(double Tinflow) {
	m_inflowTemperature = Tinflow;

	// check heat transfer type
	if (m_heatExchangeType != (int) NANDRAD::HydraulicNetworkComponent::HT_HeatFluxConstant &&
		m_heatExchangeType != (int) NANDRAD::HydraulicNetworkComponent::HT_HeatFluxDataFile)
	{

		m_heatLoss = 0.0;

		// assume constant heat transfer coefficient along pipe, using average temperature
		const double velocity = std::fabs(m_massFlux)/(m_fluidVolume * m_fluidDensity);
		const double viscosity = m_fluidViscosity.value(m_meanTemperature);
		const double reynolds = ReynoldsNumber(velocity, viscosity, m_innerDiameter);
		const double prandtl = PrandtlNumber(viscosity, m_fluidHeatCapacity, m_fluidConductivity, m_fluidDensity);
		double nusselt = NusseltNumber(reynolds, prandtl, m_length, m_innerDiameter);
		double innerHeatTransferCoefficient = nusselt * m_fluidConductivity / m_innerDiameter;

		// UAValueTotal has W/K, basically the u-value per length pipe (including transfer coefficients) x pipe length.
		// see documentation above
		const double UAValue = m_discLength / (
					  1.0/(innerHeatTransferCoefficient * m_innerDiameter * PI
					+ 1.0/(m_outerHeatTransferCoefficient * m_outerDiameter * PI)
					+ 1.0/m_UValuePipeWall )
			);

		const double ambientTemperature = *m_externalTemperatureRef;
		for(unsigned int i = 0; i < m_nVolumes; ++i) {
			// calculate heat loss with given parameters
			m_heatLosses[i] = UAValue * (m_temperatures[i] - ambientTemperature);
			// sum up heat losses
			m_heatLoss += m_heatLosses[i];
		}
	}
}


void TNDynamicPipeElement::initialInternalStates(double * y0) {
	// copy internal states
	for(unsigned int i = 0; i < m_nVolumes; ++i)
		y0[i] = m_temperatures[i] * m_fluidHeatCapacity * m_fluidDensity * m_discVolume ;
}


void TNDynamicPipeElement::setInternalStates(const double * y) {
	double temp = 0.0;
	// calculate specific enthalpy
	for(unsigned int i = 0; i < m_nVolumes; ++i) {
		m_temperatures[i] = y[i] / ( m_discVolume * m_fluidDensity * m_fluidHeatCapacity);
		temp += m_temperatures[i];
	}
	m_meanTemperature = temp/m_nVolumes;
}


void TNDynamicPipeElement::internalDerivatives(double * ydot) {
	// heat fluxes into the fluid and enthalpy change are heat sources
	if (m_massFlux >= 0.0) {
		// first element copies boundary conditions

		ydot[0] = -m_heatLosses[0] + m_massFlux * m_fluidHeatCapacity * (m_inflowTemperature  - m_temperatures[0]);
		for(unsigned int i = 1; i < m_nVolumes; ++i) {
			ydot[i] = -m_heatLosses[i] + m_massFlux * m_fluidHeatCapacity * (m_temperatures[i - 1] - m_temperatures[i]);
		}
	}
	else { // m_massFlux < 0
		// last element copies boundary conditions
		ydot[m_nVolumes - 1] = -m_heatLosses[m_nVolumes - 1] + m_massFlux * m_fluidHeatCapacity * (m_temperatures[m_nVolumes - 1] - m_inflowTemperature);
		for(unsigned int i = 0; i < m_nVolumes - 1; ++i) {
			ydot[i] = -m_heatLosses[i] + m_massFlux * m_fluidHeatCapacity * (m_temperatures[i] - m_temperatures[i + 1]);
		}
	}
}


double TNDynamicPipeElement::outflowTemperature() {
	if (m_massFlux >= 0)
		return m_temperatures[m_nVolumes-1];
	else
		return m_temperatures[0];
}


void TNDynamicPipeElement::dependencies(const double *ydot, const double *y,
										const double *mdot, const double* TInflowLeft, const double*TInflowRight,
										std::vector<std::pair<const double *, const double *> > & resultInputDependencies) const {

	// set dependency to inlet and outlet enthalpy
	resultInputDependencies.push_back(std::make_pair(TInflowLeft, y) );
	resultInputDependencies.push_back(std::make_pair(TInflowRight, y + nInternalStates() - 1) );
	resultInputDependencies.push_back(std::make_pair(ydot, TInflowLeft) );
	resultInputDependencies.push_back(std::make_pair(ydot + nInternalStates() - 1, TInflowRight) );


	for(unsigned int n = 0; n < nInternalStates(); ++n) {
		// set dependency to mean temperatures
		resultInputDependencies.push_back(std::make_pair(&m_meanTemperature, y + n) );

		// set depedency to ydot
		// heat balance per default sums up heat fluxes and entahpy flux differences through the pipe
		if(n > 0)
			resultInputDependencies.push_back(std::make_pair(ydot + n, y + n - 1) );

		resultInputDependencies.push_back(std::make_pair(ydot + n, y + n) );

		if(n < nInternalStates() - 1)
			resultInputDependencies.push_back(std::make_pair(ydot + n, y + n + 1) );

		resultInputDependencies.push_back(std::make_pair(ydot + n, mdot) );
		resultInputDependencies.push_back(std::make_pair(ydot + n, m_externalTemperatureRef));

		// set dependeny to Qdot
		resultInputDependencies.push_back(std::make_pair(&m_heatLoss, y + n) );
	}

	// set dependeny to Qdot
	resultInputDependencies.push_back(std::make_pair(&m_heatLoss, mdot) );
	resultInputDependencies.push_back(std::make_pair(&m_heatLoss, m_externalTemperatureRef) );
}


// *** DynamicAdiabaticPipeElement ***

TNDynamicAdiabaticPipeElement::TNDynamicAdiabaticPipeElement(const NANDRAD::HydraulicNetworkElement & elem,
							 const NANDRAD::HydraulicNetworkComponent & comp,
							const NANDRAD::HydraulicNetworkPipeProperties & pipePara,
							const NANDRAD::HydraulicFluid & fluid)
{
	double length = elem.m_para[NANDRAD::HydraulicNetworkElement::P_Length].value;
	double innerDiameter = pipePara.m_para[NANDRAD::HydraulicNetworkPipeProperties::P_PipeInnerDiameter].value;

	// caluclate discretization
	double minDiscLength = comp.m_para[NANDRAD::HydraulicNetworkComponent::P_PipeMaxDiscretizationWidth].value;
	// in case given discretization length is larger than pipe length:
	// set discretization length to pipe length, so we have just one volume
	if (minDiscLength > length)
		minDiscLength = length;

	// claculte number of discretization elements
	m_nVolumes = (unsigned int) (length/minDiscLength);
	// resize all vectors
	m_temperatures.resize(m_nVolumes, 273.15);
	// calculate fluid volume inside the pipe
	m_fluidVolume = PI/4. * innerDiameter * innerDiameter * length;
	// copy fluid properties
	m_fluidDensity = fluid.m_para[NANDRAD::HydraulicFluid::P_Density].value;
	m_fluidHeatCapacity = fluid.m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value;

	// calculate segment specific quantities
	m_discVolume = m_fluidVolume/(double) m_nVolumes;
}


void TNDynamicAdiabaticPipeElement::setInitialTemperature(double T0) {
	// use standard implementation
	ThermalNetworkAbstractFlowElement::setInitialTemperature(T0);
	// fill vector valued quantiteis
	std::fill(m_temperatures.begin(), m_temperatures.end(), T0);
}


void TNDynamicAdiabaticPipeElement::initialInternalStates(double * y0) {
	// copy internal states
	for(unsigned int i = 0; i < m_nVolumes; ++i)
		y0[i] = m_temperatures[i] * m_fluidHeatCapacity * m_fluidDensity * m_discVolume ;
}


void TNDynamicAdiabaticPipeElement::setInternalStates(const double * y) {
	double temp = 0.0;
	// calculate specific enthalpy
	for(unsigned int i = 0; i < m_nVolumes; ++i) {
		m_temperatures[i] = y[i] / ( m_discVolume * m_fluidDensity * m_fluidHeatCapacity);
		temp += m_temperatures[i];
	}
	m_meanTemperature = temp/m_nVolumes;
}


void TNDynamicAdiabaticPipeElement::internalDerivatives(double * ydot) {
	if (m_massFlux >= 0.0) {
		// first element copies boundary conditions
		ydot[0] = m_massFlux * m_fluidHeatCapacity * (m_inflowTemperature  - m_temperatures[0]);
		for(unsigned int i = 1; i < m_nVolumes; ++i) {
			ydot[i] = m_massFlux * m_fluidHeatCapacity * (m_temperatures[i - 1] - m_temperatures[i]);
		}
	}
	else { // m_massFlux < 0
		// last element copies boundary conditions
		ydot[m_nVolumes - 1] = m_massFlux * m_fluidHeatCapacity * (m_temperatures[m_nVolumes - 1] - m_inflowTemperature);
		for(unsigned int i = 0; i < m_nVolumes - 1; ++i) {
			ydot[i] = m_massFlux * m_fluidHeatCapacity * (m_temperatures[i] - m_temperatures[i + 1]);
		}
	}
}


double TNDynamicAdiabaticPipeElement::outflowTemperature() {
	if (m_massFlux >= 0)
		return m_temperatures[m_nVolumes-1];
	else
		return m_temperatures[0];
}


void TNDynamicAdiabaticPipeElement::dependencies(const double *ydot, const double *y,
										const double *mdot, const double* TInflowLeft, const double*TInflowRight,
										std::vector<std::pair<const double *, const double *> > & resultInputDependencies) const {

	// set dependency to inlet and outlet enthalpy
	resultInputDependencies.push_back(std::make_pair(TInflowLeft, y) );
	resultInputDependencies.push_back(std::make_pair(TInflowRight, y + nInternalStates() - 1) );
	resultInputDependencies.push_back(std::make_pair(ydot, TInflowLeft) );
	resultInputDependencies.push_back(std::make_pair(ydot + nInternalStates() - 1, TInflowRight) );


	for(unsigned int n = 0; n < nInternalStates(); ++n) {
		// set dependency to mean temperatures
		resultInputDependencies.push_back(std::make_pair(&m_meanTemperature, y + n) );

		// heat balance per default sums up heat fluxes and entahpy flux differences through the pipe
		if(n > 0)
			resultInputDependencies.push_back(std::make_pair(ydot + n, y + n - 1) );

		resultInputDependencies.push_back(std::make_pair(ydot + n, y + n) );

		if(n < nInternalStates() - 1)
			resultInputDependencies.push_back(std::make_pair(ydot + n, y + n + 1) );

		// set depedency to mdot
		resultInputDependencies.push_back(std::make_pair(ydot + n, mdot) );
	}
}



// *** PumpWithPerformanceLoss ***

TNPumpWithPerformanceLoss::TNPumpWithPerformanceLoss(
							 const NANDRAD::HydraulicFluid & fluid,
							const NANDRAD::HydraulicNetworkComponent & comp,
							const double &pRef)
{
	// copy component properties
	m_fluidVolume = comp.m_para[NANDRAD::HydraulicNetworkComponent::P_Volume].value;
	m_pumpEfficiency = comp.m_para[NANDRAD::HydraulicNetworkComponent::P_PumpEfficiency].value;
	m_motorEfficiency = comp.m_para[NANDRAD::HydraulicNetworkComponent::P_MotorEfficiency].value;
	// copy fluid properties
	m_fluidDensity = fluid.m_para[NANDRAD::HydraulicFluid::P_Density].value;
	m_fluidHeatCapacity = fluid.m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value;
	// copy references to hydraulic simulation
	m_pressureHeadRef = &pRef;
}


void TNPumpWithPerformanceLoss::setInflowTemperature(double Tinflow) {
	m_inflowTemperature = Tinflow;

	// mechanical power of pump is pressure head times volumetric flow
	// Pa * m3/s = N/m2 * m3/s = N*m/s

	// calculate pump performance
	double Pmechanical = m_massFlux/m_fluidDensity * (*m_pressureHeadRef);

	// efficiency is defined as portion of total electrical power used for mechanical
	// Pelectrical * m_pumpEfficiency * m_motorEfficiency = Pmechanical
	double Pelectrical = Pmechanical/(m_pumpEfficiency * m_motorEfficiency);
	// calculate heat flux into fluid
	m_heatLoss = - (Pelectrical- Pmechanical);
}




// *** TNHeatPumpIdealCarnot ***

TNHeatPumpIdealCarnot::TNHeatPumpIdealCarnot(const NANDRAD::HydraulicFluid & fluid,
											 const NANDRAD::HydraulicNetworkComponent & comp,
											 const double &QExt)
{
	m_fluidVolume = comp.m_para[NANDRAD::HydraulicNetworkComponent::P_Volume].value;
	m_condenserMeanTemperature = comp.m_para[NANDRAD::HydraulicNetworkComponent::P_CondenserMeanTemperature].value;
	m_carnotEfficiency = comp.m_para[NANDRAD::HydraulicNetworkComponent::P_CarnotEfficiency].value;

	// copy fluid properties
	m_fluidDensity = fluid.m_para[NANDRAD::HydraulicFluid::P_Density].value;
	m_fluidHeatCapacity = fluid.m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value;

	// set reference to external heat loss
	m_externalHeatLossRef = &QExt;
}

void TNHeatPumpIdealCarnot::setInflowTemperature(double Tinflow)
{
	// copy ionflow temperature
	m_inflowTemperature = Tinflow;
	// TODO Hauke: use mean evaporator temperature instead of evaporator inlet temperature?
	const double COPMax = m_condenserMeanTemperature / (m_condenserMeanTemperature - Tinflow);
	const double COP = m_carnotEfficiency * COPMax;
	m_heatLoss = *m_externalHeatLossRef * (COP - 1)/COP;
}

// *** AdiabaticElement ***

TNAdiabaticElement::TNAdiabaticElement(const NANDRAD::HydraulicFluid & fluid,
									double fluidVolume) {
	// copy fluid parameters
	m_fluidHeatCapacity = fluid.m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value;
	m_fluidDensity = fluid.m_para[NANDRAD::HydraulicFluid::P_Density].value;
	// store fluid volume
	m_fluidVolume = fluidVolume;
}


// *** ElementWithExternalHeatLoss ***

TNElementWithExternalHeatLoss::TNElementWithExternalHeatLoss(
							 const NANDRAD::HydraulicFluid & fluid,
							 double fluidVolume,
							 const double &QExt)
{
	m_fluidVolume = fluidVolume;
	// copy fluid properties
	m_fluidDensity = fluid.m_para[NANDRAD::HydraulicFluid::P_Density].value;
	m_fluidHeatCapacity = fluid.m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value;
	// set reference to external heat loss
	m_externalHeatLossRef = &QExt;
}


void TNElementWithExternalHeatLoss::internalDerivatives(double * ydot) {
	// set heat loss
	m_heatLoss = *m_externalHeatLossRef;
	// use basic routine
	ThermalNetworkAbstractFlowElementWithHeatLoss::internalDerivatives(ydot);
}


} // namespace NANDRAD_MODEL
