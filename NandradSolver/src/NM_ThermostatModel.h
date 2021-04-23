#ifndef NM_ThermostatModelH
#define NM_ThermostatModelH

#include "NM_AbstractModel.h"
#include "NM_AbstractStateDependency.h"
#include "NM_VectorValuedQuantity.h"

namespace NANDRAD {
	class Zone;
	class Thermostat;
	class ObjectList;
}

namespace NANDRAD_MODEL {

class AbstractController;

/*! A model for a thermostat.
	The thermostat generates a HeatingControlValue and CoolingControlValue for each zone in the referenced
	object list.
*/
class ThermostatModel : public AbstractModel, public AbstractStateDependency {
public:
	/*! Computed results, provided with access via zone ID. */
	enum VectorValuedResults {
		/*! Control signal for heating, 0 - off, 1 - on. */
		VVR_HeatingControlValue,		// Keyword: HeatingControlValue			[---]	'Heating control signal'
		/*! Control signal for cooling, 0 - off, 1 - on. */
		VVR_CoolingControlValue,		// Keyword: CoolingControlValue			[---]	'Cooling control signal'
		NUM_VVR
	};

	// *** PUBLIC MEMBER FUNCTIONS

	/*! Constructor. */
	ThermostatModel(unsigned int id, const std::string &displayName) :
		m_id(id), m_displayName(displayName)
	{
	}

	/*! Cleans up allocated memory (controller). */
	~ThermostatModel() override;

	/*! Initializes object.
		\param ventilationModel Ventilation model data.
		\param simPara Required simulation parameter.
		\param objLists The object list stored in the project file (persistent, remains unmodified so that persistent
			pointers to object list elements can be stored).
	*/
	void setup(const NANDRAD::Thermostat & thermostat,
			   const std::vector<NANDRAD::ObjectList> & objLists, const std::vector<NANDRAD::Zone> & zones);


	// *** Re-implemented from AbstractModel

	/*! Thermal ventilation loads can be requested via MODEL reference. */
	virtual NANDRAD::ModelInputReference::referenceType_t referenceType() const override {
		return NANDRAD::ModelInputReference::MRT_MODEL;
	}

	/*! Return unique class ID name of implemented model. */
	virtual const char * ModelIDName() const override { return "ThermostatModel"; }

	/*! Returns unique ID of this model instance. */
	virtual unsigned int id() const override { return m_id; }

	/*! Resizes m_results vector. */
	virtual void initResults(const std::vector<AbstractModel*> & /* models */) override;

	/*! Populates the vector resDesc with descriptions of all results provided by this model. */
	virtual void resultDescriptions(std::vector<QuantityDescription> & resDesc) const override;

	/*! Retrieves reference pointer to a value with given input reference name. */
	virtual const double * resultValueRef(const InputReference & quantity) const override;


	// *** Re-implemented from AbstractStateDependency

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
	/*! Constant pointer to the referenced zone parameter block. */
	const NANDRAD::Thermostat						*m_thermostat = nullptr;

	/*! Model instance ID (unused since results are provided for zones). */
	unsigned int									m_id;
	/*! Display name (for error messages). */
	std::string										m_displayName;

	/*! Quick access pointer to object list (for scheduled model). */
	const NANDRAD::ObjectList						*m_objectList = nullptr;

	/*! Cached pointer to zone parameters, to access zone volumes during init. */
	const std::vector<NANDRAD::Zone>				*m_zones = nullptr;

	/*! The actual controller instances.
		If we have a reference zone, we only have two controllers (one for heating, one for cooling).
		Otherwise we have two controllers per zone.
		Note: for PControllers without state this is a bit of an overhead, but to stay generic,
			  we must provide a state-containing controller instance in case someone defines a hysteresis or PI controller.
	*/
	std::vector<NANDRAD_MODEL::AbstractController*>	m_controllers;

	/*! Vector valued results, computed/updated during the calculation. */
	std::vector<VectorValuedQuantity>				m_vectorValuedResults;

	/*! Vector with input references.
		4 different options:
		- modelType = Constant, no reference zone:     AirTemperatures for each zone
		- modelType = Constant, reference zone given:  AirTemperature for reference zone only
		- modelType = Scheduled, no reference zone:    {AirTemperatures, HeatingSetpointSchedule, CoolingSetpointSchedule} for each zone
		- modelType = Scheduled, reference zone given: {AirTemperatures, HeatingSetpointSchedule, CoolingSetpointSchedule} for reference zone only
	*/
	std::vector<const double*>						m_valueRefs;
};

} // namespace NANDRAD_MODEL

#endif // NM_ThermostatModelH
