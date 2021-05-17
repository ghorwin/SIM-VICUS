#ifndef NM_IdealPipeRegisterModelH
#define NM_IdealPipeRegisterModelH

#include "NM_AbstractModel.h"
#include "NM_AbstractStateDependency.h"
#include "NM_VectorValuedQuantity.h"

#include <NANDRAD_ObjectList.h>

#include <IBK_LinearSpline.h>

namespace NANDRAD {
	class IdealPipeRegisterModel;
	class Thermostat;
	class Zone;
}

namespace NANDRAD_MODEL {

/*! A model for ideal heating/cooling (air heating).
	The model instance is identified by reference type MODEL and the id of the NANDRAD model parametrization block.
	It implements ideal heating/cooling loads for all zones referenced in the object list.
*/
class IdealPipeRegisterModel : public AbstractModel, public AbstractStateDependency {
public:
	/*! Computed results. */
	enum Results {
		R_MassFlux,						// Keyword: MassFlux				[kg/s]	'ControlledS mass flow'
		NUM_R
	};

	/*! Computed results, vector-valued results that provide access via zone ID. */
	enum VectorValuedResults {
		VVR_SurfaceHeatingLoad,				// Keyword: SurfaceHeatingLoad			[W]		'Surface heat load'
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

	/*! Returns object list of all referenced models. */
	const NANDRAD::ObjectList &objectList() const;

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

	/*! Model type (decides how to retrieve supply temperature). */
	int												m_modelType;

	/*! Quick access pointer to object list (for targetted construction instances). */
	const NANDRAD::ObjectList						*m_objectList = nullptr;

	/*! Cached heating power per zone area in [W/m2] */
	double											m_maxMassFlow = 666;

	/*! hydraulic (inner) diameter of pipe in [m] */
	double											m_innerDiameter = -999;

	/*! pipe length in [m] */
	double											m_length = -999;

	/*! Fluid heat capacity [J/kgK].
		Cached value from fluid properties.
	*/
	double											m_fluidHeatCapacity = -999;

	/*! Fluid density [kg/m3].
		Cached value from fluid properties.
	*/
	double											m_fluidDensity = -999;

	/*! Fluid volume [m3]. */
	double											m_fluidVolume = -999;

	/*! Effective flow cross-section [m2].
		\note This is the total cross section for fluid flow of all pipes (if m_nParallelPipes is larger than 1).
	*/
	double											m_fluidCrossSection = -999;

	/*! Fluid conductivity [W/mK].
		Cached value from fluid properties.
	*/
	double											m_fluidConductivity = 0.01;

	/*! Fluid dynamic viscosity [m/s] (temperature dependend).*/
	IBK::LinearSpline								m_fluidViscosity;

	/*! Equivalent u-value of the pipe wall and insulation per length of pipe in [W/mK] */
	double											m_UValuePipeWall;

	/*! Number of parallel pipes (=1 per default).*/
	unsigned int									m_nParallelPipes;
	/*! Cached thermostat zone id. */
	unsigned int									m_thermostatZoneId = NANDRAD::INVALID_ID;

	/*! Holds number of thermostat model objects that values were requested from. */
	unsigned int									m_thermostatModelObjects = 0;

	/*! Reference to thermotat control value. */
	const double*									m_thermostatValueRef = nullptr;

	/*! Reference to supply temperature. */
	const double*									m_supplyTemperatureRef = nullptr;

	/*! Reference to active layer temperatures. */
	std::vector<const double*>						m_activeLayerTemperatureRefs;

	/*! Results, computed/updated during the calculation. */
	std::vector<double>								m_results;

	/*! Vector valued results, computed/updated during the calculation. */
	std::vector<VectorValuedQuantity>				m_vectorValuedResults;

	/*! Vector with input references.
		For each thermostat model found, this vector contains 2*number of zones input refs, for each zone
		a heating and cooling control values is requested.
	*/
	std::vector<InputReference>						m_inputRefs;

};

} // namespace NANDRAD_MODEL

#endif // NM_IdealPipeRegisterModelH
