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

#ifndef NM_IdealPipeRegisterModelH
#define NM_IdealPipeRegisterModelH

#include "NM_AbstractModel.h"
#include "NM_AbstractStateDependency.h"
#include "NM_VectorValuedQuantity.h"

#include <IBK_LinearSpline.h>

namespace NANDRAD {
	class IdealPipeRegisterModel;
	class Thermostat;
	class Zone;
	class ObjectList;
}

namespace NANDRAD_MODEL {

/*! A model for an ideal pipe register heating and cooling. We regulate mass flow based on
	a zone thermostat control. We also compare supply temperature to layer temperature. In
	the case of heating only a larger supply temperature opens valve and in the case of cooling
	only a smaller one.
	For computation of heat tranfer we use steady-state model with constant pipe mass flow
	and constant environmental temperature.
	Control values for heating and cooling are retrieved from zone specific thermostat model.

	Note: in order to deactivate heating or cooling remove 'HeatingSetpoint' and 'CoolingSetpoint'
	from Thermostat parametrisation or chose a high/low supply temperature.*/
class IdealPipeRegisterModel : public AbstractModel, public AbstractStateDependency {
public:
	/*! Computed results, vector-valued results that provide access via construction ID. */
	enum VectorValuedResults {
		VVR_MassFlux,					// Keyword: MassFlux						[kg/s]	'Controlled mass flow'
		/*! Heat load into active layer, positive heating, negative cooling */
		VVR_ActiveLayerThermalLoad,		// Keyword: ActiveLayerThermalLoad			[W]		'Active layer thermal load'
		/*! Return temperature from pipe register */
		VVR_ReturnTemperature,			// Keyword: ReturnTemperature				[C]		'Return temperature from pipe register'
		NUM_VVR
	};

	// *** PUBLIC MEMBER FUNCTIONS

	/*! Constructor. */
	IdealPipeRegisterModel(unsigned int id, const std::string &displayName) :
		m_id(id), m_displayName(displayName)
	{
	}

	/*! Initializes object.
		\param model Model data.
		\param objLists The object list stored in the project file (persistent, remains unmodified so that persistent
			pointers to object list elements can be stored).
	*/
	void setup(const NANDRAD::IdealPipeRegisterModel & model,
			   const std::vector<NANDRAD::ObjectList> & objLists);

	// *** Re-implemented from AbstractModel

	/*! Thermal ventilation loads can be requested via MODEL reference. */
	virtual NANDRAD::ModelInputReference::referenceType_t referenceType() const override {
		return NANDRAD::ModelInputReference::MRT_MODEL;
	}

	/*! Return unique class ID name of implemented model. */
	virtual const char * ModelIDName() const override { return "IdealPipeRegisterModel"; }

	/*! Returns unique ID of this model instance. */
	virtual unsigned int id() const override { return m_id; }

	/*! Resizes m_results vector. */
	virtual void initResults(const std::vector<AbstractModel*> & /* models */) override;

	/*! Populates the vector resDesc with descriptions of all results provided by this model. */
	virtual void resultDescriptions(std::vector<QuantityDescription> & resDesc) const override;

	/*! Retrieves reference pointer to a value with given input reference name. */
	virtual const double * resultValueRef(const InputReference & quantity) const override;


	// *** Re-implemented from AbstractStateDependency

	/*! Composes all input references.*/
	virtual void initInputReferences(const std::vector<AbstractModel*> & models) override;

	/*! Returns vector with model input references.*/
	virtual void inputReferences(std::vector<InputReference>  & inputRefs) const override;

	/*! Provides the object with references to requested input variables (persistent memory location). */
	virtual void setInputValueRefs(const std::vector<QuantityDescription> &,
								   const std::vector<const double *> & resultValueRefs) override;

	/*! Returns dependencies between result variables and input variables. */
	virtual void stateDependencies(std::vector< std::pair<const double *, const double *> > & resultInputValueReferences) const override;

	/*! Sums up all provided input quantities and computes divergence of balance equations. */
	int update() override;

private:
	/*! Model instance ID (unused since results are provided for zones). */
	unsigned int									m_id;

	/*! Display name (for error messages). */
	std::string										m_displayName;

	/*! Pointer to model parametrization. */
	const NANDRAD::IdealPipeRegisterModel*			m_model = nullptr;

	/*! Quick access pointer to object list (for targetted construction instances). */
	const NANDRAD::ObjectList						*m_objectList = nullptr;

	/*! Hydraulic (inner) diameter of pipe in [m] */
	double											m_innerDiameter = -999;

	/*! Pipe length in [m] */
	double											m_length = -999;

	/*! Fluid heat capacity [J/kgK].*/
	double											m_fluidHeatCapacity = -999;

	/*! Fluid density [kg/m3].*/
	double											m_fluidDensity = -999;

	/*! Fluid volume [m3]. */
	double											m_fluidVolume = -999;

	/*! Effective flow cross-section [m2]. */
	double											m_fluidCrossSection = -999;

	/*! Fluid conductivity [W/mK].*/
	double											m_fluidConductivity = 0.01;

	/*! Fluid dynamic viscosity [m/s] (temperature dependend).*/
	IBK::LinearSpline								m_fluidViscosity;

	/*! Equivalent u-value of the pipe wall and insulation per length of pipe in [W/mK] */
	double											m_UValuePipeWall;

	/*! Number of parallel pipes (=1 per default).*/
	unsigned int									m_nParallelPipes;
	/*! Cached thermostat zone id. */
	unsigned int									m_thermostatZoneId = NANDRAD::INVALID_ID;

	/*! Holds number of thermostat model objects that values were requested from.
		Note: since we don't know which thermostat model instance generates the results for
			  the zone we are interested in, we create input references to all and count here
			  the total number of adressed objects.
	*/
	unsigned int									m_thermostatModelObjects = 0;

	/*! Reference to heating thermostat control value. */
	const double*									m_heatingThermostatValueRef = nullptr;

	/*! Reference to cooling thermostat control value. */
	const double*									m_coolingThermostatValueRef = nullptr;

	/*! Reference to supply temperature in [W]. */
	std::vector<const double*>						m_supplyTemperatureRefs;

	/*! Reference for maximum mass flux (either constant, scheduled or FMI input). */
	std::vector<const double*>						m_maxMassFlowRefs;

	/*! Reference to active layer temperatures in [W]. */
	std::vector<const double*>						m_activeLayerTemperatureRefs;

	/*! Vector valued results, computed/updated during the calculation. */
	std::vector<VectorValuedQuantity>				m_vectorValuedResults;

	/*! Vector with input references.
		For each thermostat model found, this vector contains 2*number of zones input refs, for each zone
		a heating and cooling control value is requested.
	*/
	std::vector<InputReference>						m_inputRefs;

};

} // namespace NANDRAD_MODEL

#endif // NM_IdealPipeRegisterModelH
