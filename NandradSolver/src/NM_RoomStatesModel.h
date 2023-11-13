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

#ifndef NM_RoomStatesModelH
#define NM_RoomStatesModelH

#include "NM_AbstractModel.h"

namespace NANDRAD {
	class Zone;
	class SimulationParameter;
}

namespace NANDRAD_MODEL {

/*!	A model that computes zone states (temperature/humidity) out of a given energy density/RH

	The room state model updates all states from the y-vector and
	offers it for references from other models. This model is the
	complement to the RoomBalanceModel which evaluates all heat/moisture fluxes
	and calculate divergences(). As the RoomBalanceModel owns the
	integrator communication facility it also holds the vector of states y
	(internal energy etc.).

	Depending on the solver parametrization, this model either has only thermal states (energy density)
	or hygrothermal states (energy density + air moisture mass density). Depending on
	the simulation settings, the model offers different result quantities.

	\note This model is a "head"-type model and is evaluated first during NANDRAD model evalution. Hence, all
		results are "constant" for subsequent models.

	\todo Implement support for hygrothermal states.
*/
class RoomStatesModel : public AbstractModel {
public:

	/*! Results, published by the model.
		Availability depends on thermal/hygrothermal calculation modes.
	*/
	enum Results {
		R_AirTemperature,						// Keyword: AirTemperature				[C]		'Room air temperature'
		R_RelativeHumidity,						// Keyword: RelativeHumidity			[%]		'Room air relative humidity'
		R_VaporPressure,						// Keyword: VaporPressure				[Pa]	'Room air vapor pressure'
		R_AbsoluteHumidity,						// Keyword: AbsoluteHumidity			[kg/m3]	'Absolute air humidity per volume'
		R_SpecificHumidity,						// Keyword: SpecificHumidity			[kg/kg]	'Mass specific air humidity'
		NUM_R
	};

	/*! Constructor. */
	RoomStatesModel(unsigned int id, const std::string &displayName) :
		m_id(id), m_displayName(displayName)
	{
	}

	/*! Initializes model by providing zone parameters.
		The relevant parameters are cached in the RoomStatesModel results for access by other models.
		This function also checks for required parameters and tests for validity of values.
	*/
	void setup(const NANDRAD::Zone & zone, const NANDRAD::SimulationParameter & simPara);


	// *** Re-implemented from AbstractModel

	/*! Room state model can be referenced via Zone and ID. */
	virtual NANDRAD::ModelInputReference::referenceType_t referenceType() const override {
		return NANDRAD::ModelInputReference::MRT_ZONE;
	}

	/*! Return unique class ID name of implemented model. */
	virtual const char * ModelIDName() const override { return "RoomStatesModel"; }

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



	// *** Other public member functions

	/*! Returns the underlying zone. */
	const NANDRAD::Zone * zone() const {	return m_zone; }

	/*! Returns number of conserved variables (i.e. length of y vector passed to yInitial() and update() ). */
	unsigned int nPrimaryStateResults() const;

	/*! Returns a vector of dependencies of all result quantities from y input quantities). */
	void stateDependencies(std::vector< std::pair<const double *, const double *> > & resultInputValueReferences) const;

	/*! Sets initial states in y vector.
		This function is called after setup(), so that parameters needed for
		computing the initial condition are already present.

		\param Pointer to the memory array holding all initial states for this room model (to be written into).
			   The vector has either size 1 for thermal calculations or size 2 for hygrothermal calculations.
	*/
	void yInitial(double * y) const;

	/*! Computes room air temperature.
		This function is called directly from NandradModel as first step in the model evaluation.

		\param Pointer to the memory array holding all states for this room model.
			   The vector has either size 1 for thermal calculations or size 2 for hygrothermal calculations.
	*/
	int update(const double * y);

private:

	/*! Constant pointer to the referenced zone parameter block. */
	const NANDRAD::Zone								*m_zone = nullptr;
	/*! Reference to simulation parameter block. */
	const NANDRAD::SimulationParameter				*m_simPara = nullptr;

	/*! Zone ID. */
	unsigned int									m_id;
	/*! Display name (for error messages). */
	std::string										m_displayName;
	/*! True if moisture balance is enabled. */
	bool											m_moistureBalanceEnabled;

	/*! Zone air volume in [m3], cached in setup(). */
	double											m_volume = 999;
	/*! Additional heat capacity in zone in [J/K], cached in setup(). */
	double											m_additionalHeatCapacity = 999;

	/*! Data cache for calculated results (updated in call to update()).
		Index matches enum values of Results.
	*/
	std::vector<double>								m_results;

	/*! Cached input data vector (size nPrimaryStateResults()). */
	std::vector<double>								m_y;

};

} // namespace NANDRAD_MODEL

#endif // NM_RoomStatesModelH
