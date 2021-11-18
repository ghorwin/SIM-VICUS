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

#ifndef NM_IdealHeatingCoolingModelH
#define NM_IdealHeatingCoolingModelH

#include "NM_AbstractModel.h"
#include "NM_AbstractStateDependency.h"
#include "NM_AbstractTimeDependency.h"
#include "NM_VectorValuedQuantity.h"

#include <NANDRAD_ObjectList.h>

namespace NANDRAD {
	class IdealHeatingCoolingModel;
	class Zone;
}

namespace NANDRAD_MODEL {

/*! A model for ideal heating/cooling (air heating).
	The model instance is identified by reference type MODEL and the id of the NANDRAD model parametrization block.
	It implements ideal heating/cooling loads for all zones referenced in the object list.

	For each zone in the object list, the model requests HeatingControlValue and CoolingControlValue quantities, that
	are provided by thermostat models. Also, it retrieves the zone's net floor area and computed together with
	parameters from NANDRAD::IdealHeatingCoolingModel the maximum heating/cooling power. The actual heating/cooling power
	is obtained by multiplication of the heating/cooling control value with this maximum power.
*/
class IdealHeatingCoolingModel : public AbstractModel, public AbstractStateDependency, public AbstractTimeDependency {
public:
	/*! Computed results, vector-valued results that provide access via zone ID. */
	enum VectorValuedResults {
		VVR_IdealHeatingLoad,				// Keyword: IdealHeatingLoad			[W]		'Ideal convective heat load'
		/*! Cooling _load_ is always positive, even though it reduces energy in zones */
		VVR_IdealCoolingLoad,				// Keyword: IdealCoolingLoad			[W]		'Ideal convective cooling load'
		NUM_VVR
	};

	// *** PUBLIC MEMBER FUNCTIONS

	/*! Constructor. */
	IdealHeatingCoolingModel(unsigned int id, const std::string &displayName) :
		m_id(id), m_displayName(displayName)
	{
	}

	/*! Initializes object.
		\param model Model data.
		\param objLists The object list stored in the project file (persistent, remains unmodified so that persistent
			pointers to object list elements can be stored).
	*/
	void setup(const NANDRAD::IdealHeatingCoolingModel & model, const std::vector<NANDRAD::ObjectList> & objLists, const std::vector<NANDRAD::Zone> & zones);

	/*! Returns object list of all referenced models. */
	const NANDRAD::ObjectList &objectList() const;

	// *** Re-implemented from AbstractModel

	/*! Thermal ventilation loads can be requested via MODEL reference. */
	virtual NANDRAD::ModelInputReference::referenceType_t referenceType() const override {
		return NANDRAD::ModelInputReference::MRT_MODEL;
	}

	/*! Return unique class ID name of implemented model. */
	virtual const char * ModelIDName() const override { return "IdealHeatingCoolingModel"; }

	/*! Returns unique ID of this model instance. */
	virtual unsigned int id() const override { return m_id; }

	/*! Resizes m_results vector. */
	virtual void initResults(const std::vector<AbstractModel*> & /* models */) override;

	/*! Populates the vector resDesc with descriptions of all results provided by this model. */
	virtual void resultDescriptions(std::vector<QuantityDescription> & resDesc) const override;

	/*! Retrieves reference pointer to a value with given input reference name. */
	virtual const double * resultValueRef(const InputReference & quantity) const override;

	/*! Computes and returns serialization size in bytes. */
	virtual std::size_t serializationSize() const override;

	/*! Stores control value at memory*/
	virtual void serialize(void* & dataPtr) const override;

	/*! Restores control value from memory.*/
	virtual void deserialize(void* & dataPtr) override;

	// *** Re-implemented from AbstractTimeDependency

	int setTime(double t) override { m_tCurrent = t; return 0; }
	void stepCompleted(double t) override;


	// *** Re-implemented from AbstractStateDependency

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

	/*! Quick access pointer to object list (for scheduled model). */
	const NANDRAD::ObjectList						*m_objectList = nullptr;

	/*! Cached heating power per zone area in [W/m2] */
	double											m_maxHeatingPower = 666;
	/*! Cached cooling power per zone area in [W/m2] */
	double											m_maxCoolingPower = 777;

	/*! Kp-parameter for controller. */
	double											m_Kp = 1;
	/*! Ki-parameter for controller. */
	double											m_Ki = 0;

	/*! Cached pointer to zone parameters, needed to check for valid zones in initReslts(). */
	const std::vector<NANDRAD::Zone>				*m_zones = nullptr;

	/*! Air volumes of the zones in [m3], size matches ids in m_objectList. */
	std::vector<double>								m_zoneAreas;

	/*! Vector valued results, computed/updated during the calculation. */
	std::vector<VectorValuedQuantity>				m_vectorValuedResults;

	/*! Vector with input references.
		For each thermostat model found, this vector contains 2*number of zones input refs, for each zone
		a heating and cooling control values is requested.
	*/
	std::vector<InputReference>						m_inputRefs;
	/*! Holds number of thermostat model objects that values were requested from. */
	unsigned int									m_thermostatModelObjects = 0;

	/*! Vector with value references.
		Vector contains 2 references (optional) for each zone that this model is applied to.
		The order is: HeatingControlValue(zone1), CoolingControlValue(zone1), ... CoolingControlValue(zoneN)
	*/
	std::vector<const double*>						m_valueRefs;

	/*! Integral values for each PI controller (if used). */
	std::vector<double>								m_controllerIntegralValues;

	/*! Time point at end of last step, updated in stepCompleted(). */
	double											m_tEndOfLastStep = -1;
	/*! Current time point, set in setTime(). */
	double											m_tCurrent = -1;

};


} // namespace NANDRAD_MODEL

#endif // NM_IdealHeatingCoolingModelH
