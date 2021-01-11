#ifndef ThermalNetworkFlowElementsH
#define ThermalNetworkFlowElementsH

#include "NM_ThermalNetworkAbstractFlowElement.h"

namespace NANDRAD {
	class HydraulicNetworkElement;
	class HydraulicNetworkComponent;
	class HydraulicNetworkPipeProperties;
	class HydraulicFluid;
}


namespace NANDRAD_MODEL {

class TNPipeElement : public ThermalNetworkAbstractFlowElement {
public:
	TNPipeElement() { }

	/*! C'tor, takes and caches parameters needed for function evaluation. */
	TNPipeElement(const NANDRAD::HydraulicNetworkElement & elem,
				  const NANDRAD::HydraulicNetworkComponent & comp,
				  const NANDRAD::HydraulicNetworkPipeProperties & pipePara,
				  const NANDRAD::HydraulicFluid & fluid);

	/*! D'tor, definition is in NM_HydraulicNetworkFlowElements.cpp. */
	~TNPipeElement();

	/*! Function retrieving number of internal states.*/
	unsigned int nInternalStates() const;

	/*! Function for setting internal states such as internal enery.
		May be vector valued for layered temperature models.*/
	void setInternalStates(const double *y);

	/*! Function for retrieving heat fluxes out of the flow element.*/
	void internalDerivatives(double *ydot);

	/*! Set fluid inlet conditions. */
	void setInletFluxes(double mdot, double Hdot);

	/*! Set ambient conditions. */
	void setAmbientConditions(double Tamb, double alphaAmb);

	/*! Returns fluid outlet states: spcific enthalpy. */
	void outletSpecificEnthalpy(double &h) const;

private:

	/*! the fluid, containing all physical parameters */
	const NANDRAD::HydraulicFluid	*m_fluid = nullptr;

	/*! pipe length in m */
	double							m_length;

	/*! hydraulic (inner) diameter of pipe in m */
	double							m_innerDiameter;

	/*! outer diameter of pipe in m */
	double							m_outerDiameter;

	/*! fluid volume inside the pipe m3 */
	double							m_volume;

	/*! thermal resistance of the pipe wall in Km2/W */
	double							m_UValuePipeWall;

	/*! Heat transfer coeffient from fluid towards the wall in W/m2K */
	double							m_innerHeatTransferCoefficient;

	/*! Heat transfer coefficient from outer pipe wall to environment in W/m2K */
	double							m_outerHeatTransferCoefficient;

	/*! Fluid inlet temperature */
	double							m_inletTemperature = 273.15;

	/*! Environamental temperature */
	double							m_ambientTemperature = 273.15;

	/*! Fluid mass flux */
	double							m_massFlux = 0.0;

	/*! Fluid specific enthalpy */
	double							m_specificEnthalpy = 0.0;

	/*! Heat loss through the pipe */
	double							m_heatLoss = 0.0;
};



// **** HeatExchanger ***

class TNHeatExchanger : public ThermalNetworkAbstractFlowElement {
public:
	TNHeatExchanger() { }

	/*! C'tor, takes and caches parameters needed for function evaluation. */
	TNHeatExchanger(const NANDRAD::HydraulicNetworkElement & elem,
				  const NANDRAD::HydraulicNetworkComponent & comp,
				  const NANDRAD::HydraulicFluid & fluid);

	/*! D'tor, definition is in NM_HydraulicNetworkFlowElements.cpp. */
	~TNHeatExchanger();

	/*! Function retrieving number of internal states.*/
	unsigned int nInternalStates() const;

	/*! Function for setting internal states such as internal enery.
		May be vector valued for layered temperature models.*/
	void setInternalStates(const double *y);

	/*! Function for retrieving heat fluxes out of the flow element.*/
	void internalDerivatives(double *ydot);

	/*! Set fluid inlet conditions. */
	void setInletFluxes(double mdot, double Hdot);

	/*! Set ambient conditions. */
	void setAmbientConditions(double Tamb, double alphaAmb);

	/*! Returns fluid outlet states: spcific enthalpy. */
	void outletSpecificEnthalpy(double &h) const;

	double heatLoss();

private:

	/*! the fluid, containing all physical parameters */
	const NANDRAD::HydraulicFluid	*m_fluid = nullptr;

	/*! fluid volume inside the pipe m3 */
	double							m_volume;

	double							m_heatFluxToAmbient;

	double							m_UAValue;

	/*! Fluid inlet temperature */
	double							m_inletTemperature = 273.15;

	/*! Fluid mass flux */
	double							m_massFlux = 0.0;

	/*! Fluid specific enthalpy */
	double							m_specificEnthalpy = 0.0;

};




// **** HeatExchanger ***

class TNPump: public ThermalNetworkAbstractFlowElement {
public:
	TNPump() { }

	/*! C'tor, takes and caches parameters needed for function evaluation. */
	TNPump(const NANDRAD::HydraulicNetworkElement & elem,
				const NANDRAD::HydraulicNetworkComponent & comp,
				const NANDRAD::HydraulicFluid & fluid);

	/*! D'tor, definition is in NM_HydraulicNetworkFlowElements.cpp. */
	~TNPump();

	/*! Function retrieving number of internal states.*/
	unsigned int nInternalStates() const;

	/*! Function for setting internal states such as internal enery.
		May be vector valued for layered temperature models.*/
	void setInternalStates(const double *y);

	/*! Function for retrieving heat fluxes out of the flow element.*/
	void internalDerivatives(double *ydot);

	/*! Set fluid inlet conditions. */
	void setInletFluxes(double mdot, double Hdot);

	/*! Set ambient conditions. */
	void setAmbientConditions(double Tamb, double alphaAmb);

	/*! Returns fluid outlet states: spcific enthalpy. */
	void outletSpecificEnthalpy(double &h) const;

	double heatLoss();

private:

	/*! the fluid, containing all physical parameters */
	const NANDRAD::HydraulicFluid	*m_fluid = nullptr;

	/*! fluid volume inside the pipe m3 */
	double							m_volume;

	/*! Fluid inlet temperature */
	double							m_inletTemperature = 273.15;

	/*! Fluid mass flux */
	double							m_massFlux = 0.0;

	/*! Fluid specific enthalpy */
	double							m_specificEnthalpy = 0.0;

};


} // namespace NANDRAD_MODEL

#endif // ThermalNetworkFlowElementsH
