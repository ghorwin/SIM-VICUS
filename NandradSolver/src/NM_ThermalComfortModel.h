#ifndef NM_ThermalComfortModelH
#define NM_ThermalComfortModelH

#include "NM_AbstractModel.h"
#include "NM_AbstractStateDependency.h"
#include "NM_VectorValuedQuantity.h"

#include <NANDRAD_ObjectList.h>

namespace NANDRAD {
	class SimulationParameter;
	class ThermalComfortModel;
	class Zone;
}

namespace NANDRAD_MODEL {

/*! A model to compute operative temperature in zones. This is a zone specific model.
	Model requests surface temperatures from zone-facing constructions, and window surface temperatures from
	all windows facing the zone.
*/
class ThermalComfortModel : public AbstractModel, public AbstractStateDependency {
public:
	/*! Computed results, provided with access via zone ID. */
	enum Results {
		R_OperativeTemperature,				// Keyword: OperativeTemperature				[C]	'Operative temperature'
		NUM_R
	};

	// *** PUBLIC MEMBER FUNCTIONS

	/*! Constructor. */
	ThermalComfortModel(unsigned int id, const std::string &displayName) :
		m_id(id), m_displayName(displayName)
	{
	}

	/*! Initializes object. */
	void setup();

	// *** Re-implemented from AbstractModel

	/*! Thermal ventilation loads can be requested via MODEL reference. */
	virtual NANDRAD::ModelInputReference::referenceType_t referenceType() const override {
		return NANDRAD::ModelInputReference::MRT_ZONE;
	}

	/*! Return unique class ID name of implemented model. */
	virtual const char * ModelIDName() const override { return "ThermalComfortModel"; }

	/*! Returns unique ID of this model instance. */
	virtual unsigned int id() const override { return m_id; }

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

	struct ConstructionInputData {
		unsigned int	m_conID;
		double			m_netArea;
		std::string		m_refName;
		const double *	m_valueRef = nullptr;
	};

	/*! Input references from constructions to this object. */
	std::vector<ConstructionInputData>				m_inputRefs;

	/*! Vector with input references. */
	std::vector<const double*>						m_valueRefs;

	/*! Computed operative temperature in [K] */
	double											m_operativeTemperature = -999;
};

} // namespace NANDRAD_MODEL

#endif // NM_ThermalComfortModelH
