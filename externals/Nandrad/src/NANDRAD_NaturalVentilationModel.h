#ifndef NANDRAD_NaturalVentilationModelH
#define NANDRAD_NaturalVentilationModelH

#include <IBK_Parameter.h>

#include "NANDRAD_Constants.h"
#include "NANDRAD_CodeGenMacros.h"

namespace NANDRAD {

/*! Contains all data for natural ventilation models. */
class NaturalVentilationModel {
public:
	/*! Different model variants. */
	enum modelType_t {
		/*! Ventilation rate is given as constant parameter. */
		MT_Constant,			// Keyword: Constant	'Constant ventilation rate'
		/*! Ventilation rate is provided as 'NaturalVentilationRate' schedule parameter. */
		MT_Scheduled,		// Keyword: Scheduled	'Scheduled ventilation rate'
		NUM_MT
	};

	enum para_t {
		P_VentilationRate,	// Keyword: VentilationRate		[1/h]		'Ventilation rate'
		NUM_P
	};

	NANDRAD_READWRITE

	/*! Unique ID-number for this ventilation rate model. */
	unsigned int						m_id = NANDRAD::INVALID_ID;		// XML:A:required
	/*! Some display/comment name for this model (optional). */
	std::string							m_displayName;					// XML:A

	/*! Model type. */
	modelType_t							m_modelType = NUM_MT;			// XML:A:required

	/*! Object list with zones that this model is to be apply to. */
	std::string							m_zoneObjectList;				// XML:E:required

	/*! Model parameters. */
	IBK::Parameter						m_para[NUM_P];					// XML:E
};

} // namespace NANDRAD

#endif // NANDRAD_NaturalVentilationModelH
