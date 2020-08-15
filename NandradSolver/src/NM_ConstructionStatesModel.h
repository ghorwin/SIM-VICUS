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

#ifndef ConstructionStatesModelH
#define ConstructionStatesModelH


#include "NM_DefaultModel.h"
#include "NM_DefaultStateDependency.h"

namespace NANDRAD {
	class ConstructionInstance;
	class SimulationParameter;
}

namespace NANDRAD_MODEL {


/*!	This model computes temperatures (and if moisture balance is enabled in constructions, also
	intensive moisture variables like relative humidity/vapor pressure) from the conserved quantities.

	It is updated first during the model update (a "head" model).
*/
class ConstructionStatesModel : public AbstractModel {
public:

	// ***KEYWORDLIST-START***
	enum VectorValuedResults {
		VVR_LayerTemperature			// Keyword: LayerTemperature		[C]		'Mean layer temperature for requested quanties.'
	};
	enum Results {
		R_SurfaceTemperatureA,			// Keyword: SurfaceTemperatureA		[C]		'Surface temperature at interface A.'
		R_SurfaceTemperatureB			// Keyword: SurfaceTemperatureB		[C]		'Surface temperature at interface B.'
	};
	enum InputReferences {
		InputRef_InternalEnergyDensity,	// Keyword: InternalEnergyDensity	[W/m3]	'Internal energy density of the wall.'
		InputRef_Temperature,			// Keyword: Temperature				[C]		'Wall temperatures.'
		InputRef_HeatSources,			// Keyword: HeatSources				[W/m3]	'Wall heat sources.'
		NUM_InputRef
	};
	// ***KEYWORDLIST-END***

	/*! Constructor, relays ID to DefaultStateDependency constructor. */
	ConstructionStatesModel(unsigned int id, const std::string &displayName) :
		m_id(id), m_displayName(displayName)
	{
	}


	/*! Construction solve model can be referenced via ConstructionInstance type and ID. */
	virtual NANDRAD::ModelInputReference::referenceType_t referenceType() const {
		return NANDRAD::ModelInputReference::MRT_CONSTRUCTIONINSTANCE;
	}

	/*! Return unique class ID name of implemented model. */
	virtual const char * ModelIDName() const { return "ConstructionStatesModel";}

	/*! Initializes model.
	*/
	void setup(const NANDRAD::ConstructionInstance & con, const NANDRAD::SimulationParameter & simPara);

	/*! Deactive initResults(). States model does not own a results vector.*/
	virtual void initResults(const std::vector<AbstractModel*> & models);

	/*! Populates the vector resDesc with descriptions of all results provided by this model. */
	virtual void resultDescriptions(std::vector<QuantityDescription> & resDesc) const;

	/*! Returns vector of all scalar and vector valued results pointer. */
	virtual void resultValueRefs(std::vector<const double *> &res) const;

	/*! Retrieves reference pointer to a value with given quantity ID name.
		\return Returns pointer to memory location with this quantity, otherwise NULL if parameter ID was not found.
	*/
	virtual const double * resultValueRef(const QuantityName & quantityName) const;


protected:
	std::vector<double>				m_results;

	/*! Results vector for mean layer temperatures (requested by any model input reference). */
	VectorValuedQuantity			m_layerTemperatures;
};

} // namespace NANDRAD_MODEL

#endif // ConstructionStatesModelH
