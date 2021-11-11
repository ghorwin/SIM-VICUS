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

#ifndef NM_AbstractModelH
#define NM_AbstractModelH

#include <NANDRAD_ModelInputReference.h>

#include <map>

#include "NM_QuantityDescription.h"
#include "NM_InputReference.h"

namespace NANDRAD_MODEL {

/*!	This is the base class of all models (that generate results).
	A model should define a unqiue ModelIDName and provide result quantities. For each model
	instance a unqiue id number should be reserved.
*/
class AbstractModel {
public:
	/*! Default C'tor. */
	AbstractModel() {}
	/*! Copy constructor is not available (disable copy). */
	AbstractModel(const AbstractModel &) = delete;
	/*! Assignment operator is not available (disable copy). */
	const AbstractModel & operator=(const AbstractModel &) = delete;

	/*! Virtual destructor.
		\note Definition of abstractor code is in cpp file, which serves as 'first translation unit'
		and also stores virtual function table. This speeds up compilation (and possibly execution),
		since otherwise this vft is placed and compiled and linked in *every* class deriving from
		AbstractModel.
	*/
	virtual ~AbstractModel();

	/*! Returns a reference type which in conjunction with the model ID allows identification
		of model object(s), generated from the same parametrization block.
		This can be a single model object in some cases, but in others (e.g. Zone) there will
		be several objects with the same reference type (e.g. MRT_ZONE) and the same zone ID.
		Among these objects, all published variables must have unique names!

		This function must be re-implemented by model objects that provide result variables.
		Default implementation returns invalid reference type (i.e. no results available).
		\sa NANDRAD::ModelInputReference::referenceType_t
	*/
	virtual NANDRAD::ModelInputReference::referenceType_t referenceType() const { return NANDRAD::ModelInputReference::NUM_MRT; }

	/*! Returns a unique class ID name of the implemented model for debugging/output purposes.
		Should be implemented by derived models.
		Default implementation returns an empty string.
	*/
	virtual const char * ModelIDName() const { return ""; }

	/*! Returns unique ID of this abstract state model.
		Must be implemented by derived models that generate results.
		Default implementation returns 0.
	*/
	virtual unsigned int id() const { return 0; }

	/*! Returns display name of this abstract state model.
		Should be implemented by derived models.
		Default implementation returns an empty string.
	*/
	virtual const char * displayName() const { return ""; }

	/*! This function is called when all models are already initialized (constructed).

		Should be implemented by derived models that generate results.
		Default implementation does nothing.

		When re-implementing this class, ensure, that the functions
		resultDescriptions() and resultValueRef() can be called afterwards.

		For many models the available results are already known when setup() is called.
		However, some models may need to know which dependend models are in the
		list of models. These models may search through the provided model list
		and determine the outputs to be generated.

		\param models Vector of all models (already initialized).
	*/
	virtual void initResults(const std::vector<AbstractModel*> & models) { (void)models; }

	/*! Populates the vector resDesc with descriptions of all results provided by this model.
		Must be implemented by derived models that generate results.
		Default implementation does nothing (i.e. no results generated).

		\warning This function generates and populates the vector refDesc from scratch and
			may not be the fastest. If you need to access the reference description several
			times, consider caching the resulting vector.
	*/
	virtual void resultDescriptions(std::vector<QuantityDescription> & resDesc) const { (void)resDesc; }

	/*! Looks up a result quantity and returns a pointer to the double value memory location.
		Must be implemented by derived models that generate results.
		Default implementation returns a nullptr (i.e. variable does not exist).

		This memory location must be, once reported to the framework, persistant throughout
		the lifetime of the model object.

		\return A const pointer to the double, or nullptr of no such quantity was found.
		\note If a vector quantity is requested without indication of an index, the pointer returned points to
			the begin of the memory array holding the vector data.

		\warning Function may throw an exception in case of invalid indexes for vector-valued quantities. In these cases
			the caller must be notified of possibly erroneous input data, so make sure to wrap the call to resultValueRef()
			in a try-catch-clause!
	*/
	virtual const double * resultValueRef(const InputReference & quantity) const { (void)quantity; return nullptr; }

	/*! Returns a list of optional variable name substitutions, to be generated by each model that generates result references.
		When constructing reference names, such as "Zone(id=12)", the same naming conventions are to be used as in the
		output manager.
	*/
	virtual void variableReferenceSubstitutionMap(std::map<std::string, std::string> & varSubstMap) { (void)varSubstMap; }

	/*! Computes and returns serialization size in bytes, by default returns  returns an invalid value (-1). */
	virtual std::size_t serializationSize() const { return 0; }

	/*! Stores model content at memory location pointed to by dataPtr and increases
		pointer afterwards to point just behind the memory occupied by the copied data.
		Default implementation does nothing.
	*/
	virtual void serialize(void* & dataPtr) const { (void)dataPtr; }

	/*! Restores model content from memory at location pointed to by dataPtr and increases
		pointer afterwards to point just behind the memory occupied by the copied data.
		Default implementation does nothing.
	*/
	virtual void deserialize(void* & dataPtr) { (void)dataPtr; }

};


} // namespace NANDRAD_MODEL

#endif // NM_AbstractModelH
