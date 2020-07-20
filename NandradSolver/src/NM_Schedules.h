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

#ifndef NM_SchedulesH
#define NM_SchedulesH

#include <vector>
#include <string>
#include <functional>
#include <utility>
#include <map>

#include <IBK_Time.h>

//#include "NM_ScheduleDays.h"
#include "NM_AbstractModel.h"
#include "NM_AbstractTimeDependency.h"
//#include "NM_ScheduleParameter.h"
#include "NM_VectorValuedQuantity.h"

namespace NANDRAD {
	class AnnualSchedules;
	class Schedules;
	class Schedule;
	class SpaceTypes;
	class DailyCycle;
	class Project;
}

namespace NANDRAD_MODEL {


/*! Provides access to scheduled quantities.
	Schedules provide by default only time-dependent values with no dependence on other
	objects. Also, the evaluation is guaranteed to be done prior to any other models
	Therefore the m_results vector is populated.
*/
class Schedules : public AbstractModel, public AbstractTimeDependency {
public:

	/*! Available quantities from schedules. */
	enum Results {
		R_HeatingSetPointTemperature,			// Keyword: HeatingSetPointTemperature				[C]			'Setpoint temperature for heating.'
		R_CoolingSetPointTemperature,			// Keyword: CoolingSetPointTemperature				[C]			'Setpoint temperature for cooling.'
		R_AirConditionSetPointTemperature,		// Keyword: AirConditionSetPointTemperature			[C]			'Setpoint temperature for air conditioning.'
		R_AirConditionSetPointRelativeHumidity,	// Keyword: AirConditionSetPointRelativeHumidity	[%]			'Setpoint relative humidity for air conditioning.'
		R_AirConditionSetPointMassFlux,			// Keyword: AirConditionSetPointMassFlux			[kg/s]		'Setpoint mass flux for air conditioning.'
		R_HeatingLoad,							// Keyword: HeatingLoad								[W]			'Heating load.'
		R_ThermalLoad,							// Keyword: ThermalLoad								[W]			'Thermal load (positive or negative).'
		R_MoistureLoad,							// Keyword: MoistureLoad							[g/h]		'Moisture load.'
		R_CoolingPower,							// Keyword: CoolingPower							[W]			'Cooling power.'
		R_LightingPower,						// Keyword: LightingPower							[W]			'Lighting power.'
		R_DomesticWaterSetpointTemperature,		// Keyword: DomesticWaterSetpointTemperature		[C]			'Setpoint temperature for domestic water.'
		R_DomesticWaterMassFlow,				// Keyword: DomesticWaterMassFlow					[kg/s]		'Domestic water demand mass flow for the complete zone (hot water and equipment).'
		R_ThermalEnergyLossPerPerson,			// Keyword: ThermalEnergyLossPerPerson				[W/Person]	'Energy of a single persons activities that is not available as thermal heat.'
		R_TotalEnergyProductionPerPerson,		// Keyword: TotalEnergyProductionPerPerson			[W/Person]	'Total energy production of a single persons body at a certain activity.'
		R_MoistureReleasePerPerson,				// Keyword: MoistureReleasePerPerson				[kg/s]		'Moisture release of a single persons body at a certain activity.'
		R_CO2EmissionPerPerson,					// Keyword: CO2EmissionPerPerson					[kg/s]		'CO2 emission mass flux of a single person at a certain activity.'
		R_MassFluxRate,							// Keyword: MassFluxRate							[---]		'Fraction of real mass flux to maximum  mass flux for different day times.'
		R_PressureHead,							// Keyword: PressureHead							[Pa]		'Supply pressure head of a pump.'
		R_OccupancyRate,						// Keyword: OccupancyRate							[---]		'Fraction of real occupancy to maximum  occupancy for different day times.'
		R_EquipmentUtilizationRatio,			// Keyword: EquipmentUtilizationRatio				[---]		'Ratio of usage for existing electric equipment.'
		R_LightingUtilizationRatio,				// Keyword: LightingUtilizationRatio				[---]		'Ratio of usage for lighting.'
		R_MaximumSolarRadiationIntensity,		// Keyword: MaximumSolarRadiationIntensity			[W/m2]		'Maximum solar radiation intensity before shading is activated.'
		R_UserVentilationAirChangeRate,			// Keyword: UserVentilationAirChangeRate			[1/h]		'Exchange rate for natural ventilation.'
		R_UserVentilationComfortAirChangeRate,	// Keyword: UserVentilationComfortAirChangeRate		[1/h]		'Maximum air change rate = offset for user comfort.'
		R_UserVentilationMinimumRoomTemperature,// Keyword: UserVentilationMinimumRoomTemperature	[C]			'Temperature limit over which comfort ventilation is activated.'
		R_UserVentilationMaximumRoomTemperature,// Keyword: UserVentilationMaximumRoomTemperature	[C]			'Temperature limit below which comfort ventilation is activated.'
		R_InfiltrationAirChangeRate,			// Keyword: InfiltrationAirChangeRate				[1/h]		'Exchange rate for infiltration.'
		R_ShadingFactor,						// Keyword: ShadingFactor							[---]		'Shading factor [0...1].'
		NUM_R
	};



	/*! Generate variable reference list from defined schedules. */
	void setup(const NANDRAD::Project & project) ;

	/*! Return unique class ID name of implemented model. */
	virtual const char * ModelIDName() const { return "Schedules"; }

	/*! Schedules always have the id 0. */
	virtual unsigned int id() const {return 0;}

	/*! Returns display name of this abstract state model. */
	virtual const std::string &displayName() const { return m_displayName;  }

	/*! Schedules can be referenced via MRT_SCHEDULE. */
	virtual NANDRAD::ModelInputReference::referenceType_t referenceType() const {
		return NANDRAD::ModelInputReference::MRT_SCHEDULE;
	}

	/*! Returns vector of all scalar and vector valued results pointer.
	*/
	virtual void resultValueRefs(std::vector<const double *> &res) const;

	/*! Retrieves reference pointer to a value with given quantity ID name.
		\param quantityName An identifier for the quantity of format '<spaceTypeID>:<quantityID>'
		\return Returns pointer to memory location with this quantity, otherwise NULL if parameter ID was not found.
	*/
	virtual const double * resultValueRef(const QuantityName & quantityName) const;

	/*! Populates the vector refDesc with descriptions of all parameters defined in the schedules.
		\warning This function generates and populates the vector refDesc from scratch and
			is not the fastest. If you need to access the reference description several
			times, consider caching the resulting vector.
	*/
	virtual void resultDescriptions(std::vector<QuantityDescription> & resDesc) const;

	/*! Updates the state of the schedules object to the time point stored in AbstractTimeStateObject::m_t.
		This function updates all internally cached results to match the new time point.
		Afterwards, these time points can be retrieved very efficiently several times
		through the various access functions.
	*/
	virtual int setTime(double t);

	/*! This function is ignored.
	*/
	virtual void initResults(const std::vector<AbstractModel*> &  models);

	/*! Returns start value of a quantity. Throws an exception, if quantity does not exist.
	*/
	double startValue(const QuantityName & quantityName) const;

	/*! Constant access to schedule. */
	const NANDRAD::Schedules *schedules() const;

private:
	/*! Display name stored for return function. */
	std::string												m_displayName = "Schedules";
	/*! Pointer to NANDRAD project. */
	const NANDRAD::Project*								   m_project = nullptr;
	/*! Pointer to start time from the beginning of the year in [s]. */
	const double											m_startTime = 0;
	/*! Variables, computed/updated during the calculation. */
	std::vector<IBK::Parameter>								m_results;
	/*! Schedule parameters. */
//	std::vector<ScheduleParameter>							m_scheduleParameters;
	/*! Annual schedule parameters. */
//	std::vector<NANDRAD::LinearSplineParameter>				m_annualScheduleParameters;
	/*! Schedule time definition key: provides encoding and decoding
		of schedule time definition. */
//	ScheduleDays											m_scheduleDays;
};


} // namespace NANDRAD_MODEL

#endif // NM_SchedulesH
