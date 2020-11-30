#ifndef VICUS_WindowGlazingSystemH
#define VICUS_WindowGlazingSystemH

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"
#include "VICUS_WindowGlazingLayer.h"

#include <QString>
#include <QColor>
#include <vector>

#include <IBK_Parameter.h>

#include <NANDRAD_LinearSplineParameter.h>

namespace VICUS {

class WindowGlazingSystem {
public:

	/*! Model types supported by the window model. */
	enum modelType_t {
		MT_Simple,						// Keyword: Simple								'Standard globbed-layers model.'
		MT_Detailed,					// Keyword: Detailed							'Detailed window model with layers.'
		NUM_MT
	};

	/*! Model parameters. */
	enum para_t {
		P_ThermalTransmittance,			// Keyword: ThermalTransmittance		[W/m2K]		'Thermal transmittance'
		NUM_P
	};

	/*! Enum type with all possible glazing system spline parameters.*/
	enum splinePara_t {
		SP_SHGC,						// Keyword: SHGC						[---]		'Short wave transmittance at outside directed surface.'
		NUM_SP
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE
	VICUS_COMPARE_WITH_ID

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique ID-number for this glazing system (INVALID_ID = disabled/undefined). */
	unsigned int						m_id = INVALID_ID;							// XML:A:required

	/*! Some display/comment name for this model (optional). */
	QString								m_displayName;								// XML:A

	/*! False color. */
	QColor								m_color;									// XML:A

	/*! Manufacturer. */
	QString								m_notes;									// XML:E

	/*! Manufacturer. */
	QString								m_manufacturer;								// XML:E

	/*! Data source. */
	QString								m_dataSource;								// XML:E

	/*! Model type. */
	modelType_t							m_modelType = NUM_MT;						// XML:A:required

	/*! List of parameters. */
	IBK::Parameter						m_para[NUM_P];								// XML:E

	/*! Parameter for angle-dependent SHGC value. */
	NANDRAD::LinearSplineParameter		m_splinePara[NUM_SP];						// XML:E

	/*! Parameters for the detailed multi-layer model. */
	std::vector<WindowGlazingLayer>		m_layers;									// XML:E

};

} // namespace VICUS


#endif // VICUS_WindowGlazingSystemH
