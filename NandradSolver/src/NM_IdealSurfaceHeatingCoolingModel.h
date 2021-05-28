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

#ifndef NM_IdealSurfaceHeatingCoolingModelH
#define NM_IdealSurfaceHeatingCoolingModelH

#include "NM_AbstractModel.h"
#include "NM_AbstractStateDependency.h"
#include "NM_VectorValuedQuantity.h"

#include <NANDRAD_ObjectList.h>
#include <NANDRAD_ConstructionInstance.h>

namespace NANDRAD {
	class IdealSurfaceHeatingCoolingModel;
	class Thermostat;
	class Zone;
}

namespace NANDRAD_MODEL {

/*! A model for ideal heating/cooling via surface. We control thermal active layer load (positive or negative).
	In the case of heating parametrised 'MaxHeatingPowerPerArea' with a positive value and the
	same for 'MaxCoolingPowerPerArea' in the case of cooling (heating and cooling may be used
	combined).
	Control values for heating and cooling are retrieved from zone specific thermostat model.
*/
class IdealSurfaceHeatingCoolingModel : public AbstractModel, public AbstractStateDependency {
public:
	/*! Computed results, vector-valued results that provide access via construction ID. */
	enum VectorValuedResults {
		/*! Heat load into active layer, positive heating, negative cooling */
		VVR_ActiveLayerThermalLoad,		// Keyword: ActiveLayerThermalLoad			[W]		'Active layer thermal load'
		NUM_VVR
	};

	// *** PUBLIC MEMBER FUNCTIONS

	/*! Constructor. */
	IdealSurfaceHeatingCoolingModel(unsigned int id, const std::string &displayName) :
		m_id(id), m_displayName(displayName)
	{
	}

	/*! Initializes object.
		\param model Model data.
		\param objLists The object list stored in the project file (persistent, remains unmodified so that persistent
			pointers to object list elements can be stored).
	*/
	void setup(const NANDRAD::IdealSurfaceHeatingCoolingModel & model,
			   const std::vector<NANDRAD::ObjectList> & objLists);

	// *** Re-implemented from AbstractModel

	/*! Thermal ventilation loads can be requested via MODEL reference. */
	virtual NANDRAD::ModelInputReference::referenceType_t referenceType() const override {
		return NANDRAD::ModelInputReference::MRT_MODEL;
	}

	/*! Return unique class ID name of implemented model. */
	virtual const char * ModelIDName() const override { return "IdealSurfaceHeatingCoolingModel"; }

	/*! Returns unique ID of this model instance. */
	virtual unsigned int id() const override { return m_id; }

	void initResults(const std::vector<AbstractModel *> &) override;

	/*! Populates the vector resDesc with descriptions of all results provided by this model. */
	virtual void resultDescriptions(std::vector<QuantityDescription> & resDesc) const override;

	/*! Retrieves reference pointer to a value with given input reference name. */
	virtual const double * resultValueRef(const InputReference & quantity) const override;


	// *** Re-implemented from AbstractStateDependency

	/*! Composes all input references. */
	virtual void initInputReferences(const std::vector<AbstractModel*> & models) override;

	/*! Returns vector with model input references.
		Implicit models must generate their own model input references and populate the
		vector argument.
		\note This function is not the fastest, so never call this function from within the solver
		(except maybe for output writing).
	*/
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

	/*! Quick access pointer to object list (for targeted construction instances). */
	const NANDRAD::ObjectList						*m_objectList = nullptr;

	/*! Cached heating power per surface area in [W/m2] */
	double											m_maxHeatingPower = 666;

	/*! Cached cooling power per surface area in [W/m2] */
	double											m_maxCoolingPower = 666;

	/*! Cached thermostat zone id. */
	unsigned int									m_thermostatZoneId = NANDRAD::INVALID_ID;

	/*! Holds number of thermostat model objects that values were requested from. */
	unsigned int									m_thermostatModelObjects = 0;

	/*! Reference to heating thermotat control value. */
	const double*									m_heatingThermostatValueRef = nullptr;

	/*! Reference to cooling thermotat control value. */
	const double*									m_coolingThermostatValueRef = nullptr;

	/*! Construction areas in [m2]. */
	std::vector<double>								m_constructionAreas;

	/*! Vector valued results, computed/updated during the calculation. */
	std::vector<VectorValuedQuantity>				m_vectorValuedResults;

	/*! Vector with input references.
		For each thermostat model found, this vector contains at two input refs; for each thermostat/zone
		a heating/cooling control value is requested.
	*/
	std::vector<InputReference>						m_inputRefs;

};

} // namespace NANDRAD_MODEL

#endif // NM_IdealSurfaceHeatingCoolingModelH
