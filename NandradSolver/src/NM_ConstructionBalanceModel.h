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


#ifndef ConstructionBalanceModelH
#define ConstructionBalanceModelH

#include "NM_AbstractModel.h"
#include "NM_AbstractStateDependency.h"

namespace NANDRAD {
	class ConstructionInstance;
	class SimulationParameter;
	class Interface;
}

namespace NANDRAD_MODEL {

class ConstructionStatesModel;

/*!	This model encapsulates the most part of the computation functionality for a
	1D construction solver.
	It computes internal sources and boundary condition fluxes, and finally the divergences
	of all balance equations in the model.

	Depending on formulated boundary conditions, it requires input from RoomStatesModel and Loads models.
*/
class ConstructionBalanceModel : public AbstractModel, public AbstractStateDependency {
public:

	enum Results {
		R_FluxHeatConductionA,				// Keyword: FluxHeatConductionA		[W]			'Heat conduction flux across interface A (into construction).'
		R_FluxHeatConductionB,				// Keyword: FluxHeatConductionB		[W]			'Heat conduction flux across interface B (into construction).'
		NUM_R
	};

	enum VectorValuedResults {
		VVR_ThermalLoad,					// Keyword: ThermalLoad				[W]			'Optional field fluxes for all material layers with given layer index.'
		NUM_VVR
	};

	/*! Constructor */
	ConstructionBalanceModel(unsigned int id, const std::string &displayName) :
		m_id(id), m_displayName(displayName)
	{
	}

	/*! Initializes model. */
	void setup(const NANDRAD::ConstructionInstance & con,
			   const ConstructionStatesModel * statesModel);


	// *** Re-implemented from AbstractModel

	/*! Balance model can be referenced as ConstructionInstance and ID. */
	virtual NANDRAD::ModelInputReference::referenceType_t referenceType() const override {
		return NANDRAD::ModelInputReference::MRT_CONSTRUCTIONINSTANCE;
	}

	/*! Return unique class ID name of implemented model. */
	virtual const char * ModelIDName() const override { return "ConstructionBalanceModel";}

	/*! Returns unique ID of this model instance. */
	virtual unsigned int id() const override { return m_id; }

	/*! Returns display name of this model instance. */
	virtual const char * displayName() const override { return m_displayName.c_str(); }

	/*! Populates the vector resDesc with descriptions of all results provided by this model. */
	virtual void resultDescriptions(std::vector<QuantityDescription> & resDesc) const override;

	/*! Returns vector of all scalar and vector valued results pointer. */
	virtual void resultValueRefs(std::vector<const double *> &res) const override;

	/*! Retrieves reference pointer to a value with given quantity ID name.
		\return Returns pointer to memory location with this quantity, otherwise nullptr if parameter ID was not found.
	*/
	virtual const double * resultValueRef(const QuantityName & quantityName) const override;


	// *** Re-implemented from AbstractStateDependency

	/*! Return evaluation priority. */
	int priorityOfModelEvaluation() const override;

	/*! Returns vector with model input references.
		Implicit models must generate their own model input references and populate the
		vector argument.
		\note This function is not the fastest, so never call this function from within the solver
		(except maybe for output writing).

		\todo Implement as soon as
	*/
	virtual void inputReferences(std::vector<InputReference>  & inputRefs) const override;

	/*! Provides the object with references to requested input variables (persistent memory location). */
	virtual void setInputValueRefs(const std::vector<QuantityDescription> & resultDescriptions, const std::vector<const double *> & resultValueRefs) override;

	/*! Sums up all provided input quantities and computes divergence of balance equations. */
	int update() override;


	// *** Other public member functions

	/*! Stores the divergences of all balance equations in this zone in vector ydot. */
	int ydot(double* ydot);

private:
	/*! Computes boundary condition fluxes. */
	void calculateBoundaryConditions(bool sideA, const NANDRAD::Interface & iface);


	/*! Enumeration types for ordered input references, some may be unused and remain nullptr. */
	enum InputReferences {
		InputRef_AmbientTemperature,
		InputRef_RoomATemperature,
		InputRef_RoomBTemperature,
		NUM_InputRef
	};

	/*! Construction instance ID. */
	unsigned int									m_id;
	/*! Display name (for error messages). */
	std::string										m_displayName;
	/*! True if moisture balance is enabled. */
	bool											m_moistureBalanceEnabled;
	/*! Net cross section of construction in [m2]. */
	double											m_area;
	/*! Data cache for calculated results (updated in call to update()).
		Index matches enum values of Results.
	*/
	std::vector<double>								m_results;

	/*! Cached divergences of balance equations. */
	std::vector<double>								m_ydot;

	/*! Vector with input references, first the NUM_InputRef scalar input refs, then the vector-valued. */
	std::vector<const double*>						m_valueRefs;

	/*! Construction interface. */
	const NANDRAD::ConstructionInstance				*m_con;
	/*! Cached pointer to the corresponding states model. */
	const ConstructionStatesModel					*m_statesModel;
//	/*! Cached pointer to simulation settings. */
//	const NANDRAD::SimulationParameter				*m_simPara;

	/*! Heat conduction flux density at left side [W/m2] (positive from left to right). */
	double											m_fluxDensityHeatConductionA = 0;
	/*! Heat conduction flux density at right side [W/m2] (positive from left to right). */
	double											m_fluxDensityHeatConductionB = 0;
};

} // namespace NANDRAD_MODEL

#endif // ConstructionBalanceModelH
