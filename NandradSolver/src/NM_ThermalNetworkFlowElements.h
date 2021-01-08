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
				  const NANDRAD::HydraulicNetworkComponent & component,
				  const NANDRAD::HydraulicNetworkPipeProperties & pipePara,
				  const NANDRAD::HydraulicFluid & fluid);

	/*! D'tor, definition is in NM_HydraulicNetworkFlowElements.cpp. */
	virtual ~TNPipeElement();

	/*! Function retrieving number of internal states.*/
	virtual unsigned int nInternalStates() const;

	/*! Function for setting internal states such as internal enery.
		May be vector valued for layered temperature models.*/
	virtual void setInternalEnthalpies(const double *y);

	/*! Function for retrieving heat fluxes out of the flow element.*/
	virtual void internalHeatLosses(double *ydot);

	/*! Set fluid inlet conditions. */
	virtual void setInletFluxes(double mdot, double Hdot);

	/*! Set ambient conditions. */
	virtual void setAmbientConditions(double Tamb, double alphaAmb);

	/*! Returns fluid outlet states: spcific enthalpy. */
	virtual void outletSpecificEnthalpy(double &h) const;

private:

	/*! the fluid, containing all physical parameters */
	const NANDRAD::HydraulicFluid	*m_fluid = nullptr;

	/*! pipe length in m */
	double							m_length;

	/*! hydraulic (inner) diameter of pipe in m */
	double							m_diameter;

	/*! fluid volume inside the pipe m3 */
	double							m_volume;

	/*! thermal resistance of the pipe wall in Km2/W */
	double							m_thermalResistanceWall;

	/*! Heat transfer coeffient from fluid towards the wall in W/m2K */
	double							m_innerHeatTransfer;

	/*! Heat transfer coefficient from outer pipe wall to environment in W/m2K */
	double							m_ambientHeatTransfer;

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



} // namespace NANDRAD_MODEL

#endif // ThermalNetworkFlowElementsH
