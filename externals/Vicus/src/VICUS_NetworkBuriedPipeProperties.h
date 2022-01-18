#ifndef VICUS_NETWORKSOILMODEL_H
#define VICUS_NETWORKSOILMODEL_H

#include "VICUS_CodeGenMacros.h"

#include <IBK_Parameter.h>
#include <IBK_IntPara.h>


namespace VICUS {


class NetworkBuriedPipeProperties {
public:

	VICUS_READWRITE

	/*! Type of soil */
	enum SoilType {
		ST_Sand,		// Keyword: Sand
		ST_Loam,		// Keyword: Loam
		ST_Silt,		// Keyword: Silt
		NUM_ST
	};

	/*! parameters of buried pipes */
	enum para_t {
		P_PipeSpacing,				// Keyword: PipeSpacing		[m]		'Spacing between supply and return pipes'
		P_PipeDepth,				// Keyword: PipeDepth		[m]		'Distance between soil surface and pipes'
		NUM_P
	};

	NetworkBuriedPipeProperties();

	/*! Soil type */
	SoilType					m_soilType = NUM_ST;				// XML:E

	/*! Parameters of pipes in the ground */
	IBK::Parameter				m_para[NUM_P];						// XML:E

	/*! Number of different soil models used for FMI coupling. If =0, one soil model is assigned to each single pipe */
	unsigned int				m_numberOfSoilModels = 10;				// XML:E
};


} // namespace VICUS

#endif // VICUS_NETWORKSOILMODEL_H
