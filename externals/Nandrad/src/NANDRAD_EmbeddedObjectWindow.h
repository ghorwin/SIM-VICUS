/*	The NANDRAD data model library.

	Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Anne Paepcke     <anne.paepcke -[at]- tu-dresden.de>

	This library is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This library is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

#ifndef NANDRAD_EmbeddedObjectWindowH
#define NANDRAD_EmbeddedObjectWindowH

#include <IBK_Parameter.h>

#include "NANDRAD_CodeGenMacros.h"
#include "NANDRAD_Constants.h"

#include "NANDRAD_Material.h"
#include "NANDRAD_WindowFrame.h"
#include "NANDRAD_WindowDivider.h"
#include "NANDRAD_WindowShading.h"
#include "NANDRAD_WindowGlazingSystem.h"

namespace NANDRAD {

/*!	Class EmbeddedObjectWindow defines a window and holds parameters for solar radiation
	flux calculation and heat exchange through the window.
*/
class EmbeddedObjectWindow  {
	NANDRAD_READWRITE_PRIVATE
public:

	// *** PUBLIC MEMBER FUNCTIONS ***

	NANDRAD_READWRITE_IFNOTEMPTY(EmbeddedObjectWindow)
	NANDRAD_COMP(EmbeddedObjectWindow)

	/*! Returns true, if a glazing system is defined. */
	bool hasParameters() const { return m_glazingSystemId != INVALID_ID; }

	/*! Checks for valid parameters (value ranges). Also, this function creates
		quick-access pointers to the referenced data structures, hereby testing for existence
		of referenced data types.
		\param grossArea Cross section in [m2] of embedded object. Frame and divider must
					not exceed this cross section.
	*/
	void checkParameters(double grossArea, const std::vector<Material> & materials,
						 const std::vector<WindowGlazingSystem> & glazingSystems,
						 const std::vector<ShadingControlModel> & controlModels);

	// *** PUBLIC MEMBER VARIABLES ***

	/*! ID of the glazing system used in this window.
		A glazing system with invalid ID means that this Window object is now parametrized. */
	unsigned int	m_glazingSystemId		= INVALID_ID;				// XML:A:required

	/*! Frame parameters (optional). */
	WindowFrame		m_frame;											// XML:E:tag=Frame
	/*! Divider parameters (optional). */
	WindowDivider	m_divider;											// XML:E:tag=Divider

	/*! Shading model parameters (otional). */
	WindowShading	m_shading;											// XML:E:tag=Shading

	// *** Variables used only during simulation ***

	/*! Cached gross cross-section area of surface in [m2] (embedded object gross area). */
	double									m_area = 999;
	/*! Cached cross-section area of glas surface in [m2] (embedded object gross area minus frame and divider cross section areas). */
	double									m_glasArea = 999;

	/*! Quick-access pointer to the glazing system data. */
	const NANDRAD::WindowGlazingSystem		*m_glazingSystem = nullptr;

}; // EmbeddedObjectWindow


} // namespace NANDRAD

#endif // NANDRAD_EmbeddedObjectWindowH
