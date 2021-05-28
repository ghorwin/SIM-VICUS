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

#ifndef NM_DefaultModelH
#define NM_DefaultModelH

#include "NM_AbstractModel.h"
#include "NM_VectorValuedQuantity.h"

#include <IBK_Parameter.h>

namespace NANDRAD_MODEL {

/*!	This class declares provides a default implementation for a model.
	The class is a convenience implementation that provides defaults for
	publishing calculation results based on enumeration values.

	It expects implementing classes to provide the enumerations Results and
	VectorValuedResults (both optional), with corresponding keyword definitions.
	Then, the default implementation will create storage space for the
	computed quantities (m_results and m_vectorValuedResults) and generate
	QuantityDescription for each provided result.
*/
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
	virtual const char * displayName() const override { return m_displayName.c_str(); }

	/*! Populates the vector resDesc with descriptions of all results based on enumerations Results
		and VectorValuedResults and the KeywordList.
		\warning This function generates and populates the vector resDesc using the KeywordList. It includes
			quite a lot of string operations and, hence, the function is not the fastest. If you need
			the descriptions several times, you should consider caching the vector.
		\note Vector-valued result quantities usually are resized after the
			initResults()-routine was called. If the result descriptions are requested before
			initializing the m_vectorValuedResults vector only descriptions without index information
			are composed. If you know the size of the vectors already during setup, it is recommended that you
			re-implement this function in the derived class and after a call to the DefaultModel::resultDescription()
			function, adjust the genererated default descriptions for the vector-valued quantities.
	*/
	virtual void resultDescriptions(std::vector<QuantityDescription> & resDesc) const override;

	/*! Returns vector of all scalar and vector-valued results pointer.
		\param res Vector with pointers to resulting values. First the addresses to all m_value members in
			the m_results vector are added, then the addresses of the vector valued quantities are added.
	*/
	virtual void resultValueRefs(std::vector<const double *> &res) const override;

	/*! Looks up a referenced quantity and returns a pointer to the double value memory location.
		The default implementation uses the KeywordList and searches enumeration 'Results' for matching quantity names.
		The function then returns a pointer to the respective memory location.
		\return A const pointer to the double, or nullptr of no such quantity was found.
		\note If a vector quantity is requested without indication of an index, the pointer returned points to
			the begin of the memory array holding the vector data.

		\warning Function throws an exception in case of invalid indexes for vector-valued quantities
	*/
	virtual const double * resultValueRef(const InputReference & quantity) const override;

protected:

	/*! Invalid index, returned from vectorVariableIndex(). */
	static const unsigned int InvalidVectorIndex;

	/*! Resizes m_results and m_vectorValuedResults vectors.
		This function is called when all models are already initialized (constructed).

		When re-implementing this class, make sure that m_results and
		m_vectorValuedResults a fully resized and defined. Ensure, that the function
		resultValueRef() can be called afterwards.

		The default implementation first calls resultDescriptions() to obtain
		a list of published variables (see default implementation in DefaultModel::resultDescriptions()).
		Then, it resizes and fills the m_results and m_vectorValuedResults vectors.
	*/
	virtual void initResults(const std::vector<AbstractModel*> & models) override;


	/*! Transforms a target name into a target identifier for a scalar quantity.
		Returns -1 for missing quantity.
	*/
	int decodeResultType(const std::string &quantity) const;

	/*! Transforms a target name into a target identifier for a vector valued quantity.
		Returns -1 for missing quantity.
	*/
	int decodeVectorValuedResultType(const std::string &quantity) const;

	/*! Results, computed/updated during the calculation.
		The values are stored in the base SI units of the quantities.
		\note Even if the Results enum lists a parameter with (io-unit) [C], the actual calculated
			value *must be* stored in m_results in [K].
	*/
	std::vector<double>					m_results;
	/*! Vector valued results, computed/updated during the calculation. */
	std::vector<VectorValuedQuantity>	m_vectorValuedResults;

private:

	/*! The (unique) ID of this model instance. */
	unsigned int						m_id;
	/*! The displayName of this model instance. */
	std::string							m_displayName;
};


} // namespace NANDRAD_MODEL

#endif // NM_DefaultModelH
