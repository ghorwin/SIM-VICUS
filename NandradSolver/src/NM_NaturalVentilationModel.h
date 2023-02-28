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

#ifndef NM_NaturalVentilationModelH
#define NM_NaturalVentilationModelH

#include "NM_AbstractModel.h"
#include "NM_AbstractStateDependency.h"
#include "NM_VectorValuedQuantity.h"

namespace NANDRAD {
	class SimulationParameter;
	class NaturalVentilationModel;
	class Zone;
	class ObjectList;
}

namespace NANDRAD_MODEL {

/*! A model for natural ventilation rate.
	The model instance is identified by reference type MODEL and the id of the NANDRAD model parametrization block.
	It implements either constant or scheduled ventilation rates and computes thermal ventilation loads
	for all zones referenced in the object list.

	Ventilation model supports both thermal and hygrothermal calculations. In the latter case, the
	VentilationMoistureMassFlux is provided. The enthalpy and sensible heat is always computed together in
	VentilationHeatFlux.
*/
class NaturalVentilationModel : public AbstractModel, public AbstractStateDependency {
public:
	/*! Computed results, provided with access via zone ID. */
	enum VectorValuedResults {
		VVR_VentilationRate,				// Keyword: VentilationRate					[1/h]	'Natural ventilation/infiltration air change rate'
		VVR_VentilationHeatFlux,			// Keyword: VentilationHeatFlux				[W]		'Natural ventilation/infiltration heat flux'
		VVR_VentilationMoistureMassFlux,	// Keyword: VentilationMoistureMassFlux		[kg/s]	'Natural ventilation/infiltration moisture mass flux'
		NUM_VVR
	};

	// *** PUBLIC MEMBER FUNCTIONS

	/*! Constructor. */
	NaturalVentilationModel(unsigned int id, const std::string &displayName) :
		m_id(id), m_displayName(displayName)
	{
	}

	/*! Initializes object.
		\param ventilationModel Ventilation model data.
		\param simPara Required simulation parameter.
		\param objLists The object list stored in the project file (persistent, remains unmodified so that persistent
			pointers to object list elements can be stored).
	*/
	void setup(const NANDRAD::NaturalVentilationModel & ventilationModel, const NANDRAD::SimulationParameter &simPara,
			   const std::vector<NANDRAD::ObjectList> & objLists, const std::vector<NANDRAD::Zone> & zones);


	// *** Re-implemented from AbstractModel

	/*! Thermal ventilation loads can be requested via MODEL reference. */
	virtual NANDRAD::ModelInputReference::referenceType_t referenceType() const override {
		return NANDRAD::ModelInputReference::MRT_MODEL;
	}

	/*! Return unique class ID name of implemented model. */
	virtual const char * ModelIDName() const override { return "NaturalVentilationModel"; }

	/*! Returns unique ID of this model instance. */
	virtual unsigned int id() const override { return m_id; }

	/*! Resizes m_results vector. */
	virtual void initResults(const std::vector<AbstractModel*> & /* models */) override;

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
	virtual void stateDependencies(std::vector< std::pair<const double *, const double *> > & resultInputValueReferences) const override;

	/*! Sums up all provided input quantities and computes divergence of balance equations. */
	int update() override;


private:
	/*! Constant pointer to the referenced zone parameter block. */
	const NANDRAD::NaturalVentilationModel			*m_ventilationModel = nullptr;
	/*! Reference to simulation parameter block. */
	const NANDRAD::SimulationParameter				*m_simPara = nullptr;

	/*! Model instance ID (unused since results are provided for zones). */
	unsigned int									m_id;
	/*! Display name (for error messages). */
	std::string										m_displayName;
	/*! True if moisture balance is enabled. */
	bool											m_moistureBalanceEnabled;

	/*! Constant ventilation rate in 1/s (only for constant model). */
	double											m_ventilationRate = 999;

	/*! Quick access pointer to object list (for scheduled model). */
	const NANDRAD::ObjectList						*m_objectList = nullptr;

	/*! Cached pointer to zone parameters, to access zone volumes during init. */
	const std::vector<NANDRAD::Zone>				*m_zones = nullptr;

	/*! Air volumes of the zones in [m3], size matches ids in m_objectList. */
	std::vector<double>								m_zoneVolumes;

	/*! Vector valued results, computed/updated during the calculation. */
	std::vector<VectorValuedQuantity>				m_vectorValuedResults;
	/*! Vector with input references. */
	std::vector<const double*>						m_valueRefs;

	/*! Offset in inputRefs/valueRefs vector where zone-specific variables start */
	mutable unsigned int							m_zoneVariableOffset;
	/*! Number of zone-dependent variables in inputRefs/valueRefs vector. */
	mutable unsigned int							m_zoneVariableCount;
};

} // namespace NANDRAD_MODEL

#endif // NM_NaturalVentilationModelH
