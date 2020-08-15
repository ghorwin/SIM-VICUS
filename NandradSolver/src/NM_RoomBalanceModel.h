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
}

#include <NANDRAD_Zone.h>

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
		R_CompleteMoistureLoad,								// Keyword: CompleteMoistureLoad						[kg/s]	'Sum of all moisture fluxes into the room and moisture sources.'
		NUM_R
	};

	/*! Fluxes/loads types taken as input.
		Note: some are scalars and some are expanded to several flux input quantities depending on number of models providing these values.
	*/
	enum InputReferences {
		InputRef_WallsHeatConductionLoad,					// Keyword: WallsHeatConductionLoad						[W]		'Heat load by heat conduction through all enclosing walls.'
		InputRef_WindowsSWRadLoad,							// Keyword: WindowsSWRadLoad							[W]		'Heat loads by short wave radiation through all windows of a room.'
		InputRef_WindowsHeatTransmissionLoad,				// Keyword: WindowsHeatTransmissionLoad					[W]		'Heat loads by heat transmission through all windows of a room.'
		InputRef_LWRadBalanceLoad,							// Keyword: LWRadBalanceLoad							[W]		'Balance loads by long wave radiation exchange on all window inside surfaces.'
		InputRef_SWRadBalanceLoad,							// Keyword: SWRadBalanceLoad							[W]		'Balance loads by short wave radiation exchange on all window inside surfaces.'
		InputRef_ConvectiveHeatingsLoad,					// Keyword: ConvectiveHeatingsLoad						[W]		'Heat loads by convective heating.'
		InputRef_ConvectiveCoolingsLoad,					// Keyword: ConvectiveCoolingsLoad						[W]		'Heat loss by convective cooling.'
		InputRef_ConvectiveUsersLoad,						// Keyword: ConvectiveUsersLoad							[W]		'Loads by occupancy.'
		InputRef_ConvectiveEquipmentLoad,					// Keyword: ConvectiveEquipmentLoad						[W]		'Electic equipment loads.'
		InputRef_ConvectiveLightingLoad,					// Keyword: ConvectiveLightingLoad						[W]		'Heat gains by lighting.'
		InputRef_UserVentilationThermalLoad,				// Keyword: UserVentilationThermalLoad					[W]		'Heat load by air ventilation.'
		InputRef_InfiltrationThermalLoad,					// Keyword: InfiltrationThermalLoad						[W]		'Heat load by infiltration.'
		InputRef_AirConditionThermalLoad,					// Keyword: AirConditionThermalLoad						[W]		'Heat load by air conditioning.'
		InputRef_DomesticWaterConsumptionSensitiveHeatGain,	// Keyword: DomesticWaterConsumptionSensitiveHeatGain	[W]		'Sensitive heat gain towards the room by water consumption.'
		NUM_InputRef
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
		\return Returns pointer to memory location with this quantity, otherwise nullptr if parameter ID was not found.
	*/
	virtual const double * resultValueRef(const QuantityName & quantityName) const override;


	// *** Re-implemented from AbstractStateDependency


	/*! Composes all input references.*/
	virtual void initInputReferences(const std::vector<AbstractModel*> & /* models */) override;

	/*! Returns vector with model input references.
		Implicit models must generate their own model input references and populate the
		vector argument.
		\note This function is not the fastest, so never call this function from within the solver
		(except maybe for output writing).
	*/
	virtual void inputReferences(std::vector<InputReference>  & inputRefs) const override;

	/*! Returns vector with pointers to memory locations matching input value references. */
	virtual const std::vector<const double *> & inputValueRefs() const override;

	/*! Sets a single input value reference (persistent memory location) that refers to the requested input reference.
		\param inputRef An input reference from the previously published list of input references.
		\param resultValueRef Persistent memory location to the variable slot.
	*/
	virtual void setInputValueRef(const InputReference &inputRef, const QuantityDescription & resultDesc, const double *resultValueRef) override;

	/*! Sums up all provided input quantities and computes divergence of balance equations. */
	int update() override;


	// *** Other public member functions

	/*! Stores the divergences of all balance equations in this zone in vector ydot. */
	int ydot(double* ydot);


protected:
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

	/*! Vector with pointer to input values.
		This vector is resized and filled by the framework.
		\sa inputValueRefs()
	*/
	std::vector<const double *>						m_inputValueRefs;


	/*! Vector with cached derivatives, updated at last call to update(). */
	std::vector<double>								m_ydot;

	/*! Constant pointer to the simulation parameter. */
	const NANDRAD::SimulationParameter				*m_simPara = nullptr;
};

} // namespace NANDRAD_MODEL

#endif // NM_RoomBalanceModelH
