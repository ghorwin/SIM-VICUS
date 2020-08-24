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
	enum model_t {
		/*! Ventilation rate is given as constant parameter. */
		M_Constant,			// Keyword: Constant	'Constant ventilation rate'
		/*! Ventilation rate is provided as 'NaturalVentilationRate' schedule parameter. */
		M_Scheduled,		// Keyword: Scheduled	'Scheduled ventilation rate'
		NUM_M
	};

	enum para_t {
		P_VentilationRate,	// Keyword: VentilationRate		[1/h]		'Ventilation rate'
		NUM_P
	};

	/*! Unique ID-number for this ventilation rate model. */
	unsigned int						m_id = NANDRAD::INVALID_ID;		// XML:A:required

	/*! Model type. */
	model_t								m_model;						// XML:A:required

	/*! Object list with zones that this model is to be apply to. */
	std::string							m_zoneObjectList;				// XML:E:required

	/*! Model parameters. */
	IBK::Parameter						m_para[NUM_P];					// XML:E
};

} // namespace NANDRAD

#endif // NANDRAD_NaturalVentilationModelH
