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

#ifndef NM_HeatLoadSummationModelH
#define NM_HeatLoadSummationModelH

#include "NM_AbstractModel.h"
#include "NM_AbstractStateDependency.h"
#include "NM_VectorValuedQuantity.h"

#include <NANDRAD_ObjectList.h>

namespace NANDRAD {
	class HeatLoadSummationModel;
}

namespace NANDRAD_MODEL {

/*! A model to sum
	The model instance is identified by reference type MODEL and the id of the NANDRAD model parametrization block.

	For each zone in the object list, the model requests HeatingControlValue and CoolingControlValue quantities, that
	are provided by thermostat models. Also, it retrieves the zone's net floor area and computed together with
	parameters from NANDRAD::HeatLoadSummationModel the maximum heating/cooling power. The actual heating/cooling power
	is obtained by multiplication of the heating/cooling control value with this maximum power.
*/
class HeatLoadSummationModel : public AbstractModel, public AbstractStateDependency {
public:
	/*! Computed results. */
	enum Results {
		R_TotalHeatLoad,			// Keyword: TotalHeatLoad			[W]		'Sum of heat load'
		NUM_R
	};

	// *** PUBLIC MEMBER FUNCTIONS

	/*! Constructor. */
	HeatLoadSummationModel(unsigned int id, const std::string &displayName) :
		m_id(id), m_displayName(displayName)
	{
	}

	/*! Initializes object.
		\param model Model data.
		\param objLists The object list stored in the project file (persistent, remains unmodified so that persistent
			pointers to object list elements can be stored).
	*/
	void setup(const NANDRAD::HeatLoadSummationModel & model, const std::vector<NANDRAD::ObjectList> & objLists);

	/*! Returns object list of all referenced models. */
	const NANDRAD::ObjectList &objectList() const;

	// *** Re-implemented from AbstractModel

	/*! Thermal ventilation loads can be requested via MODEL reference. */
	virtual NANDRAD::ModelInputReference::referenceType_t referenceType() const override {
		return NANDRAD::ModelInputReference::MRT_MODEL;
	}

	/*! Return unique class ID name of implemented model. */
	virtual const char * ModelIDName() const override { return "HeatLoadSummationModel"; }

	/*! Returns unique ID of this model instance. */
	virtual unsigned int id() const override { return m_id; }

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

	/*! Quick access pointer to object list (for scheduled model). */
	const NANDRAD::ObjectList						*m_objectList = nullptr;

	/*! Vector of results, computed/updated during the calculation. */
	std::vector<double>								m_results;

	/*! Vector with input references.
		For each thermostat model found, this vector contains 2*number of zones input refs, for each zone
		a heating and cooling control values is requested.
	*/
	std::vector<InputReference>						m_inputRefs;

	/*! Vector with value references. */
	std::vector<const double*>						m_valueRefs;
};

} // namespace NANDRAD_MODEL

#endif // NM_HeatLoadSummationModelH
