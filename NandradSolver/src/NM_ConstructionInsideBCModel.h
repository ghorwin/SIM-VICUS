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

#ifndef ConstructionInsideBCModelH
#define ConstructionInsideBCModelH

#include "NM_DefaultModel.h"
#include "NM_DefaultStateDependency.h"

#include <NANDRAD_ConstructionInstance.h>
#include <NANDRAD_Interface.h>

namespace NANDRAD {
	class ParametrizationDefaults;
}

namespace NANDRAD_MODEL {

/*!	Implements indoors boundary condition for construction solvers.

	The heat flux due to heat conduction is calculated using heat transfer coefficient (different models),
	the zone temperature from the RoomStates model and the surface temperature from the respective construction
	solver model (and location A or B).
	For heat fluxes arising from long wave and short wave radiation exchange it is actually
	a collector model, because it only retrieves already computed quantities from
	the balance models. In case these quantities are not available, the fluxes are taken to be zero.

	\todo Implement the radiation part. For now only heat conduction is implemented.
*/
class ConstructionInsideBCModel : public DefaultModel, public DefaultStateDependency {
public:

	enum Results {
		R_HeatConduction,				  // Keyword: HeatConduction			[W/m2]	'Heat conduction (flux density).'
		R_HeatConductionFlux,			  // Keyword: HeatConductionFlux		[W]		'Heat conduction (flux).'
		R_LWRadBalance,					  // Keyword: LWRadBalance				[W/m2]	'Long wave radiation balance (flux density).'
		R_LWRadBalanceFlux,				  // Keyword: LWRadBalanceFlux			[W]		'Long wave radiation balance (flux).'
		R_SWRadAbsorbed,				  // Keyword: SWRadAbsorbed				[W/m2]	'Absorbed heat flux by short wave radiation at the wall surface  (flux density).'
		R_SWRadAbsorbedFlux,			  // Keyword: SWRadAbsorbedFlux			[W]		'Absorbed heat flux by short wave radiation at the window surface  (flux).'
		R_Area,							  // Keyword: Area						[m2]	'Surface netto area'
		NUM_R
	};

	enum InputReferences {
		InputRef_AirTemperature,		  // Keyword: AirTemperature			[C]		'Air temperature of the neighbor room.'
		InputRef_SWRadWindow,			  // Keyword: SWRadWindow				[W/m2]	'Short wave radiation at inside wall (flux density).'
		InputRef_SWRadLighting,			  // Keyword: SWRadLighting				[W/m2]	'Short wave radiation due to lighting at inside wall (flux density).'
		InputRef_SWRadExchange,			  // Keyword: SWRadExchange				[W/m2]	'Short wave radiation exchange gains at inside wall (flux density).'
		InputRef_LWRadExchange,			  // Keyword: LWRadExchange				[W/m2]	'Long wave radiation exchange gains at inside wall (flux density).'
		InputRef_LWRadHeating,			  // Keyword: LWRadHeating				[W/m2]	'Long wave radiation due to heating at inside wall (flux density).'
		InputRef_LWRadCooling,			  // Keyword: LWRadCooling				[W/m2]	'Long wave radiation due to cooling at inside wall (flux density).'
		InputRef_LWRadUserLoad,			  // Keyword: LWRadUserLoad				[W/m2]	'Long wave radiation due to user load at inside wall (flux density).'
		InputRef_LWRadEquipmentLoad,	  // Keyword: LWRadEquipmentLoad		[W/m2]	'Long wave radiation due to equipment load at inside wall (flux density).'
		InputRef_LWRadLighting,			  // Keyword: LWRadLighting				[W/m2]	'Long wave radiation due to lighting load at inside wall (flux density).'
		NUM_InputRef
	};

	/*! Default constructor. */
	ConstructionInsideBCModel(unsigned int id, const std::string &displayName);

	/*! Setup of inside boundary condition model.
		Initializes model data and stores surface + wall information.

		\param iface Contains parametrization of interface (location...)
		\param conInstance Parametrization for construction
	*/
	void setup(const NANDRAD::Interface & iface,
		const NANDRAD::ConstructionInstance & conInstance);


	// *** Re-implemented from AbstractModel

	/*! Balance model can be referenced as ConstructionInstance and ID. */
	virtual NANDRAD::ModelInputReference::referenceType_t referenceType() const override {
		return NANDRAD::ModelInputReference::MRT_INTERFACE;
	}

	/*! Return unique class ID name of implemented model. */
	virtual const char * ModelIDName() const override { return "ConstructionInsideBCModel";}

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

	/*! Returns pointer to the connected interface. */
	const NANDRAD::Interface *iface() const {return m_interface;}

	/*! Returns pointer to the connected construction. */
	const NANDRAD::ConstructionInstance *constructionInstance() const {return m_constructionInstance;}

private:
	/*! Constant reference to interface. */
	const NANDRAD::Interface				*m_interface;
	/*! Constant reference to the connected construction instance. */
	const NANDRAD::ConstructionInstance		*m_constructionInstance;
};

} // namespace NANDRAD_MODEL

#endif // ConstructionInsideBCModelH
