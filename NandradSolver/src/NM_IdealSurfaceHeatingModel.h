#ifndef NM_IdealSurfaceHeatingModelH
#define NM_IdealSurfaceHeatingModelH

#include "NM_AbstractModel.h"
#include "NM_AbstractStateDependency.h"
#include "NM_VectorValuedQuantity.h"

#include <NANDRAD_ObjectList.h>

namespace NANDRAD {
	class IdealSurfaceHeatingModel;
	class Thermostat;
	class Zone;
}

namespace NANDRAD_MODEL {

/*! A model for ideal heating/cooling (air heating).
	The model instance is identified by reference type MODEL and the id of the NANDRAD model parametrization block.
	It implements ideal heating/cooling loads for all zones referenced in the object list.
*/
class IdealSurfaceHeatingModel : public AbstractModel, public AbstractStateDependency {
public:
	/*! Computed results. */
	enum Results {
		R_IdealSurfaceHeatingLoad,				// Keyword: IdealSurfaceHeatingLoad			[W]		'Ideal, surface heat load'
		NUM_R
	};

	// *** PUBLIC MEMBER FUNCTIONS

	/*! Constructor. */
	IdealSurfaceHeatingModel(unsigned int id, const std::string &displayName) :
		m_id(id), m_displayName(displayName)
	{
	}

	/*! Initializes object.
		\param model Model data.
		\param objLists The object list stored in the project file (persistent, remains unmodified so that persistent
			pointers to object list elements can be stored).
	*/
	void setup(const NANDRAD::IdealSurfaceHeatingModel & model,
			   const std::vector<NANDRAD::ObjectList> & objLists,
			   const std::vector<NANDRAD::Zone> & zones);

	/*! Returns object list of all referenced models. */
	const NANDRAD::ObjectList &objectList() const;

	// *** Re-implemented from AbstractModel

	/*! Thermal ventilation loads can be requested via MODEL reference. */
	virtual NANDRAD::ModelInputReference::referenceType_t referenceType() const override {
		return NANDRAD::ModelInputReference::MRT_MODEL;
	}

	/*! Return unique class ID name of implemented model. */
	virtual const char * ModelIDName() const override { return "IdealSurfaceHeatingModel"; }

	/*! Returns unique ID of this model instance. */
	virtual unsigned int id() const override { return m_id; }

	/*! Resizes m_results vector. */
	virtual void initResults(const std::vector<AbstractModel*> & /* models */) override;

	/*! Populates the vector resDesc with descriptions of all results provided by this model. */
	virtual void resultDescriptions(std::vector<QuantityDescription> & resDesc) const override;

	/*! Retrieves reference pointer to a value with given input reference name. */
	virtual const double * resultValueRef(const InputReference & quantity) const override;


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

	/*! Quick access pointer to object list (for targetted construction instances). */
	const NANDRAD::ObjectList						*m_objectList = nullptr;

	/*! Cached heating power per zone area in [W/m2] */
	double											m_maxHeatingPower = 666;

	/*! Cached thermostat zone area in [m2] */
	double											m_thermostatZoneArea = 666;

	/*! Cached thermostat zone id. */
	unsigned int									m_thermostatZoneId = NANDRAD::INVALID_ID;

	/*! Holds number of thermostat model objects that values were requested from. */
	unsigned int									m_thermostatModelObjects = 0;

	/*! Reference to thermotat control value. */
	const double*									m_thermostatValueRef = nullptr;

	/*! Vector valued results, computed/updated during the calculation. */
	std::vector<double>								m_results;

	/*! Vector with input references.
		For each thermostat model found, this vector contains 2*number of zones input refs, for each zone
		a heating and cooling control values is requested.
	*/
	std::vector<InputReference>						m_inputRefs;

};

} // namespace NANDRAD_MODEL

#endif // NM_IdealSurfaceHeatingModelH
