#ifndef VICUS_WindowGlazingLayerH
#define VICUS_WindowGlazingLayerH

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"
#include "VICUS_AbstractDBElement.h"

#include <QString>
#include <vector>

#include <IBK_Parameter.h>

#include <NANDRAD_LinearSplineParameter.h>

namespace VICUS {

/*! Window glazing layer is a child of WindowGlazingSystem, but not a stand-alone
	database element.
*/
class WindowGlazingLayer {
public:

	/*! Model types supported by the window model. */
	enum type_t {
		T_Gas,					// Keyword: Gas						'Gas layer'
		T_Glass,				// Keyword: Glass					'Glass layer'
		NUM_T
	};

	/*! Basic parameters. */
	enum para_t {
		/*! Dry density of the material. */
		P_Thickness,				// Keyword: Thickness					[m]			'Thickness of the window layer.'
		/*! Thermal conductivity of the dry material. */
		P_Conductivity,				// Keyword: Conductivity				[W/mK]		'Thermal conductivity of the window layer.'
		/*! Mass density of gas layer. */
		P_MassDensity,				// Keyword: MassDensity					[kg/m3]		'Mass density of the fill-in gas.'
		/*! Height of detailed window (needed for Convection in Cavity).  */
		P_Height,					// Keyword: Height						[m]			'Height of the detailed window.'
		/*! Width of detailed window (needed for Convection in Cavity).  */
		P_Width,					// Keyword: Width						[m]			'Width of the detailed window.'
		/*! Emissivity of surface facing outside. */
		P_LongWaveEmissivityInside,	// Keyword: LongWaveEmissivityInside	[---]		'Emissivity of surface facing outside.'
		/*! Emissivity of surface facing inside. */
		P_LongWaveEmissivityOutside,// Keyword: LongWaveEmissivityOutside [---]		'Emissivity of surface facing inside.'

		NUM_P
	};


	/*! Enum type with all possible layer spline parameters.*/
	enum splinePara_t {
		SP_ShortWaveTransmittance,		// Keyword: ShortWaveTransmittance		[---]		'Short wave transmittance at outside directed surface.'
		SP_ShortWaveReflectanceOutside,	// Keyword: ShortWaveReflectanceOutside	[---]		'Short wave reflectance of surface facing outside.'
		SP_ShortWaveReflectanceInside,	// Keyword: ShortWaveReflectanceInside	[---]		'Short wave reflectance of surface facing inside.'
		SP_Conductivity,				// Keyword: Conductivity				[W/mK]		'Thermal conductivity of the gas layer.'
		SP_DynamicViscosity,			// Keyword: DynamicViscosity			[kg/ms]		'Dynamic viscosity of the gas layer.'
		SP_HeatCapacity,				// Keyword: HeatCapacity				[J/kgK]		'Specific heat capacity of the gas layer.'
		NUM_SP
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE

	// *** PUBLIC MEMBER VARIABLES ***

	type_t							m_type = NUM_T;						// XML:A:required

	/*! Unique ID-number for this window layer. */
	unsigned int					m_id = INVALID_ID;					// XML:A:required

	/*! Display name of layer. */
	QString							m_displayName;						// XML:A

	/*! Data source. */
	QString							m_dataSource;						// XML:E

	/*! Basic parameters of the window layer  */
	IBK::Parameter					m_para[NUM_P];						// XML:E

	// Layer Data in LinearSpline

	NANDRAD::LinearSplineParameter	m_splinePara[NUM_SP];				// XML:E

};

} // namespace VICUS


#endif // VICUS_WindowGlazingLayerH
