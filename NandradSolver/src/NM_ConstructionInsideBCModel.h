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
#include "NM_ConstructionBoundaryCondition.h"

#include <NANDRAD_ConstructionInstance.h>
#include <NANDRAD_Interface.h>

namespace NANDRAD {
	class ParametrizationDefaults;
}

namespace NANDRAD_MODEL {

/*!	\brief Declaration for class ConstructionInsideBCModel
	\author Andreas Nicolai <andreas.nicolai -[at]- tu-dresden.de>

	Implements indoors boundary condition for wall solver modules. The heat flux
	due to heat conduction is calculated using heat transfer coefficient from
	a constant model (explicit models will also be allowed respectively).
	This model needs the zone temperature from the RoomStates model.
	It also needs the surface temperature from the respective construction
	solver model (and location A or B)
	For heat fluxes arising from long wave and short wave radiation exchange it is actually
	a collector model, because it only retrieves already computed quantities from
	the balance models.
*/
class ConstructionInsideBCModel : public DefaultModel, public DefaultStateDependency,
	public ConstructionBoundaryCondition {

public:
	// ***KEYWORDLIST-START***
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
		InputRef_HeatTransferCoefficient, // Keyword: HeatTransferCoefficient	[W/m2K]	'Heat Transfer Coefficient'
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
	// ***KEYWORDLIST-END***

	/*! Default constructor. */
	ConstructionInsideBCModel(unsigned int id, const std::string &displayName);

	/*! Return unique class ID name of implemented model. */
	virtual const char * ModelIDName() const { return "ConstructionInsideBCModel";}

	/*! This model can be referenced by the type Interface and the corresponding interface ID. */
	virtual NANDRAD::ModelInputReference::referenceType_t referenceType() const {
		return NANDRAD::ModelInputReference::MRT_INTERFACE;
	}


	/*! Returns pointer to the connected interface. */
	const NANDRAD::Interface *iface() const {return m_interface;}

	/*! Returns pointer to the connected construction. */
	const NANDRAD::ConstructionInstance *constructionInstance() const {return m_constructionInstance;}

	/*! Returns a priority number for the ordering in model evaluation.*/
	virtual int priorityOfModelEvaluation() const;

	/*! Setup of inside boundary condition model.
		Initializes model data and stores surface + wall information.

		\param iface Contains parametrization of interface (location...)
		\param conInstance Parametrization for construction
	*/
	void setup(const NANDRAD::Interface & iface,
		const NANDRAD::ConstructionInstance & conInstance);

	/*! Special implementation for quantity 'Surface temperature' (output property).
	*/
	virtual void resultDescriptions(std::vector<QuantityDescription> & resDesc) const;

	/*! Returns vector of all scalar and vector valued results pointer.
	*/
	virtual void resultValueRefs(std::vector<const double *> &res) const;

	/*! Special implementation for quantity 'Surface temperature' (output property).
	*/
	virtual const double * resultValueRef(const QuantityName & quantityName) const;

	/*! Composes m_results vector.*/

	virtual void initResults(const std::vector<AbstractModel*> & models);

	/*! Composes m_inputReferences vector.*/

	virtual void initInputReferences(const std::vector<AbstractModel*> & models);

	/*! Retrieves new surface and zone temperatures and re-computes heat conduction flux. */
	virtual int update();

	/*! Returns all dependency pairs: heat conduction, short wave and long wave radiation
		couple to different input references.
	*/
	virtual void stateDependencies(std::vector< std::pair<const double *, const double *> > &resultInputValueReferences) const;

	/*! Function for accessing heat conduction flux density from construction solver.
	*/
	virtual const double *heatConduction() const;

	/*! Function for accessing long wave balance flux density from construction solver.
	*/
	virtual const double *lwRadBalance() const;

	/*! Function for accessing short wave absorbed flux density from construction solver.
	*/
	virtual const double *swRadAbsorbed() const;

	/*! Function for accessing solar imposed flux density from construction solver.
	*/
	virtual const double *swRadOnPlane() const { return nullptr; }

	/*! Function for accessing vapor diffusion flux density from construction solver (= nullptr for
	thermal calculation).
	*/
	virtual const double *vaporDiffusion() const { return nullptr; }

	/*! Function for accessing vapor enthalpy flux density from construction solver (= nullptr for
	thermal calculation).
	*/
	virtual const double *vaporEnthalpy() const { return nullptr; }

	/*! Function for settuing surface temperature. Does nothing.
	*/
	virtual void setSurfaceTemperature(const double *temperature);

	/*! Function for setting relative humidity.Does nothing.
	*/
	virtual void setSurfaceRelativeHumidity(const double *relativeHumidity) { }

	/*! Function for setting vapor pressure.Does nothing.
	*/
	virtual void setSurfaceVaporPressure(const double *vaporPressure) { }

protected:
	/*! Store surface temperature for flux calculation (set from ConstructionSolverModel). */
	const double							*m_surfaceTemperature;
	/*! Constant reference to interface. */
	const NANDRAD::Interface				*m_interface;
	/*! Constant reference to the connected construction instance. */
	const NANDRAD::ConstructionInstance		*m_constructionInstance;
};

} // namespace NANDRAD_MODEL

#endif // ConstructionInsideBCModelH
