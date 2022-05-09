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

#ifndef VICUS_SurfaceHeatingH
#define VICUS_SurfaceHeatingH

#include <QColor>

#include <IBK_Parameter.h>

#include <NANDRAD_DataTable.h>

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"
#include "VICUS_AbstractDBElement.h"
#include "VICUS_Database.h"
#include "VICUS_NetworkPipe.h"

namespace VICUS {

/*! SurfaceHeating model parameters. */
class SurfaceHeating : public AbstractDBElement {
public:

	/*! Basic parameters. */
	enum para_t {
		/*! Heating limit. */
		P_HeatingLimit,							// Keyword: HeatingLimit				[W/m2]		'Heating limit'
		/*! Cooling limit. */
		P_CoolingLimit,							// Keyword: CoolingLimit				[W/m2]		'Cooling limit'
		/*! Pipe spacing. */
		P_PipeSpacing,							// Keyword: PipeSpacing					[m]			'Pipe spacing'
		/*! Maximum fluid velocity. */
		P_MaxFluidVelocity,						// Keyword: MaxFluidVelocity			[m/s]		'Maximum fluid velocity'
		/*! Temperature difference between supply and return fluid temperature. */
		P_TemperatureDifferenceSupplyReturn,	// Keyword: TemperatureDifferenceSupplyReturn	[K]		'Temperature difference between supply and return fluid temperature'

		NUM_P
	};


	/*! Surface Heating Types. */
	enum Type {
		T_Ideal,			// Keyword: Ideal							'Ideal surface conditioning'
		T_PipeRegister,		// Keyword: PipeRegister					'Water-based surface conditioning: either ideal or part of a hydraulic network'
		NUM_T
	};


	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE_OVERRIDE
	VICUS_COMPARE_WITH_ID

	/*! Checks if all parameters are valid. */
	bool isValid(const Database<NetworkPipe> & pipeDB) const;

	/*! Comparison operator */
	ComparisonResult equal(const AbstractDBElement *other) const override;

	// *** PUBLIC MEMBER VARIABLES ***

	//:inherited	unsigned int					m_id = INVALID_ID;		// XML:A:required
	//:inherited	IBK::MultiLanguageString		m_displayName;			// XML:A
	//:inherited	QColor							m_color;				// XML:A

	/*! Surface heating type. */
	Type							m_type = T_Ideal;						// XML:A:required

	/*! Notes. */
	IBK::MultiLanguageString		m_notes;								// XML:E

	/*! Heating/cooling curve points. */
	NANDRAD::DataTable				m_heatingCoolingCurvePoints;			// XML:E

	/*! Pipe id. */
	unsigned int					m_idPipe;								// XML:E

	/*! List of parameters. */
	IBK::Parameter					m_para[NUM_P];							// XML:E
};

}
#endif // VICUS_SurfaceHeatingH
