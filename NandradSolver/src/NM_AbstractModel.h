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

#ifndef AbstractModelH
#define AbstractModelH

#include <IBK_Parameter.h>
#include <IBK_UnitVector.h>

#include <NANDRAD_ModelInputReference.h>

#include "NM_QuantityDescription.h"
#include "NM_QuantityName.h"
#include "NM_VectorValuedQuantityIndex.h"

namespace NANDRAD_MODEL {

//#define DEBUG_OBJECT_SIGNATURE(x) (std::cout << x << std::endl)
//#undef DEBUG_OBJECT_SIGNATURE
#define DEBUG_OBJECT_SIGNATURE(x) ;

/*!	\brief Declaration for class AbstractModel, the base class of all models.
	\author Andreas Nicolai <andreas.nicolai -[at]- tu-dresden.de>
	This class is the base class for all models. A model should define
	a unqiue model name and provide result quantities. For each model instance
	unqiue id number should be reserved.
*/
class AbstractModel {
public:

	/*! Virtual destructor.
		\note Definition of abstractor code is in cpp file, which serves as 'first translation unit'
		and also stores virtual function table. This speeds up compilation (and possibly execution),
		since otherwise this vft is placed and compiled and linked in *every* class deriving from
		AbstractModel.
	*/
	virtual ~AbstractModel();

	/*! Returns a reference type which is unqiue in conjunction with the model name and model ID.
		The reference type is needed because the id spaces of BIM components and
		explicit models may overlap. Use ModelIDName, reference type and model ID
		to unqiuely identify a model (in order to look-up its state object in a vector of state objects).
		This function must be re-implemented by the user to provide the respective type.
		\sa NANDRAD::ModelInputReference::referenceType_t
	*/
	virtual NANDRAD::ModelInputReference::referenceType_t referenceType() const = 0;

	/*! Returns a unique class ID name of the implemented model for debugging/output purposes.
		Must be implemented by derived models.
	*/
	virtual const char * ModelIDName() const = 0;

	/*! Returns unique ID of this abstract state model. */
	virtual unsigned int id() const = 0;

	/*! Returns display name of this abstract state model. */
	virtual const std::string &displayName() const= 0;

	/*! Populates the vector resDesc with descriptions of all results provided by this model.
		\warning This function generates and populates the vector refDesc from scratch and
			is not the fastest. If you need to access the reference description several
			times, consider caching the resulting vector.
	*/
	virtual void resultDescriptions(std::vector<QuantityDescription> & resDesc) const = 0;

	/*! Returns vector of all scalar and vector valued results pointer. */
	virtual void resultValueRefs(std::vector<const double *> &res) const = 0;

	/*! Looks up a result quantity and returns a pointer to the
		double value memory location.
		This memory location must be, once reported to the framework, persistant throughout
		the lifetime of the model object.
		\return A const pointer to the double, or NULL of no such quantity was found.
		\note If a vector quantity is requested without indication of an index, the pointer returned points to
			the begin of the memory array holding the vector data.
	*/
	virtual const double * resultValueRef(const QuantityName & quantityName) const = 0;

	/*! Resizes m_results and m_vectorValuedResults vectors.
		This function is called when all models are already initialized (constructed).

		When re-implementing this class, make sure that m_results and
		m_vectorValuedResults a fully resized and defined. Ensure, that the functions
		resultDescriptions() and resultValueRef() can be called afterwards.

		The default implementation uses the keyword list and expects the
		enumerations Variables to be present in derived classes. It
		resizes and fills the m_results vectors. It then initializes
		IBK::Parameters with name and unit.

		Suggested default procedure when re-implementing this function:

		Re-implement this function and call DefaultModel::initResults().
		Resize all vector valued results (m_vectorValuedResults).
	*/
	virtual void initResults(const std::vector<AbstractModel*> & /* models */) = 0;

private:

	friend class NandradModel;
	friend class StateModelGroup;
};


} // namespace NANDRAD_MODEL

#endif // AbstractModelH
