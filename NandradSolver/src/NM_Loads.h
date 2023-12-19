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

#ifndef NM_LoadsH
#define NM_LoadsH

#include <vector>
#include <string>
#include <functional>
#include <utility>
#include <map>

#include "NM_AbstractTimeDependency.h"
#include "NM_DefaultModel.h"

#include <CCM_SolarRadiationModel.h>

namespace NANDRAD {
	class Location;
	class SimulationParameter;
}


namespace NANDRAD_MODEL {


/*! Provides access to climatic loads.
	Note that Loads only provides time-dependent results and therefore defines all values
	as parameters rather than variables. For all other models these appear as constants
	since it is guaranteed that the Loads object is evaluated before any other models.

	The Loads model provides the result values in the Results enumeration, and sensor
	values defined in VectorValuedResults enum.

	Climatic loads and location data can be addressed for example by : "Location::Temperature"
	or "Location::SWRadOnPlane[id=2]".
*/
class Loads : public DefaultModel, public AbstractTimeDependency {
public:
	/*! Parameters/Variables that can be referenced from other models.
		\note By definition, the first enumeration values up to R_Declination are used for the CCD container
		m_dataSet.
	*/
	enum Results {
		R_Temperature,				// Keyword: Temperature					[C]		'Outside temperature.'
		R_RelativeHumidity,			// Keyword: RelativeHumidity			[%]		'Relative humidity.'
		R_SWRadDirectNormal,		// Keyword: SWRadDirectNormal			[W/m2]	'Direct short-wave radiation flux density in normal direction.'
		R_SWRadDiffuseHorizontal,	// Keyword: SWRadDiffuseHorizontal		[W/m2]	'Diffuse short-wave radiation flux density on horizontal surface.'
		R_LongWaveSkyRadiation,		// Keyword: LongWaveSkyRadiation		[W/m2]	'Long wave sky radiation.'
		R_WindDirection,			// Keyword: WindDirection				[Deg]	'Wind direction (0 - north).'
		R_WindVelocity,				// Keyword: WindVelocity				[m/s]	'Wind velocity.'
		R_AirPressure,				// Keyword: AirPressure					[Pa]	'Air pressure.'
		R_VaporPressure,			// Keyword: VaporPressure				[Pa]	'Ambient vapor pressure.'
		R_AbsoluteHumidity,			// Keyword: AbsoluteHumidity			[kg/m3]	'Absolute air humidity.'
		R_CO2Concentration,			// Keyword: CO2Concentration			[---]	'Ambient CO2 concentration.'
		R_CO2Density,				// Keyword: CO2Density					[kg/m3]	'Ambient CO2 density.'
		R_DeclinationAngle,			// Keyword: DeclinationAngle			[Deg]	'Solar declination (0 - north).'
		R_ElevationAngle,			// Keyword: ElevationAngle				[Deg]	'Solar elevation (0 - at horizont, 90 - zenith).'
		R_AzimuthAngle,				// Keyword: AzimuthAngle				[Deg]	'Solar azimuth (0 - north).'
		R_Albedo,					// Keyword: Albedo						[---]	'Albedo value of the surrounding [0..1].'
		R_Latitude,					// Keyword: Latitude					[Deg]	'Latitude.'
		R_Longitude,				// Keyword: Longitude					[Deg]	'Longitude.'
		NUM_R
	};

	/*! Optional sensor values. */
	enum VectorValuedResults {
		VVR_DirectSWRadOnPlane,				// Keyword: DirectSWRadOnPlane				[W/m2]	'Direct short wave radiation on a given plane.'
		VVR_DiffuseSWRadOnPlane,			// Keyword: DiffuseSWRadOnPlane				[W/m2]	'Diffuse short wave radiation on a given plane.'
		VVR_GlobalSWRadOnPlane,				// Keyword: GlobalSWRadOnPlane				[W/m2]	'Global short wave radiation on a given plane.'
		VVR_IncidenceAngleOnPlane,			// Keyword: IncidenceAngleOnPlane			[Deg]	'The incidence angle of the suns ray onto the surface (0 deg = directly perpendicular).'
		NUM_VVR
	};

	// *** PUBLIC MEMBER FUNCTIONS

	/*! Constructor, initializes data vectors with 0, so that calculation is always possible. */
	Loads() : DefaultModel(0, "Loads") {}

	/*! Initializes object.
		This function checks for parameters.
		\param location Location data.
		\param pathPlaceHolders Path placeholders to resolve path to climate data
	*/
	void setup(const NANDRAD::Location & location, const NANDRAD::SimulationParameter &simPara,
				const std::map<std::string, IBK::Path> & pathPlaceHolders) ;


	// *** Re-implemented from AbstractModel

	/*! Climatic loads can be referenced as MRT_LOCATION. */
	virtual NANDRAD::ModelInputReference::referenceType_t referenceType() const override {
		return NANDRAD::ModelInputReference::MRT_LOCATION;
	}

	/*! Return unique class ID name of implemented model. */
	virtual const char * ModelIDName() const override { return "Loads"; }

	/*! Populates the vector resDesc with descriptions of all results provided by this model.
		Results are all quantities listed in Results enum, and also a vector-valued quantity
		SWRadOnPlane with radiation sensor values.
	*/
	virtual void resultDescriptions(std::vector<QuantityDescription> & resDesc) const override;

	/*! Resizes m_results vector. */
	virtual void initResults(const std::vector<AbstractModel*> & /* models */) override;


	// *** Re-implemented from AbstractTimeDependency

	/*! Updates the state of the loads object to the time point stored in DefaultTimeStateModel::m_t.
		This function updates all internally cached results to match the new time point.
		Afterwards, these time points can be retrieved very efficiently several times
		through the various access functions.

		\param t The simulation time (relative to simulation start) in s.

		\note The simulation time is shifted by the offset from start year and start time to get an
			  absolute time reference. Then it is passed to the climate calculation module.
	*/
	virtual int setTime(double t) override;


	// *** Other public member functions

	/*! Adds a surface to the list of computable surfaces.
		For all surfaces added to the loads class, the computation function calculates radiation loads.
		You can call this function multiply times with the same arguments, it will only add each surface once.

		This function is usually called when a construction with outside surface is instantiated.

		\param objectID Either constructionInstance or emebeddedObject id
		\param orientation	Orientation of the surface in [deg] (0 - north, 90 - east, ...).
		\param inclination	Inclination of the surface in [deg] in range [0..180] (0 - roof, 90 - wall, 180 - facing downwards).

		\warning For now, rounding errors in orientation and inclination are not handled properly.
				So be careful to pass exactly the same orientation and inclination to
				this function and the query function below. Otherwise small rounding errors will cause
				the function to register multiply surfaces for nearly same orientations, and do unnecessary calculations.

		\return Returns the ID of the new surface (can also be used to query results).
	*/
	void addSurface(unsigned int objectID, double orientationInDeg, double inclinationInDeg);

	/*! Returns the direct and diffuse radiation on a given surface. For sensors, always unshaded
		radiation loads are returned. For all other surfaces (opaque surface of a construction instance,
		an embedded object) the direct radiation includes shading from external objects.
		\param objectID		Sensor id, construction instance id or emebdded obejct id
		\param qRadDir		Here the direct radiation component is stored in [W/m2] (mean direct radiation including
							shading at windows, sensors or opaque surface).
		\param qRadDiff		Here the diffuse radiation component is stored in [W/m2].
		\param incidenceAngle Incidence angle onto surface in [rad].
		\return				The function returns the total (global) solar radiation on the surface in [W/m2] including
							external shading.
	*/
	double qSWRad(unsigned int objectID, double & qRadDir, double & qRadDiff, double & incidenceAngle) const;

	/*! Returns the long wave radiation from sky and ground on a given surface without considering the emmissivity of the surface.
		\param objectID		Sensor id, construction instance id or emebdded obejct id
		\return				The function returns the sum of long wave radiation from sky and ground
	*/
	double qLWRad(unsigned int objectID) const;

	/*! Returns sky visibility of a given surface.
		This function works essentially as the function above, but identifies the surface via a surfaceId.
		\param objectID		Model object id
		\return				The function returns the total long wave radiation on the surface in [W/m2].
	*/
	double skyVisibility(unsigned int objectID) const;

private:
	/*! Year of simulation. */
	int										m_year = 0;
	/*! Time from the beginning of the year in [s]. */
	double									m_startTime = 0;
	/*! Simulation time.*/
	double									m_t = 0;

	/*! The solar radiation model from CCM, includes the Climate data loader. */
	CCM::SolarRadiationModel				m_solarRadiationModel;
	/*! Mapping of object id to a solar radiation surface id from CCM. */
	std::map<unsigned int, unsigned int>	m_objectID2surfaceID;
	/*! Mapping of sensor id to a solar radiation surface id from CCM. */
	std::map<unsigned int, unsigned int>	m_sensorID2surfaceID;
	/*! Mapping of object id to a long wave radiation surface id. */
	std::map<unsigned int, unsigned int>	m_objectID2inclinations;
	/*! Vector of surfaces with different inclination for calculation of
		sky long wave radiation (in [rad]).
	*/
	std::vector< double >					m_inclinations;
	/*! Vector of sky visibility for all inclinations (in [---]). */
	std::vector< double >					m_skyVisbility;

	/*! Set to false, if shading factor file contains continuous data. */
	bool									m_cyclicShadingFactors = true;

	/*! Vector containing shading/reduction factors for current time point.
		These are updated in setTime() and used in qSWRad() when computing direct radiation loads (and global radiation).
		Diffuse radiation loads are currently not reduced due to shading.
	*/
	std::vector<double>						m_shadingFactors;
	/*! Maps that related construction instance/embedded object ids to the storage locations of the respective shading/reduction factors. */
	std::map<unsigned int, const double*>	m_shadingFactorsForObjectID;

	/*! All time points (offset to midnight January 1st of the start year) that we have external shading factors for. */
	std::vector<double>						m_externalShadingFactorTimePoints;
	/*! Stores the IDs (construction instance/embedded object) that we have external shading factors for. */
	std::vector<unsigned int>				m_externalShadingFactorIDs;
	/*! Matrix with time series data of the external shading factors. Rows hold values per time point
		(size m_externalShadingFactorTimePoints.size()) with values for each surface (size m_externalShadingFactorIDs.size()).
		\code
		double shadingFactor = m_externalShadingFactors[timePointIdx][surfaceColumnIndex];
		\endcodde
	*/
	std::vector< std::vector<double> >		m_externalShadingFactors;
};

} // namespace NANDRAD_MODEL

#endif // NM_LoadsH
