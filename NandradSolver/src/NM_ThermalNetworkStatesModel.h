/*	NANDRAD Solver Framework and Model Implementation.

	Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Anne Paepcke     <anne.paepcke -[at]- tu-dresden.de>

	This library is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 3 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.
*/

#ifndef NM_ThermalNetworkStatesModelH
#define NM_ThermalNetworkStatesModelH

#include "NM_AbstractModel.h"

namespace NANDRAD {
	class HydraulicNetwork;
	class HydraulicNetworkComponent;
}


namespace NANDRAD_MODEL {

<<<<<<< Updated upstream
/*!	A model that computes all temperature states of hydraulic network given the internal energy density.

	The model publishes the temperatures for each flow element, so that these temperatures can be taken
	as fluid temperature inputs by the hydraulic network elements.
=======

class ThermalNetworkStatesModelImpl;

/*! Stores connectivity information. */
struct Node {
	Node() {}
	Node(unsigned int i1, unsigned int i2) {
		m_flowElementIndexesInlet.push_back(i1);
		m_flowElementIndexesOutlet.push_back(i2);
	}

	/*! Vector with indexes of inlet flow elements. */
	std::vector<unsigned int> m_flowElementIndexesInlet;
	std::vector<unsigned int> m_flowElementIndexesOutlet;
};


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


/*!	A model that computes all temperature states of hydraulic network given the internal energy density

	Other models may request this quantities via:
	ModelReferenceType = MRT_NETWORK
	id = (id of network model)
	Quantities:
	  - FlowElementTemperatures: in [K], vector-valued, access via flow element ID
*/
class ThermalNetworkStatesModel : public AbstractModel {
public:

	/*! Constructor. */
	ThermalNetworkStatesModel(unsigned int id, const std::string &displayName) :
		m_id(id), m_displayName(displayName), m_n(0)
	{
	}

	/*! D'tor, released pimpl object. */
	~ThermalNetworkStatesModel() override;

	/*! Initializes model.
	*/
	void setup(const NANDRAD::HydraulicNetwork & nw,
			   const std::vector<NANDRAD::HydraulicNetworkComponent> & components);

	// *** Re-implemented from AbstractModel

	/*! Network model is referenced via Network and ID. */
	virtual NANDRAD::ModelInputReference::referenceType_t referenceType() const override {
		return NANDRAD::ModelInputReference::MRT_NETWORK;
	}

	/*! Return unique class ID name of implemented model. */
	virtual const char * ModelIDName() const override { return "ThermalNetworkStatesModel"; }

	/*! Returns unique ID of this model instance. */
	virtual unsigned int id() const override { return m_id; }

	/*! Returns display name of this model instance. */
	virtual const char * displayName() const override { return m_displayName.c_str(); }

	/*! Populates the vector resDesc with descriptions of all results provided by this model. */
	virtual void resultDescriptions(std::vector<QuantityDescription> & resDesc) const override;

	/*! Retrieves reference pointer to a value with given quantity ID name.
		\note Quantity name of "y" returns pointer to start of local y data vector.
		\return Returns pointer to memory location with this quantity, otherwise NULL if parameter ID was not found.
	*/
	virtual const double * resultValueRef(const QuantityName & quantityName) const override;

	// *** Other public member functions

	/*! Returns number of conserved variables (i.e. length of y vector passed to yInitial() and update() ). */
	unsigned int nPrimaryStateResults() const;

	/*! Returns a vector of dependencies of all result quantities from y input quantities). */
	void stateDependencies(std::vector< std::pair<const double *, const double *> > & resultInputValueReferences) const;

	/*! Sets initial states in y vector.
		This function is called after setup(), so that parameters needed for
		computing the initial condition are already present.

		\param Pointer to the memory array holding all initial states for this network model (to be written into).
	*/
	void yInitial(double * y) const;

	/*! Computes fluid temperatures.
		This function is called directly from NandradModel as first step in the model evaluation.

		\param Pointer to the memory array holding all states for this room model.
			   The vector has either size 1 for thermal calculations or size 2 for hygrothermal calculations.
	*/
	int update(const double * y);

private:

	/*! Network ID. */
	unsigned int									m_id;
	/*! Display name (for error messages). */
	std::string										m_displayName;

	/*! Cached input data vector (size nPrimaryStateResults()). */
	std::vector<double>								m_y;


	// for each flow element instantiate an appropriate NetworkThermalBalanceFlowElement
	/*! Total number of unknowns.*/
	unsigned int									m_n;

	/*! Private implementation (Pimpl) of the network solver. */
	ThermalNetworkStatesModelImpl					*m_p = nullptr;

	friend class ThermalNetworkBalanceModel;
};

} // namespace NANDRAD_MODEL

#endif // NM_ThermalNetworkStatesModelH
