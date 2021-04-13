#ifndef NM_WindowModelH
#define NM_WindowModelH

#include "NM_AbstractModel.h"
#include "NM_AbstractStateDependency.h"
#include "NM_AbstractTimeDependency.h"

namespace NANDRAD {
	class SimulationParameter;
	class EmbeddedObjectWindow;
	class WindowGlazingSystem;
	class ConstructionInstance;
}


namespace NANDRAD_MODEL {

class Loads;

/*! A model for heat transfer (convective and radiation) through windows. */
class WindowModel : public AbstractModel, public AbstractTimeDependency, public AbstractStateDependency {
public:
	/*! Computed results, provided with access via construction instance ID. */
	enum Results {
		R_FluxHeatConductionA,				// Keyword: FluxHeatConductionA			[W]			'Heat conduction flux across interface A (into window).'
		R_FluxHeatConductionB,				// Keyword: FluxHeatConductionB			[W]			'Heat conduction flux across interface B (into window).'
		R_FluxShortWaveRadiationA,			// Keyword: FluxShortWaveRadiationA		[W]			'Short wave radiation flux across interface A (into window).'
		R_FluxShortWaveRadiationB,			// Keyword: FluxShortWaveRadiationB		[W]			'Short wave radiation flux across interface B (into window).'
		NUM_R
	};

	// *** PUBLIC MEMBER FUNCTIONS

	/*! Constructor. */
	WindowModel(unsigned int id, const std::string &displayName) :
		m_id(id), m_displayName(displayName)
	{
	}

	/*! Initializes object.
		\param windowModelPara Model data.
		\param simPara Required simulation parameter.
		\param conInst Reference to construction instance containing this window.
		\param loads Climatic loads object to draw radiation loads from.
	*/
	void setup(const NANDRAD::EmbeddedObjectWindow & windowModelPara, const NANDRAD::SimulationParameter &simPara,
			   const NANDRAD::ConstructionInstance & conInst,
			   Loads & loads);


	// *** Re-implemented from AbstractModel

	/*! Thermal ventilation loads can be requested via MODEL reference. */
	virtual NANDRAD::ModelInputReference::referenceType_t referenceType() const override {
		return NANDRAD::ModelInputReference::MRT_EMBEDDED_OBJECT;
	}

	/*! Return unique class ID name of implemented model. */
	virtual const char * ModelIDName() const override { return "WindowModel"; }

	/*! Returns unique ID of this model instance. */
	virtual unsigned int id() const override { return m_id; }

	/*! Populates the vector resDesc with descriptions of all results provided by this model. */
	virtual void resultDescriptions(std::vector<QuantityDescription> & resDesc) const override;

	/*! Retrieves reference pointer to a value with given input reference name. */
	virtual const double * resultValueRef(const InputReference & quantity) const override;


	// *** Re-implemented from AbstractTimeDependency

	/*! Main state-changing function.
		This function sets the new time point state. Must be implemented in derived models.
		\param t Simulation time in [s] (solver time).
		\return Returns 0 when calculation was successful, 1 when a recoverable error has been detected,
			2 when something is badly wrong
	*/
	virtual int setTime(double t);

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


	// *** Other public member functions

	/*! Returns ID of associated zone at interface A (> 0 if a room zone is connected). */
	unsigned int interfaceAZoneID() const;
	/*! Returns ID of associated zone at interface B (> 0 if a room zone is connected). */
	unsigned int interfaceBZoneID() const;

private:

	/*! Enumeration types for ordered input references, some may be unused and remain nullptr. */
	enum InputReferences {
		InputRef_SideATemperature,
		InputRef_SideBTemperature,
		InputRef_ShadingFactor,
		NUM_InputRef
	};

	/*! Constant pointer to the referenced parameter block. */
	const NANDRAD::EmbeddedObjectWindow				*m_windowModel = nullptr;
	/*! Reference to simulation parameter block. */
	const NANDRAD::SimulationParameter				*m_simPara = nullptr;
	/*! Constant pointer to the referenced parameter block. */
	const NANDRAD::ConstructionInstance				*m_con = nullptr;
	/*! Cached pointer to climate loads model, to retrieve climatic loads. */
	const Loads *									m_loads = nullptr;

	/*! Flag is set true, left side (A) has a parametrized Interface and connection to zone=0 (ambient). */
	bool											m_haveSolarLoadsOnA;
	/*! Flag is set true, right side (B) has a parametrized Interface and connection to zone=0 (ambient). */
	bool											m_haveSolarLoadsOnB;

	/*! Shading factor: either constant, linear spline value or controlled. */
	double											m_shadingFactor = 1.0;

	/*! Model instance ID (unused since results are provided for zones). */
	unsigned int									m_id;
	/*! Display name (for error messages). */
	std::string										m_displayName;

	/*! Results, computed/updated during the calculation. */
	std::vector<double>								m_results;

	/*! Vector with input references. */
	std::vector<const double*>						m_valueRefs;
};

} // namespace NANDRAD_MODEL

#endif // NM_WindowModelH
