#ifndef VICUS_MaterialH
#define VICUS_MaterialH

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"

#include <QString>
#include <QColor>

#include <vector>

#include <IBK_Parameter.h>

#include <NANDRAD_LinearSplineParameter.h>

namespace VICUS {

class Material {
public:

	/*! Basic parameters. */
	enum para_t {
		//thermal paramters
		/*! Dry density of the material. */
		P_Density,					// Keyword: Density					[kg/m3]	'Dry density of the material.'
		/*! Specific heat capacity of the material. */
		P_HeatCapacity,				// Keyword: HeatCapacity			[J/kgK]	'Specific heat capacity of the material.'
		/*! Thermal conductivity of the dry material. */
		P_Conductivity,				// Keyword: Conductivity			[W/mK]	'Thermal conductivity of the dry material.'

		//hygrothermal parameters
		/*! Thermal conductivity of the dry material. */
		P_Mu,						// Keyword: Mu						[-]		'Vapor diffusion resistance factor.'
		/// TODO Beschreibung einfügen
		P_W80,						// Keyword: W80						[kg/m3]	'...'
		/// TODO Beschreibung einfügen
		P_Wsat,						// Keyword: Wsat					[kg/m3]	'...'

		//addition glass properties
		/*! Solar transmittance. */
		P_SolarTransmittance,		// Keyword: SolarTransmittance		[---]	'Solar transmittance for glass panes.'
		/*! Solare reflectance front. */
		P_SolarReflectanceFront,	// Keyword: SolarReflectanceFront	[---]	'Solar reflectance for glass panes front side.'
		/*! Solare reflectance back. */
		P_SolarReflectanceBack,		// Keyword: SolarReflectanceBack	[---]	'Solar reflectance for glass panes back side.'
		/*! Visible transmittance. */
		P_VisibleTransmittance,		// Keyword: VisibleTransmittance	[---]	'Visible transmittance for glass panes.'
		/*! Visible reflectance front. */
		P_VisibleReflectanceFront,	// Keyword: VisibleReflectanceFront	[---]	'Visible reflectance for glass panes front side.'
		/*! Visible reflectance back. */
		P_VisibleReflectanceBack,	// Keyword: VisibleReflectanceBack	[---]	'Visible reflectance for glass panes back side.'
		/*! Infrared transmittance. */
		P_InfraredTransmittance,	// Keyword: InfraredTransmittance	[---]	'Infrared transmittance for glass panes.'
		/*! Emittance front. */
		P_EmittanceFront,			// Keyword: EmittanceFront			[---]	'Emittance for glass panes front side.'
		/*! Emittance back. */
		P_EmittanceBack,			// Keyword: EmittanceBack			[---]	'Emittance for glass panes back side.'
		NUM_P
	};


	/// TODO Andreas : Dürfen in verschiedenen Enums die gleichen Keywerte enthalten sein?
	/*! Fluid temperature dependent parameters. */
	enum splinePara_t {
		/*! Dry density of the material. */
		LP_Density,					// Keyword: Density					[kg/m3]	'Dry density of the material (temperature dependent).'
		/*! Specific heat capacity of the material. */
		LP_HeatCapacity,			// Keyword: HeatCapacity			[J/kgK]	'Specific heat capacity of the material (temperature dependent).'
		/*! Thermal conductivity of the dry material. */
		LP_Conductivity,			// Keyword: Conductivity			[W/mK]	'Thermal conductivity of the dry material (temperature dependent).'
		NUM_LP
	};

	/*! Material categories.*/
	enum category_t{
		//opaque categories
		MC_Coating,					// Keyword: Coating
		MC_Plaster,					// Keyword: Plaster
		MC_Bricks,					// Keyword: Bricks
		MC_NaturalStones,			// Keyword: NaturalStones
		MC_Cementitious,			// Keyword: Cementitious
		MC_Insulations,				// Keyword: Insulations
		MC_BuildingBoards,			// Keyword: BuildingBoards
		MC_Woodbased,				// Keyword: Woodbased
		MC_NaturalMaterials,		// Keyword: NaturalMaterials
		MC_Soils,					// Keyword: Soils
		MC_CladdingSystems,			// Keyword: CladdingSystems
		MC_Foils,					// Keyword: Foils
		MC_Miscellaneous,			// Keyword: Miscellaneous
		//Glass
		MC_GlassPane,				// Keyword: GlasPane
		//Gas
		MC_Gas,						// Keyword: Gas
		Num_MC
	};


	/*! Type of the material. */
	enum modelType_t{
		MT_Opaque,					// Keyword: Opaque					' Opaque material type. '
		MT_Glass,					// Keyword: GlassPane				' Glass material type for transparent constructions. '
		MT_Gas,						// Keyword: Gas						' Gas material type for transparent constructions. '
		NUM_MT
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE
	VICUS_COMPARE_WITH_ID

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique ID of material. */
	unsigned int					m_id = INVALID_ID;						// XML:A:required

	/*! Display name of material. */
	QString							m_displayName;							// XML:A

	/*! False color. */
	QColor							m_color;								// XML:A

	/*! Manufacturer. */
	QString							m_manufacturer;							// XML:E

	/*! Data source. */
	QString							m_dataSource;							// XML:E

	/*! Material category. */
	category_t						m_category = Num_MC;					// XML:E:required

	/*! Material type. */
	modelType_t						m_modeltType = NUM_MT;					// XML:E:required

	/*! List of parameters. */
	IBK::Parameter					m_para[NUM_P];							// XML:E

	/*! List of temperature dependent parameters. Only for fluids. */
	NANDRAD::LinearSplineParameter	m_paraTemperatureDependent[NUM_LP];		// XML:E

};

} // namespace VICUS


#endif // VICUS_MaterialH
