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

#ifndef DefaultModelH
#define DefaultModelH

#include "NM_AbstractModel.h"
#include "NM_VectorValuedQuantity.h"

#include <IBK_Parameter.h>

namespace NANDRAD_MODEL {

/*!	This class declares provides a default implementation for a model. */
class DefaultModel : public AbstractModel {
public:

	/*! Standard constructor.
		An instance of a default model stores an ID and a display name for access through the
		framework. These properties must be known whenever a model instance is created.
		By default, the model copies the ID from the NANDRAD data object it is generated from
		(zone, constructionInstance, interface or model).
	*/
	DefaultModel(unsigned int id, const std::string &displayName) :
		m_id(id),
		m_displayName(displayName)
	{
	}

	/*! Returns unique ID of this model instance. */
	virtual unsigned int id() const override { return m_id; }

	/*! Returns display name of this model instance. */
	virtual const std::string &displayName() const override { return m_displayName; }

	/*! Populates the vector resDesc with descriptions of all results provided by this model
		instance. Vector valued result quantities usually are resized after the
		initResults()-routine was called. If the result descriptions are requested before
		initializing the m_vectorValuedResults vector only descriptions without index information
		are composed.
		\warning This function generates and populates the vector resDesc from the m_results
			and m_vectorValuedResults vectors and is not the fastest. If you need to access
			the reference description several times, consider caching the description vector.
	*/
	virtual void resultDescriptions(std::vector<QuantityDescription> & resDesc) const override;

	/*! Returns vector of all scalar and vector valued results pointer. */
	virtual void resultValueRefs(std::vector<const double *> &res) const override;

	/*! Looks up a referenced quantity and returns a pointer to the
		double value memory location.
		The default implementation uses the KeywordList and searches
		Variables enumerations for matching quantity names.
		The function then returns a pointer to the respective memory location.
		This memory location must be, once reported to the framework, persistant throughout
		the lifetime of the model object.
		\return A const pointer to the double, or NULL of no such quantity was found.
		\note If a vector quantity is requested without indication of an index, the pointer returned points to
			the begin of the memory array holding the vector data.
	*/
	virtual const double * resultValueRef(const QuantityName & quantityName) const override;

	/*! Transforms a target name into a target identifier for a scalar quantity.
		Returns -1 for missing quantity.
	*/
	virtual int decodeResultType(const std::string &quantity) const;

	/*! Transforms a target name into a target identifier for a vector valued quantity.
		Returns -1 for missing quantity.
	*/
	virtual int decodeVectorValuedResultType(const std::string &quantity) const;

protected:

	/*! Invalid index, returned from vectorVariableIndex(). */
	static const unsigned int InvalidVectorIndex;

	/*! Resizes m_results and m_vectorValuedResults vectors.
		This function is called when all models are already initialized (constructed).

		When re-implementing this class, make sure that m_results and
		m_vectorValuedResults a fully resized and defined. Ensure, that the functions
		resultDescriptions() and resultValueRef() can be called afterwards.

		The default implementation uses the keyword list and expects the
		enumerations Variables to be present in derived classes. It
		resizes and fills the m_results and m_vectorValuedResults vectors.
		It then initializes IBK::Parameters/IBK::UnitLists with name and unit.

		Suggested default procedure when re-implementing this function:

		Re-implement this function and call DefaultModel::initResults().
		Resize all vector-valued results with a given set of indices afterwards.
	*/
	virtual void initResults(const std::vector<AbstractModel*> & models) override;

	/*! Informs the model that a step was successfully completed.
		The time point and value vector passed to the function correspond to
		the current state in the integrator object.
		This function can be used to write restart info.
		Default implementation does nothing.
	*/
	virtual void stepCompleted(double /*t*/) { }

	/*! Results, computed/updated during the calculation. */
	std::vector<IBK::Parameter>			m_results;
	/*! Vector valued results, computed/updated during the calculation. */
	std::vector<VectorValuedQuantity>	m_vectorValuedResults;

private:
	/*! Copy constructor is private and not implemented (disable copy). */
	DefaultModel(const DefaultModel &);
	/*! Assignment operator is private and not implemented (disable copy). */
	const DefaultModel & operator=(const DefaultModel &);

	/*! The (unique) ID of this model instance. */
	unsigned int						m_id;
	/*! The displayName of this model instance. */
	std::string							m_displayName;
};


} // namespace NANDRAD_MODEL

#endif // DefaultModelH
