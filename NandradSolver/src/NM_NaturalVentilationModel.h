#ifndef NM_NaturalVentilationModelH
#define NM_NaturalVentilationModelH

#include "NM_AbstractModel.h"
#include "NM_AbstractStateDependency.h"
#include "NM_VectorValuedQuantity.h"

namespace NANDRAD {
	class SimulationParameter;
	class NaturalVentilationModel;
}

namespace NANDRAD_MODEL {

/*! A model for natural ventilation rate.
	It implements either constant or schedules ventilation rates and compute thermal ventilation loads
	for all zones referenced in the object list.

*/
class NaturalVentilationModel : public AbstractModel, AbstractStateDependency {
public:
	/*! Computed results, provided with access via zone ID. */
	enum VectorValuedResults {
		VVR_VentilationRate,				// Keyword: VentilationRate					[1/h]	'Ventilation rate'
		VVR_VentilationHeatFlux,			// Keyword: VentilationHeatFlux				[W]		'Ventilation heat flux'
		NUM_VVR
	};

	// *** PUBLIC MEMBER FUNCTIONS

	/*! Constructor. */
	NaturalVentilationModel(unsigned int id, const std::string &displayName) :
		m_id(id), m_displayName(displayName)
	{
	}

	/*! Initializes object.
		\param ventilationModel Ventilation model data.
		\param simPara Required simulation parameter.
	*/
	void setup(const NANDRAD::NaturalVentilationModel & ventilationModel, const NANDRAD::SimulationParameter &simPara);


	// *** Re-implemented from AbstractModel

	/*! Thermal ventilation loads can be requested via zone reference. */
	virtual NANDRAD::ModelInputReference::referenceType_t referenceType() const override {
		return NANDRAD::ModelInputReference::MRT_ZONE;
	}

	/*! Return unique class ID name of implemented model. */
	virtual const char * ModelIDName() const override { return "NaturalVentilationModel"; }

	/*! Populates the vector resDesc with descriptions of all results provided by this model. */
	virtual void resultDescriptions(std::vector<QuantityDescription> & resDesc) const override;

	/*! Retrieves reference pointer to a value with given input reference name. */
	virtual const double * resultValueRef(const QuantityName & quantityName) const override;

	/*! Resizes m_results vector. */
	virtual void initResults(const std::vector<AbstractModel*> & /* models */) override;


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

	/*! Returns dependencies between result variables and input variables. */
	virtual void stateDependencies(std::vector< std::pair<const double *, const double *> > & resultInputValueReferences) const override;

	/*! Provides the object with references to requested input variables (persistent memory location). */
	virtual void setInputValueRefs(const std::vector<QuantityDescription> & resultDescriptions, const std::vector<const double *> & resultValueRefs) override;

	/*! Sums up all provided input quantities and computes divergence of balance equations. */
	int update() override;


private:
	/*! Constant pointer to the referenced zone parameter block. */
	const NANDRAD::NaturalVentilationModel			*m_zone = nullptr;
	/*! Reference to simulation parameter block. */
	const NANDRAD::SimulationParameter				*m_simPara = nullptr;

	/*! Model instance ID (unused since results are provided for zones). */
	unsigned int									m_id;
	/*! Display name (for error messages). */
	std::string										m_displayName;
	/*! True if moisture balance is enabled. */
	bool											m_moistureBalanceEnabled;

	/*! Vector valued results, computed/updated during the calculation. */
	std::vector<VectorValuedQuantity>	m_vectorValuedResults;
};

} // namespace NANDRAD_MODEL

#endif // NM_NaturalVentilationModelH
