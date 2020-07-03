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

#ifndef NANDRAD_EmbeddedObjectWindowH
#define NANDRAD_EmbeddedObjectWindowH

#include <string>

#include <IBK_Parameter.h>

class TiXmlElement;

namespace NANDRAD {

/*!	\brief Declaration for class EmbeddedObjectWindow

	An embedded object generally defines a wall opening (a window or a door).
	That means, the calculation radiant heat fluxes and heat fluxes by heat transmission are performed
	by a window model or door model. The embedded oject	stores an exchangable parameter model
	that defines the name for the calculation model and constant model parameters.
*/
class EmbeddedObjectWindow  {
public:

	// ***KEYWORDLIST-START***
	/*! Parameters to be defined for the various window model types. */
	enum para_t {
		P_GlassFraction,			// Keyword: GlassFraction				[---]		{1}		'Fraction of glass area versus total area [1 - no frame, 0 - all frame].'
		P_SolarHeatGainCoefficient,	// Keyword: SolarHeatGainCoefficient	[---]		{0.8}	'Constant transmissibility [0 - opaque, 1 - fully translucent].'
		P_ThermalTransmittance,		// Keyword: ThermalTransmittance		[W/m2K]		{0.8}	'Effective heat transfer coefficient (area-weighted U-value of frame and glass).'
		P_ShadingFactor,			// Keyword: ShadingFactor				[---]		{1}		'Shading factor (between 0...1).'
		P_LeakageCoefficient,		// Keyword: LeakageCoefficient			[---]		{1}		'Leakage coefficient of joints [m3/sPa^n].'
		NUM_P
	};

	/*! Model types supported by the window model. */
	enum modelType_t {
		MT_CONSTANT,				// Keyword: Constant				'Constant model.'
		MT_DETAILED,				// Keyword: Detailed				'Model with detailed layers for calculation of long wave radiation, short wave radiation and gas convection transport.'
		MT_DETAILED_WITH_STORAGE,	// Keyword: DetailedWithStorage		'Model with detailed layers and thermal storage of glass layers.'
		NUM_MT
	};
	// ***KEYWORDLIST-END***


	// *** PUBLIC MEMBER FUNCTIONS ***

	/*! Default constructor. */
	EmbeddedObjectWindow();

	/*! Reads the data from the xml element.
		Throws an IBK::Exception if a syntax error occurs.
	*/
	void readXML(const TiXmlElement * element);

	/*! Appends the element to the parent xml element.
		Throws an IBK::Exception in case of invalid data.
	*/
	void writeXML(TiXmlElement * parent, bool detailedOutput) const;


	// *** PUBLIC MEMBER VARIABLES ***

	/*! Model type. */
	modelType_t							m_modelType;
	/*! Provided parameters for all model types, stored in a set of enums. */
	std::map<int, std::set<int> >		m_modelTypeToParameterMapping;

	/*! List of constant parameters.*/
	IBK::Parameter						m_para[NUM_P];
	/*! Path to a window type file for detailed calculation. Empty for model type 'Constant'*/
	std::string							m_windowTypeReference;

}; // EmbeddedObjectWindow

} // namespace NANDRAD

#endif // NANDRAD_EmbeddedObjectWindowH
