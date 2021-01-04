#ifndef ThermalNetworkAbstractFlowElementH
#define ThermalNetworkAbstractFlowElementH

namespace NANDRAD_MODEL {

/*! Defines the interface for an abstract flow element. */
class ThermalNetworkAbstractFlowElement {
public:
	ThermalNetworkAbstractFlowElement() {}
	ThermalNetworkAbstractFlowElement(unsigned int n_inlet, unsigned int n_outlet) :
		m_nInlet(n_inlet), m_nOutlet(n_outlet) {}

	/*! D'tor, definition is in NM_HydraulicNetworkFlowElements.cpp. */
	virtual ~ThermalNetworkAbstractFlowElement() { }

	/*! Function retrieving number of internal states.*/
	virtual unsigned int nInternalStates() const = 0;

	/*! Function for setting internal states such as internal enery.
		May be vector valued for layered temperature models.*/
	virtual void setInternalStates(const double *y) = 0;

	/*! Function for retrieving heat fluxes out of the flow element.*/
	virtual void internalHeatLosses(double *ydot) const = 0;

	/*! Set fluid inlet conditions. */
	virtual void setInletFluxes(double mdot, double Hdot) = 0;

	/*! Returns fluid outlet states: spcific enthalpy. */
	virtual void outletSpecificEnthalpy(double &h) const = 0;

	/*! Index of inlet node. */
	unsigned int m_nInlet;
	/*! Index of outlet node. */
	unsigned int m_nOutlet;
};


class TNPipeElement : public ThermalNetworkAbstractFlowElement {
public:
	TNPipeElement() { }

	/*! D'tor, definition is in NM_HydraulicNetworkFlowElements.cpp. */
	virtual ~TNPipeElement() { }

	/*! Function retrieving number of internal states.*/
	virtual unsigned int nInternalStates() const {return 0;}

	/*! Function for setting internal states such as internal enery.
		May be vector valued for layered temperature models.*/
	virtual void setInternalStates(const double *y) { }

	/*! Function for retrieving heat fluxes out of the flow element.*/
	virtual void internalHeatLosses(double *ydot) const { }

	/*! Set fluid inlet conditions. */
	virtual void setInletFluxes(double mdot, double Hdot) { }

	/*! Returns fluid outlet states: spcific enthalpy. */
	virtual void outletSpecificEnthalpy(double &h) const { }
};



} // namespace NANDRAD_MODEL

#endif // ThermalNetworkAbstractFlowElementH
