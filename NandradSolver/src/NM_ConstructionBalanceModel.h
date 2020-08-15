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

namespace NANDRAD_MODEL {

/*!	This model encapsulates the whole computation functionality for a
	1D construction solver.
*/
class ConstructionBalanceModel : public AbstractModel, public AbstractStateDependency {
public:

	// ***KEYWORDLIST-START***
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
	// ***KEYWORDLIST-END***

	/*! Constructor. */
	ConstructionBalanceModel(unsigned int constructionSolverId, const std::string &displayName);

	/*! Construction solve model can be referenced via ConstructionInstance type and ID. */
	virtual NANDRAD::ModelInputReference::referenceType_t referenceType() const {
		return NANDRAD::ModelInputReference::MRT_CONSTRUCTIONINSTANCE;
	}

	/*! Return unique class ID name of implemented model. */
	virtual const char * ModelIDName() const { return "ConstructionBalanceModel"; }

	/*! Returns a priority number for the ordering in model evaluation.*/
	virtual int priorityOfModelEvaluation() const;

	/*! Sets initial states in y vector.
		This function is called after setup(), so that parameters needed for
		computing the initial condition are already present.
	*/
	void yInitial(double * y);

	/*! Stores the previously computed divergences in all elements in vector ydot.
		\return Returns 0 when calculation was successful, 1 when a recoverable error has been detected, 2 when something is badly wrong
	*/
	int ydot(double* ydot);

private:
};

} // namespace NANDRAD_MODEL

#endif // ConstructionBalanceModelH
