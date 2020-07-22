/*	The Nandrad model library.

Copyright (c) 2012, Institut fuer Bauklimatik, TU Dresden, Germany

Written by
A. Paepcke		<anne.paepcke -[at]- tu-dresden.de>
A. Nicolai		<andreas.nicolai -[at]- tu-dresden.de>
All rights reserved.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 3 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.
*/

#ifndef ThermalComfortModelH
#define ThermalComfortModelH

#include "NM_DefaultModel.h"
#include "NM_DefaultStateDependency.h"

namespace NANDRAD
{
	class Interface;
}

namespace NANDRAD_MODEL {

/*!	\brief Declaration for class ThermalComfortModel
	\author Andreas Nicolai <andreas.nicolai -[at]- tu-dresden.de>

	The thermal comfort model calculates radiant temperature and operative
	temperature for one zone. We obtain the radiant temperature by a area-weighted 
	average of the surface temperatures of the enclosing walls and compose input 
	references to all inside surface temperatures and wall areas.
	Note, that the current input references may be substituted by quantities
	resulting from a more precise calculation rule, i.e. the references to wall 
	area weighting factors may be substituted by precalculated view factors.
*/
class ThermalComfortModel : public DefaultModel, public DefaultStateDependency {
public:
	// ***KEYWORDLIST-START***
	enum Results {
		R_RadiantTemperature,				// Keyword: RadiantTemperature		[C]		'Mean surface temperature of all surfaces facing the room.'
		R_OperativeTemperature,				// Keyword: OperativeTemperature	[C]		'Operative temperature of the room.'
		NUM_R
	};
	enum InputReferences {
		InputRef_AirTemperature,			// Keyword: AirTemperature			[C]		'Air temperature of the room.'
		InputRef_RadiantTemperature,		// Keyword: RadiantTemperature		[C]		'Wall radiant temperature.'
		InputRef_Area,						// Keyword: Area					[m2]	'Wall surface area'
		NUM_InputRef
	};
	// ***KEYWORDLIST-END***

	/*! Constructor, relays ID to DefaultModel constructor. */
	ThermalComfortModel(unsigned int id, const std::string &displayName):
		DefaultModel(id, displayName),
		DefaultStateDependency(SteadyState)
	{
	}

	/*! Return unique class ID name of implemented model. */
	virtual const char * ModelIDName() const { return "ThermalComfortModel";}

	/*! This model can be referenced by the type Interface and the corresponding interface ID. */
	NANDRAD::ModelInputReference::referenceType_t referenceType() const { return NANDRAD::ModelInputReference::MRT_ZONE; }

	/*! Returns a priority number for the ordering in model evaluation.*/
	virtual int priorityOfModelEvaluation() const;

	/*! At the moment this function is meaningless.
	*/
	void setup() { }

	/*! Populates the vector refDesc with descriptions of all input references
	requested by this model. We exclude inactive references.
	*/
	virtual void inputReferenceDescriptions(std::vector<QuantityDescription> & refDesc) const;

	/*! Stores all surrounding interfaces and construction instances.
		Called when all models are already initialised.
	*/
	virtual void initInputReferences(const std::vector<AbstractModel*> & models);

	/*! Computes all thermal comfort result quantities and fills the m_results vector. */
	virtual int update();

	/*! Returns all dependency pairs: radiant temperature and room air temperature
		are not connected. 
	*/
	virtual void stateDependencies(std::vector< std::pair<const double *, const double *> > &resultInputValueReferences) const;

private:
	/*! Pointer to all surface ids with view to current room. */
	std::set<unsigned int>								m_surfaceIds;
};

} // namespace NANDRAD_MODEL

#endif // ThermalComfortModelH
