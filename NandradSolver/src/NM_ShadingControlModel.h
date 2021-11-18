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

#ifndef NM_ShadingControlModelH
#define NM_ShadingControlModelH

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
	control. This model depends only on time.

	The implementation of the controller is encapsulated in class DigitalHysteresisController.

	The model provides 'ShadingControlValue' and 'SolarIntensityOnShadingSensor'.
*/
class ShadingControlModel : public AbstractModel, public AbstractTimeDependency {
public:
	ShadingControlModel(unsigned int id, const std::string &displayName):
		m_id(id), m_displayName(displayName)
	{
	}

	/*! Initializes object.
		\param controller Model data.
	*/
	void setup(const NANDRAD::ShadingControlModel &controller, const Loads &loads);


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

	/*! Retrieves reference pointer to a value with given input reference name. */
	virtual const double * resultValueRef(const InputReference & quantity) const override;

	/*! Computes and returns serialization size in bytes. */
	virtual std::size_t serializationSize() const override;

	/*! Stores model content at memory location pointed to by dataPtr. */
	virtual void serialize(void* & dataPtr) const override;

	/*! Restores model content from memory at location pointed to by dataPtr. */
	virtual void deserialize(void* & dataPtr) override;

	// *** Re-implemented from AbstractTimeDependency

	/*! Sets a new controller state. */
	int setTime(double t) override;

	/*! Forwarded to controller. */
	void stepCompleted(double t) override;


private:
	/*! Model instance ID. */
	unsigned int									m_id;
	/*! Display name (for error messages). */
	std::string										m_displayName;
	/*! The shading controller. */
	DigitalHysteresisController						m_controller;
	/*! Cached set point for the shading controller (average between min and max intensity). */
	double											m_targetValue;
	/*! Cached solar radiation intensity in [W/m2] on sensor. */
	double											m_currentIntensity = 666;
	/*! Model parameters. */
	const NANDRAD::ShadingControlModel				*m_shadingControlModel = nullptr;
	/*! Cached pointer to climate loads model, to retrieve climatic loads. */
	const Loads 									*m_loads = nullptr;

};



} // namespace NANDRAD_MODEL

#endif // NM_ShadingControlModelH
