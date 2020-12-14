#ifndef HydraulicNetworkAbstractFlowElementH
#define HydraulicNetworkAbstractFlowElementH

/*! Defines the interface for an abstract flow element. */
class HydraulicNetworkAbstractFlowElement {
public:
	HydraulicNetworkAbstractFlowElement();
	HydraulicNetworkAbstractFlowElement(unsigned int n_inlet, unsigned int n_outlet) :
		m_nInlet(n_inlet), m_nOutlet(n_outlet) {}

	virtual ~HydraulicNetworkAbstractFlowElement();

	virtual double systemFunction(double mdot, double p_inlet, double p_outlet) const = 0;

	/*! Returns mass flux for given pressures. */
	virtual double mdot(double p_inlet, double p_outlet) const = 0;

	/*! Returns partial derivatives. */
	virtual void dmdot_dp(double mdot, double p_inlet, double p_outlet, double & dmdp_in, double & dmdp_out) const = 0;

	/*! Index of inlet node. */
	unsigned int m_nInlet;
	/*! Index of outlet node. */
	unsigned int m_nOutlet;
};

#endif // HydraulicNetworkAbstractFlowElementH
