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

#ifndef NM_QuantityDescriptionH
#define NM_QuantityDescriptionH

#include <limits>
#include <vector>

#include <IBK_Unit.h>

#include <NANDRAD_CodeGenMacros.h>
#include <NANDRAD_ModelInputReference.h>
#include <NANDRAD_Constants.h>

#include "NM_VectorValuedQuantityIndex.h"

namespace NANDRAD_MODEL {

/*! Data type that uniquely identifies a (result) variable.

	Reference type (m_referenceType) and id (m_id) identify the object that generates the data.
	Variable name (m_name) identifies the name of the variable to retrieve (may be a vector-valued variable name, without
	index of course).

	This data structure is used to determine the object a variable is requested from. The results
	from all objects/model instances are stored in a map (from variable reference to object), hence
	the operator< in this class.

	When looking up requested input variables from models, we simply search for this quantity descriptino, and hereby determine
	object and id and eventually get the object instance that provides the variable we need. Then, we can ask
	the object instance to give us a persistent pointer to the result variable's memory location.

	This is the datatype returned by resultDescriptions() and inputReferenceDescriptions(). It contains
	definitions of result values and input references as name and physical unit. For
	vector-valued results or input references to vector-valued target quantities additional
	vector information (size, indices of occupied vector elements and index key type) is provided.

	This description is obtained from each model instance generating results. Since there may be several instances
	(with different IDs), the actual value reference is obtained by completing this description with
	type and ID of the generating model instance.

	For example: a RoomStateModel generates an AirTemperature. Since we may have several rooms, each of the room instances
	will provide a memory slot with the air temperature. So once we have the quantity description, we expand by adding
	reference type ZONE and ids of the zone to actually 3 value references.
*/
class QuantityDescription {
public:

	/*! Default constructor. */
	QuantityDescription() :
		m_referenceType(NANDRAD::ModelInputReference::NUM_MRT),
		m_minMaxValue(std::make_pair<double, double>
		(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max()))
	{}

	/*! Constructor for scalar valued quantities.*/
	QuantityDescription(const std::string & name, const std::string & unit, const std::string & description,
						const bool constant) :
		m_referenceType(NANDRAD::ModelInputReference::NUM_MRT),
		m_name(name),
		m_unit(unit),
		m_description(description),
		m_minMaxValue(std::make_pair<double, double>(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max())),
		m_constant(constant)
	{
	}

	/*! Constructor for vector valued quantities.*/
	QuantityDescription(const std::string & name, const std::string & unit, const std::string & description,
						const bool constant, VectorValuedQuantityIndex::IndexKeyType indexKeyType,
						const std::vector<unsigned int> &indexKeys) :
		m_referenceType(NANDRAD::ModelInputReference::NUM_MRT),
		m_name(name),
		m_unit(unit),
		m_description(description),
		m_minMaxValue(std::make_pair<double, double>
		(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max())),
		m_constant(constant),
		m_size((unsigned int) indexKeys.size()),
		m_indexKeyType(indexKeyType),
		m_indexKeys(indexKeys)
	{
	}

	/*! Comparison operator for sorting result quantities.
		Sorts quantities based on reference type and id and name. This operator is used
		to find a QuantityDescription in a map, when id, refType and variable name are give
		(in an input reference, for example).
	*/
	bool operator<(const QuantityDescription &other) const {
		if (m_referenceType < other.m_referenceType)
			return true;
		if (m_referenceType > other.m_referenceType)
			return false;
		if (m_id < other.m_id)
			return true;
		if (m_id > other.m_id)
			return false;

		// string comparison last, because slowest
		return (m_name < other.m_name);
	}

	/*! Sets index key type and indexes to initialize vector valued quantity description. */
	void resize(const std::vector<unsigned int> &indexes, VectorValuedQuantityIndex::IndexKeyType keyType) {
		m_indexKeys = indexes;
		m_indexKeyType = keyType;
		m_size = indexes.size();
	}


	/*! Resizes index set to given size (using continuous numbering) and sets indexKeyType to IK_Index. */
	void resize(unsigned int size) {
		m_indexKeys.clear();

		// resize index and index description vectors
		for (unsigned int i = 0;  i < size; ++i)
			m_indexKeys.push_back(i);

		m_indexKeyType = VectorValuedQuantityIndex::IK_Index;
		m_size = size;
	}


	/*! Declare comparison operator with string to find QuantityDescription by m_name. */
	NANDRAD_COMPARE_WITH_NAME

	// *** General attributes ***

	/*! ReferenceType: zone, model or constructionInstance, etc.
		Note: the reference type is NUM_MRT by default. Unless explicitely set by models providing QuantityDescriptions,
			  reference type and ID are set by the framework based on the exporting model object.
	*/
	NANDRAD::ModelInputReference::referenceType_t	m_referenceType = NANDRAD::ModelInputReference::NUM_MRT;
	/*! ID of the referenced source model/parametrization entity. */
	unsigned int							m_id = NANDRAD::INVALID_ID;

	/*! Quantity/variable name.
		This is used together with the reference type name and object id to form the output variable name. This
		ensure a consistent and unqiue naming of output variables across all generated output files.
	*/
	std::string								m_name;
	/*! Quantity unit, which is actually the input/output unit used for logging value in output files.
		\note The value stored in the actual memory location corresponding to this variable is *always*
			in the base SI unit.
	*/
	std::string								m_unit;
	/*! Description string for the current quantity (optional). */
	std::string								m_description;
	/*! Lower and upper value limit, (-\infty, +\infty) perdefault */
	std::pair<double, double>				m_minMaxValue;
	/*! Attribute determining whether the quantity is a constant or not. Parameters
		from project files will always be marked as constant while result quantities from
		models will be treated as non-constant.*/
	bool									m_constant = false;

	// Attributes for vector valued quantities
	/*! Vector size, = 1 for scalar quantities.*/
	unsigned int							m_size = 1;
	/*! Type of index (id or index) to be used to identify vector elements. */
	VectorValuedQuantityIndex::IndexKeyType	m_indexKeyType = VectorValuedQuantityIndex::NUM_IndexKeyType;
	/*! Set of indices for each vector element (size is m_size).*/
	std::vector<unsigned int>				m_indexKeys;
};


} // namespace NANDRAD_MODEL

#endif // NM_QuantityDescriptionH
