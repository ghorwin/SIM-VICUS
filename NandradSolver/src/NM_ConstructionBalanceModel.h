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


#ifndef ConstructionBalanceModelH
#define ConstructionBalanceModelH

#include "NM_AbstractModel.h"
#include "NM_AbstractStateDependency.h"

namespace NANDRAD {
	class ConstructionInstance;
	class SimulationParameter;
}

namespace NANDRAD_MODEL {

class ConstructionStatesModel;

/*!	This model encapsulates the whole computation functionality for a
	1D construction solver.
*/
class ConstructionBalanceModel : public AbstractModel, public AbstractStateDependency {
public:

	enum WallMoistureBalanceCalculationMode {
		CM_Average,							// Keyword: Average								'Vvapor transport calculation through average wall layers including boundaries.'
		CM_Detailed,						// Keyword: Detailed							'Detailed vapor storage and transport calculation including spacial discretization.'
		CM_None								// Keyword: None								'No wall moisture calculation.'
	};
	enum VectorValuedResults {
		VVR_ThermalLoad,					// Keyword: ThermalLoad				[W]			'Optional field fluxes for all material layers with given layer index.'
		NUM_VVR
	};
	enum InputReferences {
		InputRef_FieldFlux,					// Keyword: FieldFlux				[W]			'Optional field flux for a given material layer.'
		NUM_InputRef
	};


	/*! Constructor */
	ConstructionBalanceModel(unsigned int id, const std::string &displayName) :
		m_id(id), m_displayName(displayName)
	{
	}

	/*! Initializes model. */
	void setup(const NANDRAD::ConstructionInstance & con, const NANDRAD::SimulationParameter & simPara, ConstructionStatesModel * statesModel);


	// *** Re-implemented from AbstractModel

	/*! Balance model can be referenced as ConstructionInstance and ID. */
	virtual NANDRAD::ModelInputReference::referenceType_t referenceType() const override {
		return NANDRAD::ModelInputReference::MRT_CONSTRUCTIONINSTANCE;
	}

	/*! Return unique class ID name of implemented model. */
	virtual const char * ModelIDName() const override { return "ConstructionBalanceModel";}

	/*! Returns unique ID of this model instance. */
	virtual unsigned int id() const override { return m_id; }

	/*! Returns display name of this model instance. */
	virtual const char * displayName() const override { return m_displayName.c_str(); }

	/*! Populates the vector resDesc with descriptions of all results provided by this model. */
	virtual void resultDescriptions(std::vector<QuantityDescription> & resDesc) const override;

	/*! Returns vector of all scalar and vector valued results pointer. */
	virtual void resultValueRefs(std::vector<const double *> &res) const override;

	/*! Retrieves reference pointer to a value with given quantity ID name.
		\return Returns pointer to memory location with this quantity, otherwise nullptr if parameter ID was not found.
	*/
	virtual const double * resultValueRef(const QuantityName & quantityName) const override;


	// *** Re-implemented from AbstractStateDependency

	/*! Composes all input references.*/
	virtual void initInputReferences(const std::vector<AbstractModel*> & /* models */) override;

	/*! Returns vector with model input references.
		Implicit models must generate their own model input references and populate the
		vector argument.
		\note This function is not the fastest, so never call this function from within the solver
		(except maybe for output writing).
	*/
	virtual void inputReferences(std::vector<InputReference>  & inputRefs) const override;

	/*! Returns vector with pointers to memory locations matching input value references. */
	virtual const std::vector<const double *> & inputValueRefs() const override;

	/*! Sets a single input value reference (persistent memory location) that refers to the requested input reference.
		\param inputRef An input reference from the previously published list of input references.
		\param resultValueRef Persistent memory location to the variable slot.
	*/
	virtual void setInputValueRef(const InputReference &inputRef, const QuantityDescription & resultDesc, const double *resultValueRef) override;

	/*! Sums up all provided input quantities and computes divergence of balance equations. */
	int update() override;


	// *** Other public member functions

	/*! Stores the divergences of all balance equations in this zone in vector ydot. */
	int ydot(double* ydot);

private:
	/*! Construction instance ID. */
	unsigned int									m_id;
	/*! Display name (for error messages). */
	std::string										m_displayName;
	/*! True if moisture balance is enabled. */
	bool											m_moistureBalanceEnabled;
	/*! Data cache for calculated results (updated in call to update()).
		Index matches enum values of Results.
	*/
	std::vector<double>								m_results;
};

} // namespace NANDRAD_MODEL

#endif // ConstructionBalanceModelH
