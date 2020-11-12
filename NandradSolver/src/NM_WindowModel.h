#ifndef NM_WindowModelH
#define NM_WindowModelH

#include "NM_AbstractModel.h"
#include "NM_AbstractStateDependency.h"

namespace NANDRAD {
	class SimulationParameter;
	class EmbeddedObjectWindow;
	class WindowGlazingSystem;
	class ConstructionInstance;
}


namespace NANDRAD_MODEL {

/*! A model for heat transfer (convective and radiation) through windows. */
class WindowModel : public AbstractModel, public AbstractStateDependency {
public:
	/*! Computed results, provided with access via construction instance ID. */
	enum Results {
		R_HeatConductionFlux,				// Keyword: HeatConductionFlux					[W]		'Heat conduction flux'
		R_HeatConductionFluxDensity,		// Keyword: HeatConductionFluxDensity			[W/m2]	'Heat conduction flux density'
		R_ShortWaveRadiationFlux,			// Keyword: ShortWaveRadiationFlux				[W]		'Solar radiation flux (global shortwave radiation)'
		R_ShortWaveRadiationFluxDensity,	// Keyword: ShortWaveRadiationFluxDensity		[W/m2]	'Solar radiation flux density (global shortwave radiation)'
		NUM_R
	};

	// *** PUBLIC MEMBER FUNCTIONS

	/*! Constructor. */
	WindowModel(unsigned int id, const std::string &displayName) :
		m_id(id), m_displayName(displayName)
	{
	}

	/*! Initializes object.
		\param ventilationModel Ventilation model data.
		\param simPara Required simulation parameter.
		\param objLists The object list stored in the project file (persistent, remains unmodified so that persistent
			pointers to object list elements can be stored).
	*/
	void setup(const NANDRAD::EmbeddedObjectWindow & windowModelPara, const NANDRAD::SimulationParameter &simPara,
			   const NANDRAD::ConstructionInstance & conInst);


	// *** Re-implemented from AbstractModel

	/*! Thermal ventilation loads can be requested via MODEL reference. */
	virtual NANDRAD::ModelInputReference::referenceType_t referenceType() const override {
		return NANDRAD::ModelInputReference::MRT_EMBEDDED_OBJECT;
	}

	/*! Return unique class ID name of implemented model. */
	virtual const char * ModelIDName() const override { return "WindowModel"; }

	/*! Returns unique ID of this model instance. */
	virtual unsigned int id() const override { return m_id; }

	/*! Resizes m_results vector. */
	virtual void initResults(const std::vector<AbstractModel*> & /* models */) override;

	/*! Populates the vector resDesc with descriptions of all results provided by this model. */
	virtual void resultDescriptions(std::vector<QuantityDescription> & resDesc) const override;

	/*! Retrieves reference pointer to a value with given input reference name. */
	virtual const double * resultValueRef(const QuantityName & quantityName) const override;


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
	/*! Constant pointer to the referenced parameter block. */
	const NANDRAD::EmbeddedObjectWindow				*m_windowModel = nullptr;
	/*! Reference to simulation parameter block. */
	const NANDRAD::SimulationParameter				*m_simPara = nullptr;
	/*! Constant pointer to the referenced parameter block. */
	const NANDRAD::ConstructionInstance				*m_conInst = nullptr;

	/*! Model instance ID (unused since results are provided for zones). */
	unsigned int									m_id;
	/*! Display name (for error messages). */
	std::string										m_displayName;

	/*! Vector with input references. */
	std::vector<const double*>						m_valueRefs;
};

} // namespace NANDRAD_MODEL

#endif // NM_WindowModelH
