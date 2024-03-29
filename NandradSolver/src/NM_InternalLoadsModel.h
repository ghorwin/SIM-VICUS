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

#ifndef NM_InternalLoadsModelH
#define NM_InternalLoadsModelH

#include "NM_AbstractModel.h"
#include "NM_AbstractStateDependency.h"
#include "NM_VectorValuedQuantity.h"

#include <NANDRAD_ObjectList.h>

namespace NANDRAD {
	class InternalLoadsModel;
	class Zone;
}

namespace NANDRAD_MODEL {

/*! A model for internal loads.
	The model instance is identified by reference type MODEL and the id of the NANDRAD model parametrization block.
	It implements either constant or scheduled loads per ares and computes convective/radiant
	equipment, person and lighting loads for all zones referenced in the object list.
*/
class InternalLoadsModel : public AbstractModel, public AbstractStateDependency {
public:
	/*! Computed results, vector-valued results that provide access via zone ID. */
	enum VectorValuedResults {
		VVR_TotalElectricalPower,				// Keyword: TotalElectricalPower				[W]		'Total electrical power from equipment and lighting per zone'
		VVR_EquipmentElectricalPower,			// Keyword: EquipmentElectricalPower			[W]		'Electrical power due to electric equipment usage per zone'
		VVR_LightingElectricalPower,			// Keyword: LightingElectricalPower				[W]		'Electrical power due to lighting per zone'
		VVR_ConvectiveEquipmentHeatLoad,		// Keyword: ConvectiveEquipmentHeatLoad			[W]		'Convective heat load due to electric equipment usage per zone'
		VVR_ConvectivePersonHeatLoad,			// Keyword: ConvectivePersonHeatLoad			[W]		'Convective heat load due to person occupance per zone'
		VVR_ConvectiveLightingHeatLoad,			// Keyword: ConvectiveLightingHeatLoad			[W]		'Convective lighting heat load per zone'
		VVR_RadiantEquipmentHeatLoad,			// Keyword: RadiantEquipmentHeatLoad			[W]		'Radiant heat load due to electric equipment usage per zone'
		VVR_RadiantPersonHeatLoad,				// Keyword: RadiantPersonHeatLoad				[W]		'Radiant heat load due to person occupance per zone'
		VVR_RadiantLightingHeatLoad,			// Keyword: RadiantLightingHeatLoad				[W]		'Radiant lighting heat load per zone'
		NUM_VVR
	};

	// *** PUBLIC MEMBER FUNCTIONS

	/*! Constructor. */
	InternalLoadsModel(unsigned int id, const std::string &displayName) :
		m_id(id), m_displayName(displayName)
	{
	}

	/*! Initializes object.
		\param internalLoadsModel Model data.
		\param objLists The object list stored in the project file (persistent, remains unmodified so that persistent
			pointers to object list elements can be stored).
		\param zones List of defined zones.
	*/
	void setup(const NANDRAD::InternalLoadsModel & internalLoadsModel, const std::vector<NANDRAD::ObjectList> & objLists,
			   const std::vector<NANDRAD::Zone> & zones);

	/*! Returns object list of all referenced zones. */
	const NANDRAD::ObjectList &objectList() const;

	// *** Re-implemented from AbstractModel

	/*! Internal loads can be requested via MODEL reference. */
	virtual NANDRAD::ModelInputReference::referenceType_t referenceType() const override {
		return NANDRAD::ModelInputReference::MRT_MODEL;
	}

	/*! Return unique class ID name of implemented model. */
	virtual const char * ModelIDName() const override { return "InternalLoadsModel"; }

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
	const NANDRAD::InternalLoadsModel				*m_internalLoadsModel = nullptr;

	/*! Model instance ID (unused since results are provided for zones). */
	unsigned int									m_id;
	/*! Display name (for error messages). */
	std::string										m_displayName;

	/*! Radiation fraction (percentage of radiant emitted heat) of electric equipment. */
	double											m_eqipmentRadiationFraction = 999;

	/*! Radiation fraction (percentage of radiant emitted heat) of person load. */
	double											m_personRadiationFraction = 999;

	/*! Radiation fraction (percentage of radiant emitted heat) of lighting. */
	double											m_lightingRadiationFraction = 999;

	/*! Constant equipment load per area (only for constant model. */
	double											m_equipmentLoadPerArea = 999;

	/*! Constant person load per area (only for constant model. */
	double											m_personLoadPerArea = 999;

	/*! Constant lighting load per area (only for constant model. */
	double											m_lightingLoadPerArea = 999;

	/*! Quick access pointer to object list (for scheduled model). */
	const NANDRAD::ObjectList						*m_objectList = nullptr;

	/*! Cached pointer to zone parameters, to access zone volumes during init. */
	const std::vector<NANDRAD::Zone>				*m_zones = nullptr;

	/*! Air volumes of the zones in [m3], size matches ids in m_objectList. */
	std::vector<double>								m_zoneAreas;

	/*! Vector valued results, computed/updated during the calculation. */
	std::vector<VectorValuedQuantity>				m_vectorValuedResults;
	/*! Vector with input references. */
	std::vector<const double*>						m_valueRefs;
};

} // namespace NANDRAD_MODEL

#endif // NM_InternalLoadsModelH
