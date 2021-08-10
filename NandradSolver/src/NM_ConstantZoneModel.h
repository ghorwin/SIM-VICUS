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

#ifndef NM_ConstantZoneModelH
#define NM_ConstantZoneModelH

#include "NM_AbstractModel.h"
#include "NM_AbstractStateDependency.h"

namespace NANDRAD {
	class Zone;
};


namespace NANDRAD_MODEL {

/*! A model for a Zone with predefined temperature. Temperature must be given as parameter 'Temperature', but is
	overwritten if a schedule 'TemperatureSchedule' is provided.

	Generates a single result value 'AirTemperature', that is used just as the computed air temperature of
	an active zone. Note that for "ground" zones the term 'AirTemperature' may be a bit misleading, but
	using the same result variable name as for active zones makes it much easier for ConstructionInstance models.
*/
class ConstantZoneModel : public AbstractModel, public AbstractStateDependency {
public:
	ConstantZoneModel(unsigned int id, const std::string &displayName):
		m_id(id), m_displayName(displayName)
	{
	}

	/*! Initializes object. */
	void setup(const NANDRAD::Zone &zone);

	// *** Re-implemented from AbstractModel

	/*! Thermal ventilation loads can be requested via MODEL reference. */
	virtual NANDRAD::ModelInputReference::referenceType_t referenceType() const override {
		return NANDRAD::ModelInputReference::MRT_ZONE;
	}

	/*! Return unique class ID name of implemented model. */
	virtual const char * ModelIDName() const override { return "ConstantZoneModel"; }

	/*! Returns unique ID of this model instance. */
	virtual unsigned int id() const override { return m_id; }

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
	virtual void stateDependencies(std::vector< std::pair<const double *, const double *> > & /*resultInputValueReferences*/) const override { }

	/*! Does nothing. */
	int update() override {return 0;}

private:
	/*! Model instance ID. */
	unsigned int									m_id;
	/*! Display name (for error messages). */
	std::string										m_displayName;
	/*! Coded zone type (either 'Constant' or 'Scheduled'). */
	int												m_zoneType = 999;
	/*! Reference to predefined zone temperature [K]. */
	const double*									m_temperature = nullptr;
};



} // namespace NANDRAD_MODEL

#endif // NM_ConstantZoneModelH
