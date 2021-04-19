#ifndef NM_SHADINGCONTROLMODEL_H
#define NM_SHADINGCONTROLMODEL_H

#include "NM_Controller.h"
#include "NM_AbstractModel.h"
#include "NM_AbstractTimeDependency.h"


namespace NANDRAD {
	class ShadingControlModel;
	class Sensor;
};


namespace NANDRAD_MODEL {

class Loads;

/*! A model for sensor based shading control.
	It can be used as controller instance for different windows and implements a digital hysteresis
	control.
*/
class ShadingControlModel : public DigitalHysteresisController, public AbstractModel
{
public:

	ShadingControlModel(unsigned int id, const std::string &displayName):
		m_id(id), m_displayName(displayName)
	{
	}

	/*! Initializes object.
		\param controller Model data.
	*/
	void setup(const NANDRAD::ShadingControlModel &controller,
			   const Loads &loads);

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

	// *** Re-implemented from AbstractTimeDependency

	/*! Sets a new controller state. */
	int setTime(double t) override;

private:
	/*! Model instance ID. */
	unsigned int									m_id;
	/*! Display name (for error messages). */
	std::string										m_displayName;
	/*! Base model definition. */
	const NANDRAD::ShadingControlModel				*m_controller = nullptr;
	/*! Cached pointer to climate loads model, to retrieve climatic loads. */
	const Loads 									*m_loads = nullptr;
};


} // namespace NANDRAD_MODEL

#endif // NM_SHADINGCONTROLMODEL_H
