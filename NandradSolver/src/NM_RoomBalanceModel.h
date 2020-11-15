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

#ifndef NM_RoomBalanceModelH
#define NM_RoomBalanceModelH

#include "NM_AbstractModel.h"
#include "NM_AbstractStateDependency.h"

namespace NANDRAD {
	class SimulationParameter;
	class Zone;
}


namespace NANDRAD_MODEL {

/*!	The room balance model handles the calculation of all heat fluxes
	towards the room which are results of other model objects.

	The modell basically sums up all provided fluxes from constructions and equipment
	and then computed the time derivative of the balanced energies. When moisture balance
	is enabled, it also balances moisture content in the room air.

	This is a "tail"-type model and explicitely evaluated last in the model evaluation.
	It is not part of the dependency graph and the solver framework calls update() and ydot()
	directly at the end of the model update.

	The retrieved ydot values are then used by the time integrator.
*/
class RoomBalanceModel : public AbstractModel, public AbstractStateDependency {
public:

	/*! Results computed by the model. */
	enum Results {
		R_CompleteThermalLoad,								// Keyword: CompleteThermalLoad							[W]		'Sum of all thermal fluxes into the room and energy sources.'
		R_InfiltrationHeatLoad,								// Keyword: InfiltrationHeatLoad						[W]		'Infiltration/natural ventilation heat flux into the room.'
		R_ConstructionHeatConductionLoad,					// Keyword: ConstructionHeatConductionLoad				[W]		'Sum of heat conduction fluxes from construction surfaces into the room.'
		R_WindowHeatConductionLoad,							// Keyword: WindowHeatConductionLoad					[W]		'Sum of heat conduction fluxes through windows into the room.'
		R_WindowSolarRadiationLoad,							// Keyword: WindowSolarRadiationLoad					[W]		'Sum of solar radiation fluxes through windows into the room (only the fraction applied to room volume).'
		R_CompleteMoistureLoad,								// Keyword: CompleteMoistureLoad						[kg/s]	'Sum of all moisture fluxes into the room and moisture sources.'
		NUM_R
	};

	/*! Constructor */
	RoomBalanceModel(unsigned int id, const std::string &displayName) :
		m_id(id), m_displayName(displayName)
	{
	}

	/*! Initializes model by providing simulation parameters and resizing the y and ydot vectors. */
	void setup( const NANDRAD::SimulationParameter &simPara);


	// *** Re-implemented from AbstractModel

	/*! Room balance model can be referenced via Zone and ID. */
	virtual NANDRAD::ModelInputReference::referenceType_t referenceType() const override {
		return NANDRAD::ModelInputReference::MRT_ZONE;
	}

	/*! Return unique class ID name of implemented model. */
	virtual const char * ModelIDName() const override { return "RoomBalanceModel";}

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
	/*! Zone ID. */
	unsigned int									m_id;
	/*! Display name (for error messages). */
	std::string										m_displayName;
	/*! True if moisture balance is enabled. */
	bool											m_moistureBalanceEnabled;
	/*! Data cache for calculated results (updated in call to update()).
		Index matches enum values of Results.
	*/
	std::vector<double>								m_results;

	/*! Input references used by this object.
		Inputs are populated in the following order:
			- heat conduction [W] towards all walls (size m_heatCondValueRefs)
	*/
	std::vector<InputReference>						m_inputRefs;

	/*! Value references for heat conduction fluxes in [W] (positive if out-of-room). */
	std::vector<const double *>						m_heatCondValueRefs;
	/*! Value references for window heat conduction fluxes in [W] (positive if out-of-room). */
	std::vector<const double *>						m_windowHeatCondValueRefs;
	/*! Value references for window solar radiation fluxes in [W] (positive if out-of-room). */
	std::vector<const double *>						m_windowSolarRadiationLoadsRefs;
	/*! Number of infiltration model input refs that we have generated and that we get value refs for. */
	unsigned int									m_infiltrationModelCount = 0;
	/*! Value reference for natural ventilation/infiltration flux in [W] (positive if into-room). */
	const double *									m_infiltrationValueRef = nullptr;

	/*! Vector with cached derivatives, updated at last call to update(). */
	std::vector<double>								m_ydot;

	/*! Constant pointer to the simulation parameter. */
	const NANDRAD::SimulationParameter				*m_simPara = nullptr;
};

} // namespace NANDRAD_MODEL

#endif // NM_RoomBalanceModelH
