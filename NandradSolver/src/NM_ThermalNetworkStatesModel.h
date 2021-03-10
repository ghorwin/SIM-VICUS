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
#include "NM_AbstractTimeDependency.h"

#include <NANDRAD_Constants.h>

namespace IBK {
	class LinearSpline;
}

namespace NANDRAD {
	class HydraulicNetwork;
	class HydraulicNetworkComponent;
	class SimulationParameter;
}


namespace NANDRAD_MODEL {

/*!	Struct for all value references exchanged between element model
	and ThermalNetworkStatesModel/ThermalNetworkBalanceModel.
*/
struct ThermalNetworkElementValueRefs {


	// resizing
	void resize(unsigned int nValues) {
		m_nValues = nValues;
		m_zoneIdxs.resize(nValues, NANDRAD::INVALID_ID);
		m_constructionInstanceIdxs.resize(nValues, NANDRAD::INVALID_ID);
		m_meanTemperatureRefs.resize(nValues,nullptr);
		m_heatExchangeSplineRefs.resize(nValues,nullptr);
		m_flowElementHeatLossRefs.resize(nValues,nullptr);
		m_inletNodeTemperatureRefs.resize(nValues,nullptr);
		m_outletNodeTemperatureRefs.resize(nValues,nullptr);
		m_heatExchangeRefValues.resize(nValues,-999);
	}

	/*! Number of vector valued for each quantity. */
	unsigned int									m_nValues = 999;
	/*! Indexes of all zones for each network element (size = m_nValues),
		NANDRAD::INVALID_ID for missing zone.
	*/
	std::vector<unsigned int>						m_zoneIdxs;
	/*! Indexes of all construction instances for each network element (size = m_nValues),
		NANDRAD::INVALID_ID for missing construction.
	*/
	std::vector<unsigned int>						m_constructionInstanceIdxs;
	/*! Vector with references to mean temperatures (size = m_nValues). */
	std::vector<const double*>						m_meanTemperatureRefs;
	/*! References to heat exchange spline: nullptr if not needed (size = m_nValues). */
	std::vector<const IBK::LinearSpline*>			m_heatExchangeSplineRefs;
	/*! References to heat fluxes out of each heat flow element (size = m_nValues).
	*/
	std::vector<const double*>						m_flowElementHeatLossRefs;
	/*! References to temperatures for inlet node of each flow element (size = m_nValues).
	*/
	std::vector<const double*>						m_inletNodeTemperatureRefs;
	/*! References to with temperatures for outlet node of each flow element (size = m_nValues).
	*/
	std::vector<const double*>						m_outletNodeTemperatureRefs;
	/*! Container with current spline or reference values: either temperature [K] or heat flux [W]
		 (size = m_nValues).
	*/
	std::vector<double>								m_heatExchangeRefValues;

};


class HydraulicNetworkModel;
class ThermalNetworkModelImpl;

/*!	A model that computes all temperature states of hydraulic network given the internal energy density

	Other models may request this quantities via:
	ModelReferenceType = MRT_NETWORK
	id = (id of network model)
	Quantities:
	  - FlowElementTemperatures: in [K], vector-valued, access via flow element ID
*/
class ThermalNetworkStatesModel : public AbstractModel, public AbstractTimeDependency {
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
			   const HydraulicNetworkModel &networkModel,
			   const NANDRAD::SimulationParameter &simPara);

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
	virtual const double * resultValueRef(const InputReference & quantity) const override;

	// *** Re-implemented from AbstractStateDependency
	virtual int setTime(double t) override;

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
	void yInitial(double * y);

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
	/*! Storage of all network element ids, used for vector output. */
	std::vector<unsigned int>						m_elementIds;
	/*! All zone ids referenced by a fluid component (size = m_elementIDs.size()). */
	std::vector<unsigned int>						m_zoneIds;
	/*! All construction instance ids referenced by a fluid component (size = m_elementIDs.size()). */
	std::vector<unsigned int>						m_constructionInstanceIds;
	/*! References to zone temperatures (size = m_zoneIds.size()). */
	std::vector<const double*>						m_zoneTemperatureRefs;
	/*! References to construction layer temperatures (size = m_constrctionInstanceIds.size()). */
	std::vector<const double*>						m_activeLayerTemperatureRefs;
	/*! Container of all model value references.*/
	ThermalNetworkElementValueRefs					m_elementValueRefs;

	/*! Cached input data vector (size nPrimaryStateResults()). */
	std::vector<double>								m_y;
	/*! Total number of unknowns. */
	unsigned int									m_n;

	/*! Pointer to NANDRAD network structure*/
	const NANDRAD::HydraulicNetwork					*m_network=nullptr;

	/*! Private implementation (Pimpl) of the network solver. */
	ThermalNetworkModelImpl							*m_p = nullptr;

	friend class ThermalNetworkBalanceModel;
};

} // namespace NANDRAD_MODEL

#endif // NM_ThermalNetworkStatesModelH
