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

#ifndef NM_ThermalNetworkBalanceModelH
#define NM_ThermalNetworkBalanceModelH

#include "NM_AbstractModel.h"
#include "NM_AbstractStateDependency.h"

namespace NANDRAD_MODEL {

/*!	The thermal network balance model manages the calculation of enthalpy fuxes through
	the network elements and corresponding solver feedback.

	The thermal network states model and balance model share the same data structures (i.e.
	flow element thermal balance objects). To simplify things, the ThermalNetworkBalanceModel
	gets a pointer to the corresponding ThermalNetworkStatesModel instance.

	To evaluate the thermal balances in each flow element of the network, it needs:
	- prescribed/imposed fluxes to flow elements (externally computed)
	- surrounding temperatures (for example room air and radiation surface temperatures for heater equation models)
	- mass fluxes computed by network model

	The latter are retrieved from hydraulic network model. The nodal temperatures are then computed using upwinding
	rules and the "inlet/outlet" temperatures provided from the flow element thermal models.

	The other dependencies are formulated by the flow element thermal models themselves, and simply forwarded by
	the ThermalNetworkBalanceModel to the framework.
*/
class ThermalNetworkStatesModel;

/*!	The thermal network balance model manages the calculation of enrhalpy fuxes through
	the network elements and correponding solver feedback.
*/
class ThermalNetworkBalanceModel : public AbstractModel, public AbstractStateDependency {
public:

	/*! Constructor */
	ThermalNetworkBalanceModel(unsigned int id, const std::string &displayName) :
		m_id(id), m_displayName(displayName)
	{
	}

	/*! Initializes model by resizing the y and ydot vectors. */
	void setup(ThermalNetworkStatesModel *statesModel);

	// *** Re-implemented from AbstractModel

	/*! Thermal network balance model can be referenced via Network and ID. */
	virtual NANDRAD::ModelInputReference::referenceType_t referenceType() const override {
		return NANDRAD::ModelInputReference::MRT_NETWORK;
	}

	/*! Return unique class ID name of implemented model. */
	virtual const char * ModelIDName() const override { return "ThermalNetworkBalanceModel";}

	/*! Returns unique ID of this model instance. */
	virtual unsigned int id() const override { return m_id; }

	/*! Returns display name of this model instance. */
	virtual const char * displayName() const override { return m_displayName.c_str(); }

	/*! Populates the vector resDesc with descriptions of all results provided by this model. */
	virtual void resultDescriptions(std::vector<QuantityDescription> & resDesc) const override;

	/*! Returns vector of all scalar and vector valued results pointer. */
	virtual void resultValueRefs(std::vector<const double *> &res) const override;

	/*! Retrieves reference pointer to a value with given quantity ID name.
		\note For quantity 'ydot' the memory with computed ydot-values is returned.
		\return Returns pointer to memory location with this quantity, otherwise nullptr if parameter ID was not found.
	*/
	virtual const double * resultValueRef(const QuantityName & quantityName) const override;


	// *** Re-implemented from AbstractStateDependency

	/*! Returns model evaluation priority. */
	int priorityOfModelEvaluation() const override;

	/*! Composes all input references.
		Here we collect all loads/fluxes into the room and store them such, that we can efficiently compute
		sums, for example for all heat fluxes from constructions into the room etc.
	*/
	virtual void initInputReferences(const std::vector<AbstractModel*> & models) override;

	/*! Returns vector with model input references.
		Implicit models must generate their own model input references and populate the
		vector argument.
		\note This function is not the fastest, so never call this function from within the solver
		(except maybe for output writing).
	*/
	virtual void inputReferences(std::vector<InputReference>  & inputRefs) const override;

	/*! Provides the object with references to requested input variables (persistent memory location). */
	virtual void setInputValueRefs(const std::vector<QuantityDescription> & resultDescriptions, const std::vector<const double *> & resultValueRefs) override;

	/*! Returns dependencies between result variables and input variables. */
	virtual void stateDependencies(std::vector< std::pair<const double *, const double *> > & resultInputValueReferences) const override;

	/*! Sums up all provided input quantities and computes divergence of balance equations. */
	int update() override;


	// *** Other public member functions

	/*! Stores the divergences of all balance equations in this zone in vector ydot. */
	int ydot(double* ydot);

private:
	void printVars() const;

	/*! Zone ID. */
	unsigned int									m_id;
	/*! Display name (for error messages). */
	std::string										m_displayName;
	/*! Vector with cached derivatives, updated at last call to update(). */
	std::vector<double>								m_ydot;

	/*! Poiter to states model. */
	ThermalNetworkStatesModel						*m_statesModel;
};

} // namespace NANDRAD_MODEL

#endif // NM_ThermalNetworkBalanceModelH
