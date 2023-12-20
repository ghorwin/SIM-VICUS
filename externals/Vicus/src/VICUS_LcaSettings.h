/*	The SIM-VICUS data model library.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Dirk Weiss  <dirk.weiss -[at]- tu-dresden.de>
	  Stephan Hirth  <stephan.hirth -[at]- tu-dresden.de>
	  Hauke Hirsch  <hauke.hirsch -[at]- tu-dresden.de>

	  ... all the others from the SIM-VICUS team ... :-)

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

#ifndef VICUS_LcaSettingsH
#define VICUS_LcaSettingsH

#include <IBK_Flag.h>

#include <IBKMK_Vector3D.h>
#include <IBK_Parameter.h>

#include "VICUS_CodeGenMacros.h"
#include "VICUS_EpdModuleDataset.h"


namespace VICUS {

/*! Stores general settings about LCA Options. */
class LcaSettings {
	VICUS_READWRITE_PRIVATE
public:

	// *** PUBLIC MEMBER FUNCTIONS ***

	enum para_t {
		// FIXME: TimePeriod must be an integer and moved to intPara_t
		P_TimePeriod,					// Keyword: TimePeriod					[a]		'Time period for consideration [a].'
		P_FactorBnbSimpleMode,			// Keyword: FactorSimpleMode			[-]		'Calculation factor for simple mode calculation (BNB). [-]'
		P_NetUsageArea,					// Keyword: NetUsageArea				[m2]	'Net usage area [m2].'
		NUM_P
	};

	//the most frequently used categories
	enum Module {
		M_A1 = 0x00001,		// Keyword: A1					'Raw material procurement (A1)'
		M_A2 = 0x00002,		// Keyword: A2					'Transport (A2)'
		M_A3 = 0x00004,		// Keyword: A3					'Production (A3)'
		M_A4 = 0x00008,		// Keyword: A4					'Transport (A4)'
		M_A5 = 0x00010,		// Keyword: A5					'Construction / Installation (A5)'
		M_B1 = 0x00020,		// Keyword: B1					'Usage (B1)'
		M_B2 = 0x00040,		// Keyword: B2					'Maintenance (B2)'
		M_B3 = 0x00080,		// Keyword: B3					'Maintenance (B3)'
		M_B4 = 0x00100,		// Keyword: B4					'Replacement (B4)'
		M_B5 = 0x00200,		// Keyword: B5					'Modernisation (B5)'
		M_B6 = 0x00400,		// Keyword: B6					'Energy consumption in operation (B6)'
		M_B7 = 0x00800,		// Keyword: B7					'Water consumption in operation (B7)'
		M_C1 = 0x01000,		// Keyword: C1					'deconstruction / demolition (C1)'
		M_C2 = 0x02000,		// Keyword: C2					'Transport (C2)'
		M_C3 = 0x04000,		// Keyword: C3					'Waste recycling (C3)'
		M_C4 = 0x08000,		// Keyword: C4					'Disposal (C4)'
		M_D = 0x10000,		// Keyword: D					'Potential for reuse, recovery and recycling (D)'
		NUM_M = 17
	};

	enum LcaCategory {
		L_CategoryA,		// Keyword: A
		L_CategoryB,		// Keyword: B
		L_CategoryC,		// Keyword: C
		L_CategoryD,		// Keyword: D
		NUM_L
	};

	enum UsageType {
		UT_Gas,				// Keyword: Gas
		UT_Electricity,		// Keyword: Electricity
		UT_Coal,			// Keyword: Coal
		NUM_UT
	};

	/*! LCA Caluclation modes. */
	enum CalculationMode {
		CM_Simple,							// Keyword: Simple					'Use predefined certification system settings.'
		CM_Detailed,						// Keyword: Detailed				'Set detailed LCA Settings.'
		NUM_CM
	};

	/*! Certificytion systems predefined modules. */
	enum CertificationSytem {
		CS_BNB,							// Keyword: BNB				'Bewertungssystem Nachhaltiges Bauen (BNB)'
		NUM_CS
	};

	/*! Certificytion systems predefined modules. */
	enum CertificationModules {
		CT_BNB = LcaSettings::M_A1 | LcaSettings::M_A2 | LcaSettings::M_A3 | LcaSettings::M_B6 | LcaSettings::M_C3 | LcaSettings::M_C4
	};

	VICUS_READWRITE_IFNOTEMPTY(LcaSettings)
	VICUS_COMP(LcaSettings)

	/*! Constructor. */
	LcaSettings() {
		initDefaults();
	}


	/*! Init default values. */
	void initDefaults();

	/*! Returns whether a Category is defined in LCA Settings. */
	bool isLcaCategoryDefined(EpdModuleDataset::Module mod) const;
	static bool isLcaCategoryDefined(EpdModuleDataset::Module mod, CertificationModules modules);

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Time Period for LCA/LCC consideration. */
	IBK::Parameter						m_para[NUM_P];								// XML:E

	/*! Set flags for LCA Calculations. */
	IBK::Flag							m_flags[NUM_M];								// XML:E

	/*! Calculation mode of LCA. */
	CalculationMode						m_calculationMode;							// XML:E

	/*! Certification system to be used for LCA. */
	CertificationSytem					m_certificationSystem;						// XML:E

	/*! Filter for checking, which modules are selected by certification system. */
	CertificationModules				m_certificationModules;

	/*! IDs of EPDs for Usage. */
	IDType								m_idUsage[NUM_UT];							// XML:E

};

inline bool LcaSettings::operator!=(const LcaSettings & other) const {
	for(unsigned int i=0; i<NUM_P; ++i)
		if (m_para[i] != other.m_para[i]) return true;

	for(unsigned int i=0; i<NUM_M; ++i)
		if (m_flags[i].isEnabled() != other.m_flags[i].isEnabled()) return true;

	for(unsigned int i=0; i<NUM_UT; ++i)
		if (m_idUsage[i] != other.m_idUsage[i]) return true;

	if (m_calculationMode != other.m_calculationMode) return true;
	if (m_certificationSystem != other.m_certificationSystem) return true;

	return false;
}


} // namespace VICUS


#endif // VICUS_LcaSettingsH
