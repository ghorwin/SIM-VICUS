/*	NANDRAD Solver Framework and Model Implementation.

	Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Anne Paepcke     <anne.paepcke -[at]- tu-dresden.de>

	This program is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

#ifndef NM_ThermalNetworkBalanceModelH
#define NM_ThermalNetworkBalanceModelH

#include "NM_AbstractModel.h"
#include "NM_AbstractStateDependency.h"

#include <NANDRAD_Constants.h>

#include <list>

namespace IBK {
	class LinearSpline;
}

namespace NANDRAD {
	class LinearSplineParameter;
}

namespace NANDRAD_MODEL {

class ThermalNetworkStatesModel;

/*!	The thermal network balance model manages the calculation of enthalpy fuxes through
	the network elements and heat fluxes towards other NANDRAD model components (zones, constructions).

	The ThermalNetworkStatesModel and ThermalNetworkBalanceModel share the same data structures (i.e.
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

	## Provided results ##

	ModelReferenceType = MRT_NETWORK
	id = (id of network)
	Quantities:
		- NetworkZoneThermalLoad (in [W]), vector-valued, access via zone ID
		- ActiveLayerThermalLoad (in [W]), vector-valued, access via construction instance ID

	ModelReferenceType = MRT_NETWORKELEMENT
	id = (id of flow element)
	Quantities:
		- FlowElementHeatLoss  (in [W])
		- InletNodeTemperature (in [C])
		- OutletNodeTemperature (in [C])
		- ... additional, flow-element specific outputs
*/
class ThermalNetworkBalanceModel : public AbstractModel, public AbstractStateDependency {
public:

	/*! Constructor */
	ThermalNetworkBalanceModel(unsigned int id, const std::string &displayName) :
		m_id(id), m_displayName(displayName)
	{
	}

	/*! Initializes model by resizing the y and ydot vectors.
		Most of the data structures are already initialized by ThermalNetworkStatesModel.
	*/
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

	/*! Retrieves reference pointer to a value with given quantity ID name.
		\note For quantity 'ydot' the memory with computed ydot-values is returned.
		\return Returns pointer to memory location with this quantity, otherwise nullptr if parameter ID was not found.
	*/
	virtual const double * resultValueRef(const InputReference & quantity) const override;

	// *** Re-implemented from AbstractStateDependency

	/*! Returns model evaluation priority. */
	int priorityOfModelEvaluation() const override;

	/*! Returns vector with model input references.
		Implicit models must generate their own model input references and populate the
		vector argument.
		\note This function is not the fastest, so never call this function from within the solver
		(except maybe for output writing).
	*/
	virtual void inputReferences(std::vector<InputReference>  & inputRefs) const override;

	/*! Provides the object with references to requested input variables (persistent memory location). */
	virtual void setInputValueRefs(const std::vector<QuantityDescription> & resultDescriptions,
								   const std::vector<const double *> & resultValueRefs) override;

	/*! Returns dependencies between result variables and input variables. */
	virtual void stateDependencies(std::vector< std::pair<const double *, const double *> > & resultInputValueReferences) const override;

	/*! Sums up all provided input quantities and computes divergence of balance equations. */
	int update() override;


	// *** Other public member functions

	/*! Stores the divergences of all balance equations in this zone in vector ydot. */
	int ydot(double* ydot);

private:
	void printVars() const;


	/*!	This struct stores information needed for exchange between the ThermalNetworkBalanceModel and
		RoomBalanceModels. The ThermalNetworkBalanceModel collects in this struct for each zone
		the sum of energy loads into the zone and publishes that as

		Such a struct is *only* created for each zone that one or more flow elements exchanges heat with.
	*/
	struct ZoneProperties {
		/*! Standard constructor. */
		ZoneProperties(unsigned int id): m_zoneId(id) { }

		/*! Comparison operator via id. */
		bool operator==(unsigned int x) const { return m_zoneId == x; }

		/*! Zone id. */
		unsigned int						m_zoneId = NANDRAD::INVALID_ID;
		/*! Heat flux (sum) of *all* flow elements into selected zone. */
		double								m_zoneHeatLoad = -999;
	};

	/*!	Struct stores information needed for exchange between ThermalNetworkBalanceModel and ConstructionBalanceModels.

		Such a struct is *only* created for each construction instance that a flow element exchanges heat with.

		NOTE: in contrast to zones, there must only be exactly one flow element exchanging heat with exactly
			  one construction instance.
	*/
	struct ActiveLayerProperties {
		/*! Standard constructor. */
		ActiveLayerProperties(unsigned int id): m_constructionInstanceId(id) { }

		/*! Comparison operator via id. */
		bool operator==(unsigned int x) const { return m_constructionInstanceId == x; }

		/*! Construction instance id. */
		unsigned int						m_constructionInstanceId = NANDRAD::INVALID_ID;
		/*! Heat flux into (the one and only) active layer if the selected construction instance. */
		double								m_activeLayerHeatLoad = -999;
	};


	/*!	Detailed description of flow element physical behaviour: calculated values, value
		references and zone/construction references.

		This struct groups all data assembled for a flow element.
	*/
	struct FlowElementProperties {
		/*! Standard constructor. */
		FlowElementProperties(unsigned int id): m_elementId(id) { }

		/*! Comparison operator via id. */
		bool operator==(unsigned int x) const { return m_elementId == x; }

		/*! Flow element id. */
		unsigned int						m_elementId = NANDRAD::INVALID_ID;
		/*! Zone properties for heat exchange to zone (nullptr if no heat exchange with a zone is defined). */
		ZoneProperties						*m_zoneProperties = nullptr;
		/*! Active layer properties for heat exchange with a construction layer
			(nullptr if no heat exchange with a construtcion layer is defined).
		*/
		ActiveLayerProperties				*m_activeLayerProperties = nullptr;

		// the following references point to results computed from flow elements.

		/*! Reference to heat flux out of the flow element). */
		const double*						m_heatLossRef = nullptr;
		/*! Reference to temperatures for inlet node of the flow element. */
		const double*						m_inletNodeTemperatureRef = nullptr;
		/*! Reference to temperatures for outlet node of the flow element. */
		const double*						m_outletNodeTemperatureRef = nullptr;
	};


	/*!	Detailed description of node behaviour: calculated values, value
		references and zone references.

		Note: this struct is created for all nodes, even for plain network nodes without
			  state. In this case, m_nodeId == INVALID_ID.

		This struct groups all data assembled for a network node.
	*/
	struct NodeProperties {
		/*! Standard constructor. */
		NodeProperties(unsigned int nodeId, unsigned int zoneId): m_nodeId(nodeId), m_zoneId(zoneId) { }

		/*! Comparison operator via id. */
		bool operator==(unsigned int x) const { return m_nodeId == x || m_zoneId == x; }

		/*! Node id. */
		unsigned int						m_nodeId = NANDRAD::INVALID_ID;
		/*! Zone id. */
		unsigned int						m_zoneId = NANDRAD::INVALID_ID;

		/*! Zone properties for heat exchange to zone (nullptr if nodeID == INVALID_ID). */
		ZoneProperties						*m_zoneProperties = nullptr;
	};


	/*! Zone ID. */
	unsigned int									m_id;
	/*! Display name (for error messages). */
	std::string										m_displayName;
	/*! Vector with cached derivatives, updated at last call to update(). */
	std::vector<double>								m_ydot;

	/*! Properties of all zones involved in heat exchange to a network element.
		One ZoneProperties element for each zone that one or more flow elements exchange heat with.
		The objects in this list are accessed via pointer, stored in the respective FlowElementProperties object.
	*/
	std::list<ZoneProperties>						m_zoneProperties;
	/*! Properties of all active layers involved in heat exchange to a network element.
		One ActiveLayerProperties object for each construction instance that has heat exchange with a flow
		element.
		The objects in this list are accessed via pointer, stored in the respective FlowElementProperties object.
	*/
	std::list<ActiveLayerProperties>				m_activeProperties;

	/*! Physical properties of all network elements (size = ThermalNetworkModelImpl::m_flowElements.size()).*/
	std::vector<FlowElementProperties>				m_flowElementProperties;

	/*! Connectivity properties of all network nodes (size = ...), reference from zone node to ZoneProperties. */
	std::vector<NodeProperties>						m_nodeProperties;

	/*! Vector of all additional model quantities for outputs. */
	std::vector<QuantityDescription>				m_modelQuantities;
	/*! Vector of all additional model quantity references. */
	std::vector<const double *>						m_modelQuantityRefs;
	/*! Offset of quantities for all models inside modelQuantities and modelQuantityRefs vector. */
	std::vector<unsigned int>						m_modelQuantityOffset;

	/*! Pointer to states model. */
	ThermalNetworkStatesModel						*m_statesModel = nullptr;

};

} // namespace NANDRAD_MODEL

#endif // NM_ThermalNetworkBalanceModelH
