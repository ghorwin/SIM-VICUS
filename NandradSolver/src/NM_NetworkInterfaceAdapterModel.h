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

#ifndef NM_NetworkInterfaceAdapterModelH
#define NM_NetworkInterfaceAdapterModelH

#include "NM_AbstractModel.h"
#include "NM_AbstractStateDependency.h"

namespace NANDRAD {
	class NetworkInterfaceAdapterModel;
}

namespace NANDRAD_MODEL {

/*! A model for network interface (return temperature) calculation.
	This model takes the summation heat load from the referenced heat load summation model and scheduled mass
	flux and supply temperatures. Input refs needed for the model:

		 'Model(id=801).TotalHeatLoad'
		 'Model(id=1212).SupplyTemperatureSchedule'
		 'Model(id=1212).MassFluxSchedule'

	Provides return temperature of medium: 'Model(id=1234).ReturnTemperature'
*/
class NetworkInterfaceAdapterModel : public AbstractModel, public AbstractStateDependency {
public:

	/*! Results, published by the model. */
	enum Results {
		R_ReturnTemperature,					// Keyword: ReturnTemperature			[C]		'Return temperature.'
		NUM_R
	};

	// *** PUBLIC MEMBER FUNCTIONS

	/*! Constructor. */
	NetworkInterfaceAdapterModel(unsigned int id, const std::string &displayName) :
		m_id(id), m_displayName(displayName)
	{
	}

	/*! Initializes object.
		\param modelData Model data.
	*/
	void setup(const NANDRAD::NetworkInterfaceAdapterModel & modelData);

	// *** Re-implemented from AbstractModel

	/*! MODEL reference type. */
	virtual NANDRAD::ModelInputReference::referenceType_t referenceType() const override {
		return NANDRAD::ModelInputReference::MRT_MODEL;
	}

	/*! Return unique class ID name of implemented model. */
	virtual const char * ModelIDName() const override { return "NetworkInterfaceAdapterModel"; }

	/*! Returns unique ID of this model instance. */
	virtual unsigned int id() const override { return m_id; }

	/*! Populates the vector resDesc with descriptions of all results provided by this model. */
	virtual void resultDescriptions(std::vector<QuantityDescription> & resDesc) const override;

	/*! Retrieves reference pointer to a value with given input reference name. */
	virtual const double * resultValueRef(const InputReference & quantity) const override;


	// *** Re-implemented from AbstractStateDependency

	/*! Returns model evaluation priority. */
	int priorityOfModelEvaluation() const override;

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
	/*! Constant pointer to the referenced zone parameter block. */
	const NANDRAD::NetworkInterfaceAdapterModel		*m_modelData = nullptr;

	/*! Model instance ID. */
	unsigned int									m_id;
	/*! Display name (for error messages). */
	std::string										m_displayName;

	/*! Data cache for calculated results (updated in call to update()).
		Index matches enum values of Results.
	*/
	std::vector<double>								m_results;

	/*! Vector with input references. */
	std::vector<const double*>						m_valueRefs;
};

} // namespace NANDRAD_MODEL

#endif // NM_NetworkInterfaceAdapterModelH
