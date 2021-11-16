#ifndef VICUS_NETWORKSOILMODEL_H
#define VICUS_NETWORKSOILMODEL_H

#include "VICUS_CodeGenMacros.h"

#include <IBK_Parameter.h>


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
		P_MaxTempChangeIndicator,	// Keyword: MaxTempChangeIndicator		[---]		'MaxTempChangeIndicator'
		NUM_P
	};

	NetworkBuriedPipeProperties();

	SoilType					m_soilType = NUM_ST;				// XML:E

	IBK::Parameter				m_para[NUM_P];						// XML:E

};


} // namespace VICUS

#endif // VICUS_NETWORKSOILMODEL_H
