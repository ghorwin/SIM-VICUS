#ifndef HydraulicNetworkAbstractFlowElementH
#define HydraulicNetworkAbstractFlowElementH

namespace NANDRAD_MODEL {

/*! Defines the interface for an abstract flow element. */
class HydraulicNetworkAbstractFlowElement {
public:
	HydraulicNetworkAbstractFlowElement() {}
	HydraulicNetworkAbstractFlowElement(unsigned int n_inlet, unsigned int n_outlet) :
		m_nInlet(n_inlet), m_nOutlet(n_outlet) {}

	/*! D'tor, definition is in NM_HydraulicNetworkFlowElements.cpp. */
	virtual ~HydraulicNetworkAbstractFlowElement();

	virtual double systemFunction(double mdot, double p_inlet, double p_outlet) const = 0;

	/*! Returns partial derivatives. */
	virtual void partials(double mdot, double p_inlet, double p_outlet,
						  double & df_dmdot, double & df_dp_inlet, double & df_dp_outlet) const = 0;

	/*! Index of inlet node. */
	unsigned int m_nInlet;
	/*! Index of outlet node. */
	unsigned int m_nOutlet;
};

} // namespace NANDRAD_MODEL

#endif // HydraulicNetworkAbstractFlowElementH
