/*	NANDRAD Solver Framework and Model Implementation.

	Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Anne Paepcke     <anne.paepcke -[at]- tu-dresden.de>

	This library is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 3 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.
*/

#ifndef NM_RoomRadiationLoadsModelH
#define NM_RoomRadiationLoadsModelH

#include "NM_AbstractModel.h"
#include "NM_AbstractStateDependency.h"

namespace NANDRAD {
	class SimulationParameter;
}


namespace NANDRAD_MODEL {

/*! A model for summing up radiation loads through all windows of a zone. */
class RoomRadiationLoadsModel : public AbstractModel, public AbstractStateDependency {
public:
	/*! Computed results, provided with access via construction instance ID. */
	enum Results {
		R_WindowSolarRadiationFluxSum,			// Keyword: WindowSolarRadiationFluxSum		[W]			'Sum of all short wave radiation fluxes across all windows of a zone (positive into zone).'
		NUM_R
	};

	// *** PUBLIC MEMBER FUNCTIONS

	/*! Constructor. */
	RoomRadiationLoadsModel(unsigned int id, const std::string &displayName) :
		m_id(id), m_displayName(displayName)
	{
	}

	// *** Re-implemented from AbstractModel

	/*! Thermal ventilation loads can be requested via MODEL reference. */
	virtual NANDRAD::ModelInputReference::referenceType_t referenceType() const override {
		return NANDRAD::ModelInputReference::MRT_ZONE;
	}

	/*! Return unique class ID name of implemented model. */
	virtual const char * ModelIDName() const override { return "RoomRadiationLoadsModel"; }

	/*! Returns unique ID of this model instance. */
	virtual unsigned int id() const override { return m_id; }

	/*! Populates the vector resDesc with descriptions of all results provided by this model. */
	virtual void resultDescriptions(std::vector<QuantityDescription> & resDesc) const override;

	/*! Retrieves reference pointer to a value with given input reference name. */
	virtual const double * resultValueRef(const QuantityName & quantityName) const override;


	// *** Re-implemented from AbstractStateDependency

	/*! Composes all input references (by scanning through provided models and picking out results that we
		are interested in).
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

	/*! Sums up all provided input quantities. */
	int update() override;

private:

	/*! Reference to simulation parameter block. */
	const NANDRAD::SimulationParameter				*m_simPara = nullptr;

	/*! Model instance ID (unused since results are provided for zones). */
	unsigned int									m_id;
	/*! Display name (for error messages). */
	std::string										m_displayName;

	/*! Input references of this summation model. */
	std::vector<InputReference>						m_inputRefs;

	/*! Value references for window solar radiation fluxes in [W] (positive if out-of-room). */
	std::vector<const double *>						m_windowSolarRadiationLoadsRefs;

	/*! Results, computed/updated during the calculation. */
	double											m_result = 202020;

	/*! Vector with input references. */
	std::vector<const double*>						m_valueRefs;

};

} // namespace NANDRAD_MODEL

#endif // NM_RoomRadiationLoadsModelH
