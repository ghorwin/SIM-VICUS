#ifndef NM_SHADINGCONTROLMODEL_H
#define NM_SHADINGCONTROLMODEL_H

#include "NM_Controller.h"
#include "NM_AbstractModel.h"
#include "NM_AbstractStateDependency.h"


namespace NANDRAD {
	class ShadingControlModel;
	class Sensor;
};


namespace NANDRAD_MODEL {

/*! A model for sensor based shading control.
	It can be used as controller instance for different windows and implements a digital hysteresis
	control.
*/
class ShadingControlModel : public DigitalHysteresisController, public AbstractModel, public AbstractStateDependency
{
public:

	ShadingControlModel(unsigned int id, const std::string &displayName):
		m_id(id), m_displayName(displayName)
	{
	}

	/*! Initializes object.
		\param controller Model data.
	*/
	void setup(const NANDRAD::ShadingControlModel &controller);

	/*! D'tor, definition is in NM_ShadingControlModel.cpp. */
	virtual ~ShadingControlModel() override { }

	// *** Re-implemented from AbstractModel

	/*! Thermal ventilation loads can be requested via MODEL reference. */
	virtual NANDRAD::ModelInputReference::referenceType_t referenceType() const override {
		return NANDRAD::ModelInputReference::MRT_MODEL;
	}

	/*! Return unique class ID name of implemented model. */
	virtual const char * ModelIDName() const override { return "ShadingControlModel"; }

	/*! Returns unique ID of this model instance. */
	virtual unsigned int id() const override { return m_id; }

	/*! Populates the vector resDesc with descriptions of all results provided by this model. */
	virtual void resultDescriptions(std::vector<QuantityDescription> & resDesc) const override;

	/*! Returns vector of all scalar and vector valued results pointer.*/
	virtual void resultValueRefs(std::vector<const double *> &res) const override;

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
	/*! Model instance ID. */
	unsigned int									m_id;
	/*! Display name (for error messages). */
	std::string										m_displayName;
	/*! Base model definition. */
	const NANDRAD::ShadingControlModel				*m_controller = nullptr;
	/*! Constant pointer to solar radiation pointer. */
	const double									*m_SWRadiationRef = nullptr;
};


} // namespace NANDRAD_MODEL

#endif // NM_SHADINGCONTROLMODEL_H
