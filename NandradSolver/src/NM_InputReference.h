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

#ifndef NM_InputReferenceH
#define NM_InputReferenceH

#include <string>
#include <IBK_Unit.h>

#include "NM_QuantityName.h"

#include <NANDRAD_ModelInputReference.h>

namespace NANDRAD_MODEL {

/*! This class substitutes the NANDRAD::ModelInputReference inside all
	AbstractStateDependencies.
	Contrarily to the NANDRAD::ModelInputReference structure it 
	references just one single id.
	Generate a vector of InputReferences from every single
	ModelInputReference inside the initInputReferences() function as soon
	as all encoded model instances are common.
*/

class InputReference {
public:
	/*! Default constructor. */
	InputReference() :
		m_referenceType(NANDRAD::ModelInputReference::NUM_MRT),
		m_id(0),
		m_constant(false)
	{}

	/*! Target name: name of the input quantity inside the dependend
		model*/
	QuantityName									m_targetName;
	/*! ReferenceType: zone, model or constructionInstance */
	NANDRAD::ModelInputReference::referenceType_t	m_referenceType;
	/*! Name of the result quantity inside the referenced model.*/
	QuantityName									m_sourceName;
	/*! ID of the referenced source model/parametrization entity.*/
	unsigned int									m_id;
	/*! True, if referenced quantity is to be treated constant during
		update cycles (essentially prevents adding an evaluation dependency).
	*/
	bool											m_constant;
};

} // namespace NANDRAD_MODEL

#endif // NM_InputReferenceH
