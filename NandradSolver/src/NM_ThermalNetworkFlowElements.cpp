#include "NM_ThermalNetworkFlowElements.h"
#include "NM_Physics.h"

#include "NANDRAD_HydraulicFluid.h"
#include "NANDRAD_HydraulicNetworkElement.h"
#include "NANDRAD_HydraulicNetworkPipeProperties.h"
#include "NANDRAD_HydraulicNetworkComponent.h"

#include "numeric"

#include "IBK_messages.h"


namespace NANDRAD_MODEL {

// *** TNSimplePipeElement ***

TNSimplePipeElement::TNSimplePipeElement(const NANDRAD::HydraulicNetworkElement & elem,
							 const NANDRAD::HydraulicNetworkComponent & /*comp*/,
							const NANDRAD::HydraulicNetworkPipeProperties & pipePara,
							const NANDRAD::HydraulicFluid & fluid)
{
	m_length = elem.m_para[NANDRAD::HydraulicNetworkElement::P_Length].value;
	m_innerDiameter = pipePara.m_para[NANDRAD::HydraulicNetworkPipeProperties::P_PipeInnerDiameter].value;
	m_outerDiameter = pipePara.m_para[NANDRAD::HydraulicNetworkPipeProperties::P_PipeOuterDiameter].value;
	m_UValuePipeWall = pipePara.m_para[NANDRAD::HydraulicNetworkPipeProperties::P_UValuePipeWall].value;
	m_outerHeatTransferCoefficient =
			elem.m_heatExchange.m_para[NANDRAD::HydraulicNetworkHeatExchange::P_ExternalHeatTransferCoefficient].value;
	// copy number of pipes
	m_nParallelPipes = (unsigned int) elem.m_intPara[NANDRAD::HydraulicNetworkElement::IP_NumberParallelPipes].value;
	// compute fluid volume
	m_fluidCrossSection = PI/4. * m_innerDiameter * m_innerDiameter * m_nParallelPipes;
	m_fluidVolume = m_fluidCrossSection * m_length;

	// copy fluid properties
	m_fluidDensity = fluid.m_para[NANDRAD::HydraulicFluid::P_Density].value;
	m_fluidHeatCapacity = fluid.m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value;
	m_fluidConductivity = fluid.m_para[NANDRAD::HydraulicFluid::P_Conductivity].value;
	m_fluidViscosity = fluid.m_kinematicViscosity.m_values;
}


void TNSimplePipeElement::setInflowTemperature(double Tinflow) {
	m_inflowTemperature = Tinflow;

	// check heat transfer type
	m_volumeFlow = std::fabs(m_massFlux)/m_fluidDensity; // m3/s !!! unit conversion is done when writing outputs

	// note: velcoty is caluclated for a single pipe (but mass flux interpreted as flux through all parallel pipes
	m_velocity = m_volumeFlow/m_fluidCrossSection;

	m_viscosity = m_fluidViscosity.value(m_meanTemperature);
	m_reynolds = ReynoldsNumber(m_velocity, m_viscosity, m_innerDiameter);
	m_prandtl = PrandtlNumber(m_viscosity, m_fluidHeatCapacity, m_fluidConductivity, m_fluidDensity);
	m_nusselt = NusseltNumber(m_reynolds, m_prandtl, m_length, m_innerDiameter);
	// calculate inner heat transfer coefficient
	double innerHeatTransferCoefficient = m_nusselt * m_fluidConductivity /
											m_innerDiameter;

	if (m_outerHeatTransferCoefficient == 0.) {
		// UAValueTotal has W/K, basically the u-value per length pipe (including transfer coefficients) x pipe length.
		m_thermalTransmittance = m_length /
					  ( 1.0 / (innerHeatTransferCoefficient * m_innerDiameter * PI ) + 1.0/m_UValuePipeWall ) ;
	}
	else {
		// UAValueTotal has W/K, basically the u-value per length pipe (including transfer coefficients) x pipe length.
		m_thermalTransmittance = m_length /
					( 1.0 / (innerHeatTransferCoefficient * m_innerDiameter * PI )
					+ 1.0 / (m_outerHeatTransferCoefficient * m_outerDiameter * PI)
					+ 1.0 / m_UValuePipeWall ) ;
	}

	IBK_ASSERT(m_heatExchangeValueRef != nullptr);
	const double externalTemperature = *m_heatExchangeValueRef;
	// calculate heat loss with given parameters
	// Q in [W] = DeltaT * UAValueTotal
	m_heatLoss = m_thermalTransmittance * (m_meanTemperature - externalTemperature) * m_nParallelPipes;
}


#ifdef STATIC_PIPE_MODEL_ENABLED

// *** TNStaticPipeElement ***

TNStaticPipeElement::TNStaticPipeElement(const NANDRAD::HydraulicNetworkElement & elem,
							 const NANDRAD::HydraulicNetworkComponent & comp,
							const NANDRAD::HydraulicNetworkPipeProperties & pipePara,
							const NANDRAD::HydraulicFluid & fluid)
{
	m_length = elem.m_para[NANDRAD::HydraulicNetworkElement::P_Length].value;
	m_innerDiameter = pipePara.m_para[NANDRAD::HydraulicNetworkPipeProperties::P_PipeInnerDiameter].value;
	m_outerDiameter = pipePara.m_para[NANDRAD::HydraulicNetworkPipeProperties::P_PipeOuterDiameter].value;
	m_UValuePipeWall = pipePara.m_para[NANDRAD::HydraulicNetworkPipeProperties::P_UValuePipeWall].value;
	m_outerHeatTransferCoefficient =
			elem.m_heatExchange.m_para[NANDRAD::HydraulicNetworkHeatExchange::P_ExternalHeatTransferCoefficient].value;
	// compute fluid volume
	m_fluidVolume = 0.01;

	// copy fluid properties
	m_fluidDensity = fluid.m_para[NANDRAD::HydraulicFluid::P_Density].value;
	m_fluidHeatCapacity = fluid.m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value;
	m_fluidConductivity = fluid.m_para[NANDRAD::HydraulicFluid::P_Conductivity].value;
	m_fluidViscosity = fluid.m_kinematicViscosity.m_values;
}


void TNStaticPipeElement::setInflowTemperature(double Tinflow) {
	m_inflowTemperature = Tinflow;

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
	IBK_ASSERT(m_externalTemperatureRef != nullptr);
	const double externalTemperature = *m_externalTemperatureRef;
	// calculate heat loss with given (for steady state model we interpret mean temperature as
	// outflow temperature and calculate a corresponding heat flux)
	m_heatLoss = m_massFlux * m_fluidHeatCapacity *
			(m_inflowTemperature - externalTemperature) *
			(1. - std::exp(-UAValueTotal / (std::fabs(m_massFlux) * m_fluidHeatCapacity )) );
}
#endif // STATIC_PIPE_MODEL_ENABLED



// *** DynamicPipeElement ***

TNDynamicPipeElement::TNDynamicPipeElement(const NANDRAD::HydraulicNetworkElement & elem,
							const NANDRAD::HydraulicNetworkComponent & comp,
							const NANDRAD::HydraulicNetworkPipeProperties & pipePara,
							const NANDRAD::HydraulicFluid & fluid)
{
	m_length = elem.m_para[NANDRAD::HydraulicNetworkElement::P_Length].value;
	m_innerDiameter = pipePara.m_para[NANDRAD::HydraulicNetworkPipeProperties::P_PipeInnerDiameter].value;
	m_outerDiameter = pipePara.m_para[NANDRAD::HydraulicNetworkPipeProperties::P_PipeOuterDiameter].value;
	m_UValuePipeWall = pipePara.m_para[NANDRAD::HydraulicNetworkPipeProperties::P_UValuePipeWall].value;
	m_outerHeatTransferCoefficient =
			elem.m_heatExchange.m_para[NANDRAD::HydraulicNetworkHeatExchange::P_ExternalHeatTransferCoefficient].value;
	// copy number of pipes
	m_nParallelPipes = (unsigned int) elem.m_intPara[NANDRAD::HydraulicNetworkElement::IP_NumberParallelPipes].value;
	// calculate fluid volume inside the pipe
	m_fluidCrossSection = PI/4. * m_innerDiameter * m_innerDiameter * m_nParallelPipes;
	m_fluidVolume = m_fluidCrossSection * m_length;
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
}


void TNDynamicPipeElement::setInitialTemperature(double T0) {
	// use standard implementation
	ThermalNetworkAbstractFlowElementWithHeatLoss::setInitialTemperature(T0);
	// fill vector valued quantiteis
	std::fill(m_temperatures.begin(), m_temperatures.end(), T0);
}


void TNDynamicPipeElement::setInflowTemperature(double Tinflow) {
	m_inflowTemperature = Tinflow;

	m_volumeFlow = std::fabs(m_massFlux)/m_fluidDensity; // m3/s !!! unit conversion is done when writing outputs
	// note: velcoty is caluclated for a single pipe (but mass flux interpreted as flux through all parallel pipes
	m_velocity = m_volumeFlow/m_fluidCrossSection;

	m_heatLoss = 0.0;

	// assume constant heat transfer coefficient along pipe, using average temperature
	m_viscosity = m_fluidViscosity.value(m_meanTemperature);
	m_reynolds = ReynoldsNumber(m_velocity, m_viscosity, m_innerDiameter);
	m_prandtl = PrandtlNumber(m_viscosity, m_fluidHeatCapacity, m_fluidConductivity, m_fluidDensity);
	m_nusselt = NusseltNumber(m_reynolds, m_prandtl, m_length, m_innerDiameter);
	double innerHeatTransferCoefficient = m_nusselt * m_fluidConductivity / m_innerDiameter;

	// UAValueTotal has W/K, basically the u-value per length pipe (including transfer coefficients) x pipe length.
	// see documentation above
	if(m_outerHeatTransferCoefficient == 0.) {
		// UAValueTotal has W/K, basically the u-value per length pipe (including transfer coefficients) x pipe length.
		m_thermalTransmittance = m_discLength /
					   ( 1.0 / ( innerHeatTransferCoefficient * m_innerDiameter * PI ) +  1 / m_UValuePipeWall  );
	}
	else {
		// UAValueTotal has W/K, basically the u-value per length pipe (including transfer coefficients) x pipe length.
		m_thermalTransmittance = m_discLength /
								( 1.0 / (innerHeatTransferCoefficient * m_innerDiameter * PI)
								+ 1.0 / (m_outerHeatTransferCoefficient * m_outerDiameter * PI)
								+ 1.0 / m_UValuePipeWall ) ;
	}


	IBK_ASSERT(m_heatExchangeValueRef != nullptr);
	const double externalTemperature = *m_heatExchangeValueRef;
	for (unsigned int i = 0; i < m_nVolumes; ++i) {
		// calculate heat loss with given parameters
		// TODO : Hauke, check equation... hier fehlt glaub ich noch der Faktor 1/m_nVolumes
		m_heatLosses[i] = m_thermalTransmittance * (m_temperatures[i] - externalTemperature) * m_nParallelPipes;
		// sum up heat losses
		m_heatLoss += m_heatLosses[i];
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


double TNDynamicPipeElement::outflowTemperature() const {
	if (m_massFlux >= 0)
		return m_temperatures[m_nVolumes-1];
	else
		return m_temperatures[0];
}


void TNDynamicPipeElement::dependencies(const double *ydot, const double *y,
					const double *mdot, const double* TInflowLeft, const double*TInflowRight,
					std::vector<std::pair<const double *, const double *> > & resultInputDependencies) const
{
	// The model computes ydot[...], m_meanTemperature and m_heatLoss

	// Dependencies correspond to a 1D Finite Volume discretization using first-order upwinding
	// flux approxiation. Since flow go into either direction, we use the dependencies for central differencing,
	// thus ydot[i] depends on y[i-1], y[i] and y[i+1]. The boundary elements depend on the inlet and outlet temperatures.
	// And of course, all ydot depend on the mass flux.

	// Also, since we have heat exchange to the environment, each ydot depends on the external temperature.

	// set dependency to inlet and outlet enthalpy
	resultInputDependencies.push_back(std::make_pair(TInflowLeft, y) );
	resultInputDependencies.push_back(std::make_pair(TInflowRight, y + nInternalStates() - 1) );
	resultInputDependencies.push_back(std::make_pair(ydot, TInflowLeft) );
	resultInputDependencies.push_back(std::make_pair(ydot + nInternalStates() - 1, TInflowRight) );

	for (unsigned int n = 0; n < nInternalStates(); ++n) {
		// set dependency to mean temperatures
		resultInputDependencies.push_back(std::make_pair(&m_meanTemperature, y + n) );

		// set dependency to ydot from y i-1, i and i+1
		if (n > 0)
			resultInputDependencies.push_back(std::make_pair(ydot + n, y + n - 1) );

		resultInputDependencies.push_back(std::make_pair(ydot + n, y + n) );

		if (n < nInternalStates() - 1)
			resultInputDependencies.push_back(std::make_pair(ydot + n, y + n + 1) );

		// set dependency to mdot
		resultInputDependencies.push_back(std::make_pair(ydot + n, mdot) );
		// and to external temperature
		resultInputDependencies.push_back(std::make_pair(ydot + n, m_heatExchangeValueRef));

		// set dependency to Qdot
		resultInputDependencies.push_back(std::make_pair(&m_heatLoss, y + n) );
	}

	// set dependency to Qdot
	resultInputDependencies.push_back(std::make_pair(&m_heatLoss, mdot) );
	resultInputDependencies.push_back(std::make_pair(&m_heatLoss, m_heatExchangeValueRef) );
}


// *** DynamicAdiabaticPipeElement ***

TNDynamicAdiabaticPipeElement::TNDynamicAdiabaticPipeElement(const NANDRAD::HydraulicNetworkElement & elem,
							 const NANDRAD::HydraulicNetworkComponent & comp,
							const NANDRAD::HydraulicNetworkPipeProperties & pipePara,
							const NANDRAD::HydraulicFluid & fluid)
{
	double length = elem.m_para[NANDRAD::HydraulicNetworkElement::P_Length].value;
	double innerDiameter = pipePara.m_para[NANDRAD::HydraulicNetworkPipeProperties::P_PipeInnerDiameter].value;

	m_flowCrossSection = PI/4. * innerDiameter * innerDiameter;
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

	m_volumeFlow = std::fabs(m_massFlux)/m_fluidDensity; // m3/s !!! unit conversion is done when writing outputs
	m_velocity = m_volumeFlow/m_flowCrossSection;
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


double TNDynamicAdiabaticPipeElement::outflowTemperature() const {
	if (m_massFlux >= 0)
		return m_temperatures[m_nVolumes-1];
	else
		return m_temperatures[0];
}


void TNDynamicAdiabaticPipeElement::dependencies(const double *ydot, const double *y,
			const double *mdot, const double* TInflowLeft, const double*TInflowRight,
			std::vector<std::pair<const double *, const double *> > & resultInputDependencies) const
{
	// Dependencies correspond to a 1D Finite Volume discretization using first-order upwinding
	// flux approxiation. Since flow go into either direction, we use the dependencies for central differencing,
	// thus ydot[i] depends on y[i-1], y[i] and y[i+1]. The boundary elements depend on the inlet and outlet temperatures.
	// And of course, all ydot depend on the mass flux.

	// set dependency to inlet and outlet enthalpy
	resultInputDependencies.push_back(std::make_pair(TInflowLeft, y) );
	resultInputDependencies.push_back(std::make_pair(TInflowRight, y + nInternalStates() - 1) );
	resultInputDependencies.push_back(std::make_pair(ydot, TInflowLeft) );
	resultInputDependencies.push_back(std::make_pair(ydot + nInternalStates() - 1, TInflowRight) );

	for (unsigned int n = 0; n < nInternalStates(); ++n) {
		// set dependency to mean temperatures
		resultInputDependencies.push_back(std::make_pair(&m_meanTemperature, y + n) );

		// set dependency to ydot from y i-1, i and i+1
		if (n > 0)
			resultInputDependencies.push_back(std::make_pair(ydot + n, y + n - 1) );

		resultInputDependencies.push_back(std::make_pair(ydot + n, y + n) );

		if (n < nInternalStates() - 1)
			resultInputDependencies.push_back(std::make_pair(ydot + n, y + n + 1) );

		// set dependency to mdot
		resultInputDependencies.push_back(std::make_pair(ydot + n, mdot) );
	}
}



// *** PumpWithPerformanceLoss ***

TNPumpWithPerformanceLoss::TNPumpWithPerformanceLoss(
							const NANDRAD::HydraulicFluid & fluid,
							const NANDRAD::HydraulicNetworkComponent & comp,
							double pRef)
{
	// copy component properties
	m_fluidVolume = comp.m_para[NANDRAD::HydraulicNetworkComponent::P_Volume].value;
	m_pumpEfficiency = comp.m_para[NANDRAD::HydraulicNetworkComponent::P_PumpEfficiency].value;
	// copy fluid properties
	m_fluidDensity = fluid.m_para[NANDRAD::HydraulicFluid::P_Density].value;
	m_fluidHeatCapacity = fluid.m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value;
	// store pressure head
	m_pressureHead = pRef;
}


void TNPumpWithPerformanceLoss::setInflowTemperature(double Tinflow) {
	m_inflowTemperature = Tinflow;

	// mechanical power of pump is pressure head times volumetric flow
	// Pa * m3/s = N/m2 * m3/s = N*m/s

	// calculate pump performance
	m_mechanicalPower = m_massFlux/m_fluidDensity * m_pressureHead;

	// efficiency is defined as portion of total electrical power used for mechanical
	// Pelectrical * m_pumpEfficiency = Pmechanical
	m_electricalPower = m_mechanicalPower/m_pumpEfficiency;
	// calculate heat flux into fluid
	m_heatLoss = - (m_electricalPower - m_mechanicalPower);
}




// *** TNHeatPumpIdealCarnot ***

TNHeatPumpIdealCarnot::TNHeatPumpIdealCarnot(unsigned int flowElementId,
											 const NANDRAD::HydraulicFluid & fluid,
											 const NANDRAD::HydraulicNetworkComponent & comp) :
	m_flowElementId(flowElementId)
{
	m_fluidVolume = comp.m_para[NANDRAD::HydraulicNetworkComponent::P_Volume].value;
	m_carnotEfficiency = comp.m_para[NANDRAD::HydraulicNetworkComponent::P_CarnotEfficiency].value;
	m_condenserMaximumHeatFlux = comp.m_para[NANDRAD::HydraulicNetworkComponent::P_MaximumHeatingPower].value;
	m_nominalTemperatureDifference = comp.m_para[NANDRAD::HydraulicNetworkComponent::P_HeatPumpNominalTemperatureDifference].value;
	m_heatpumpIntegration = comp.m_heatPumpIntegration;

	// copy fluid properties
	m_fluidDensity = fluid.m_para[NANDRAD::HydraulicFluid::P_Density].value;
	m_fluidHeatCapacity = fluid.m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value;
}


void TNHeatPumpIdealCarnot::setInflowTemperature(double Tinflow) {
	FUNCID("TNHeatPumpIdealCarnot::setInflowTemperature");

	// copy inflow temperature
	m_inflowTemperature = Tinflow;

	switch (m_heatpumpIntegration ) {

		case NANDRAD::HydraulicNetworkComponent::HP_SourceSide: {

			// get scheduled temperature
			m_condenserMeanTemperature = *m_scheduledTemperature1;

			// cut condenser heat flux
			IBK_ASSERT(m_heatExchangeValueRef != nullptr);
			m_condenserHeatFlux = *m_heatExchangeValueRef;
			if (m_condenserHeatFlux > m_condenserMaximumHeatFlux)
				m_condenserHeatFlux = m_condenserMaximumHeatFlux;

			// heat pump physics only work when condenser temperature is above evaporator temperature
			m_evaporatorMeanTemperature = Tinflow - m_nominalTemperatureDifference / 2;
			if (m_condenserMeanTemperature > m_evaporatorMeanTemperature){
				const double COPMax = m_condenserMeanTemperature / (m_condenserMeanTemperature - m_evaporatorMeanTemperature);
				m_COP = m_carnotEfficiency * COPMax;
				m_heatLoss = m_condenserHeatFlux * (m_COP - 1) / m_COP;
				m_electricalPower  = m_condenserHeatFlux / m_COP;
			}
			else {
				IBK::IBK_Message(IBK::FormatString("Condenser temperature >= evaporator temperature in "
												   "HeatPumpIdealCarnot, flow element with id '%1'\n").arg(m_flowElementId),
													IBK::MSG_WARNING, FUNC_ID, IBK::VL_DETAILED);
				m_COP = 0.0;
				m_heatLoss = 0.0;
				m_electricalPower  = 0.0;
			}
		} break;


		// not fully implemented yet
		case NANDRAD::HydraulicNetworkComponent::HP_SupplySide:{

			// get scheduled temperatures
			m_evaporatorMeanTemperature = *m_scheduledTemperature1;
			const double condenserOutletSetpointTemperature = *m_scheduledTemperature2;

			// implement controller here
			double y = 100; // = f(Tinflow, condenserOutletSetpointTemperature)
			// cut condenser heat flux
			m_condenserHeatFlux = y;
			if (m_condenserHeatFlux > m_condenserMaximumHeatFlux)
				m_condenserHeatFlux = m_condenserMaximumHeatFlux;
			// heat pump can only add heat
			if (m_condenserHeatFlux < 0)
				m_condenserHeatFlux = 0;

			// heat pump physics only work when condenser temperature is above evaporator temperature
			m_condenserMeanTemperature = Tinflow + m_nominalTemperatureDifference / 2;
			if (m_condenserMeanTemperature > m_evaporatorMeanTemperature){
				const double COPMax = m_condenserMeanTemperature / (m_condenserMeanTemperature - m_evaporatorMeanTemperature);
				m_COP = m_carnotEfficiency * COPMax;
				m_heatLoss = - m_condenserHeatFlux;
				m_electricalPower  = m_condenserHeatFlux / m_COP;
			}
			else {
				IBK::IBK_Message(IBK::FormatString("Condenser temperature >= evaporator temperature in "
												   "HeatPumpIdealCarnot, flow element with id '%1'\n").arg(m_flowElementId),
													IBK::MSG_WARNING, FUNC_ID, IBK::VL_DETAILED);
				m_COP = 0.0;
				m_heatLoss = 0.0;
				m_electricalPower  = 0.0;
			}

		} break;

		case NANDRAD::HydraulicNetworkComponent::HP_SupplyAndSourceSide:
		case NANDRAD::HydraulicNetworkComponent::NUM_HP:
			break;
	}
}



void TNHeatPumpIdealCarnot::inputReferences(std::vector<InputReference> & inputRefs) const {

	switch (m_heatpumpIntegration) {

		case NANDRAD::HydraulicNetworkComponent::HP_SourceSide: {
			InputReference ref;
			ref.m_id = m_flowElementId;
			ref.m_referenceType = NANDRAD::ModelInputReference::MRT_NETWORKELEMENT;
			ref.m_name.m_name = "CondenserMeanTemperatureSchedule";
			ref.m_required = true;
			inputRefs.push_back(ref);
		} break;

		case NANDRAD::HydraulicNetworkComponent::HP_SupplySide: {
			InputReference ref;
			ref.m_id = m_flowElementId;
			ref.m_referenceType = NANDRAD::ModelInputReference::MRT_NETWORKELEMENT;
			ref.m_name.m_name = "EvaporatorMeanTemperatureSchedule";
			ref.m_required = true;
			inputRefs.push_back(ref);
			InputReference ref2;
			ref2.m_id = m_flowElementId;
			ref2.m_referenceType = NANDRAD::ModelInputReference::MRT_NETWORKELEMENT;
			ref2.m_name.m_name = "HeatingSetpointSchedule";
			ref2.m_required = true;
			inputRefs.push_back(ref2);
		} break;

		case NANDRAD::HydraulicNetworkComponent::HP_SupplyAndSourceSide:
		case NANDRAD::HydraulicNetworkComponent::NUM_HP:
			break;
	}
}


void TNHeatPumpIdealCarnot::setInputValueRefs(std::vector<const double *>::const_iterator & resultValueRefs) {

	// now store the pointer returned for our input ref request and advance the iterator by one
	switch (m_heatpumpIntegration) {

		case NANDRAD::HydraulicNetworkComponent::HP_SourceSide:
			m_scheduledTemperature1 = *(resultValueRefs++); // CondenserMeanTemperatureSchedule
			break;
		case NANDRAD::HydraulicNetworkComponent::HP_SupplySide:
			m_scheduledTemperature1 = *(resultValueRefs++); // EvaporatorMeanTemperatureSchedule
			m_scheduledTemperature2 = *(resultValueRefs++); // HeatingSetpointSchedule
			break;
		case NANDRAD::HydraulicNetworkComponent::HP_SupplyAndSourceSide:
		case NANDRAD::HydraulicNetworkComponent::NUM_HP:
			break;
	}
}



// *** AdiabaticElement ***

TNAdiabaticElement::TNAdiabaticElement(const NANDRAD::HydraulicFluid & fluid, double fluidVolume) {
	// copy fluid parameters
	m_fluidHeatCapacity = fluid.m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value;
	m_fluidDensity = fluid.m_para[NANDRAD::HydraulicFluid::P_Density].value;
	// store fluid volume
	m_fluidVolume = fluidVolume;
}



// *** ElementWithExternalHeatLoss ***

TNElementWithExternalHeatLoss::TNElementWithExternalHeatLoss(const NANDRAD::HydraulicFluid & fluid, double fluidVolume) {
	m_fluidVolume = fluidVolume;
	// copy fluid properties
	m_fluidDensity = fluid.m_para[NANDRAD::HydraulicFluid::P_Density].value;
	m_fluidHeatCapacity = fluid.m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value;
}


void TNElementWithExternalHeatLoss::internalDerivatives(double * ydot) {
	// set heat loss
	IBK_ASSERT(m_heatExchangeValueRef != nullptr);
	m_heatLoss = *m_heatExchangeValueRef;
	// use basic routine
	ThermalNetworkAbstractFlowElementWithHeatLoss::internalDerivatives(ydot);
}


} // namespace NANDRAD_MODEL
