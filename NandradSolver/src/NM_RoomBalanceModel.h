/*	The Nandrad model library.

Copyright (c) 2012, Institut fuer Bauklimatik, TU Dresden, Germany

Written by
A. Paepcke		<anne.paepcke -[at]- tu-dresden.de>
A. Nicolai		<andreas.nicolai -[at]- tu-dresden.de>
All rights reserved.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 3 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.
*/

#ifndef RoomBalanceModelH
#define RoomBalanceModelH

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

	enum Results {
		R_CompleteThermalLoad,								// Keyword: CompleteThermalLoad							[W]		'Sum of all thermal fluxes into the room.'
		NUM_R
	};

	enum InputReferences {
		InputRef_RadiationLoadFraction,						// Keyword: RadiationLoadFraction						[%]		'Percentage of solar radiation gains attributed direcly to current room.'
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
	/*! Copy constructor is not available (disable copy). */
	RoomStatesModel(const RoomStatesModel &) = delete;
	/*! Assignment operator is not available (disable copy). */
	const RoomStatesModel & operator=(const RoomStatesModel &) = delete;

	/*! Initializes model by providing simulation parameters and resizing the y and ydot vectors. */
	void setup( const NANDRAD::SimulationParameter &simPara);


	// *** Re-implemented from AbstractModel

	/*! Room balance model can be referenced via Zone and ID. */
	virtual NANDRAD::ModelInputReference::referenceType_t referenceType() const override {
		return NANDRAD::ModelInputReference::MRT_ZONE;
	}

	/*! Return unique class ID name of implemented model. */
	virtual const char * ModelIDName() const override { return "RoomBalanceModel";}

	/*! Populates the vector resDesc with descriptions of all results provided by this model. */
	virtual void resultDescriptions(std::vector<QuantityDescription> & resDesc) const override;

	/*! Returns vector of all scalar and vector valued results pointer. */
	virtual void resultValueRefs(std::vector<const double *> &res) const override;

	/*! Retrieves reference pointer to a value with given quantity ID name.
		\return Returns pointer to memory location with this quantity, otherwise nullptr if parameter ID was not found.
	*/
	virtual const double * resultValueRef(const QuantityName & quantityName) const override;

	/*! Resizes m_results vector.*/
	virtual void initResults(const std::vector<AbstractModel*> & models) override;


	// *** Other public member functions

	/*! Returns constant reference to the vector of solver states.*/
	const double *y() const { return &m_y[0]; }

	/*! Returns number of primary state results (number of unknows). */
	unsigned int nPrimaryStateResults() const { return 1; }

	/*! Composes all input references.*/
//	void initInputReferences(const std::vector<AbstractModel*> & models);

	/*! Adds dependencies between ydot and y to default pattern. */
//	void stateDependencies(std::vector< std::pair<const double *, const double *> > &resultInputValueReferences) const;

	/*! Computes divergence of balance equations. */
	int update();

	/*! Sets new states by passing a linear memory array with the room states.
		For now, the vector y only has a single value, the energy density.
		This function is called from the main model interface setY() function.
	*/
	void setY(const double * y);

	/*! Stores the divergences of all balance equations in vector ydot. */
	int ydot(double* ydot);


protected:
	/*! Zone ID. */
	unsigned int									m_id;
	/*! Display name (for error messages). */
	std::string										m_displayName;
	/*! Data cache for calculated results (updated in call to update()).
		Index matches enum values of Results.
	*/
	std::vector<double>								m_results;

	/*! Vector with cached states, updated at last call to setY(). */
	std::vector<double>								m_y;
	/*! Vector with cached derivatives, updated at last call to update(). */
	std::vector<double>								m_ydot;

	/*! Constant pointer to the simulation parameter. */
	const NANDRAD::SimulationParameter				*m_simulationParameter = nullptr;
};

} // namespace NANDRAD_MODEL

#endif // RoomBalanceModelH
