/*	The NANDRAD data model library.
Copyright (c) 2012, Institut fuer Bauklimatik, TU Dresden, Germany

Written by
A. Nicolai		<andreas.nicolai -[at]- tu-dresden.de>
A. Paepcke		<anne.paepcke -[at]- tu-dresden.de>
St. Vogelsang	<stefan.vogelsang -[at]- tu-dresden.de>
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

#ifndef NANDRAD_InterfaceAirFlowH
#define NANDRAD_InterfaceAirFlowH

#include <string>

#include "NANDRAD_LinearSplineParameter.h"

#include <IBK_Parameter.h>

#include "NANDRAD_CodeGenMacros.h"

class TiXmlElement;

namespace NANDRAD {

/*!	\brief Declaration for class InterfaceAirFlow

	Model for wind flow calculation.
*/
class InterfaceAirFlow {
public:


	/*! Parameters to be defined for the various window model types. */
	enum splinePara_t {
		SP_PressureCoefficient,				// Keyword: PressureCoefficient			[---]	'Pressure coeffient.'
		NUM_SP
	};
	/*! Model types supported by the window model. */
	enum modelType_t {
		MT_WINDFLOW,						// Keyword: WindFlow							'Use results from external wind flow calculation.'
		NUM_MT
	};



	// *** PUBLIC MEMBER FUNCTIONS ***

	/*! Default constructor. */
	InterfaceAirFlow();

	/*! Reads the data from the xml element.
		Throws an IBK::Exception if a syntax error occurs.
	*/
	//void readXML(const TiXmlElement * element);

	/*! Appends the element to the parent xml element.
		Throws an IBK::Exception in case of invalid data.
	*/
	//void writeXML(TiXmlElement * parent) const;

	NANDRAD_READWRITE

	/*! Compares this instance with another by value and returns true if they differ. */
	bool operator!=(const InterfaceAirFlow & other) const;

	/*! Compares this instance with another by value and returns true if they are the same. */
	bool operator==(const InterfaceAirFlow & other) const { return ! operator!=(other); }

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Model type. */
	modelType_t							m_modelType;								// XML:E
	/*! Provided parameters for all model types, stored in a set of enums. */
	std::map<int, std::set<int> >       m_modelTypeToSplineParameterMapping;		// XML:E
	/*! List of constant parameters.*/
	NANDRAD::LinearSplineParameter		m_splinePara[NUM_SP];						// XML:E

}; // InterfaceAirFlow

} // namespace NANDRAD

#endif // NANDRAD_InterfaceAirFlowH
