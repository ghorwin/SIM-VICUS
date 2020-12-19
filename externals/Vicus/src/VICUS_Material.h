#ifndef VICUS_MaterialH
#define VICUS_MaterialH

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"
#include "VICUS_AbstractDBElement.h"

#include <QString>
#include <QColor>
#include <QCoreApplication>

#include <IBK_Parameter.h>

#include <NANDRAD_LinearSplineParameter.h>

namespace VICUS {

class Material : public AbstractDBElement {
	Q_DECLARE_TR_FUNCTIONS(Material)
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
		P_W80,						// Keyword: W80						[kg/m3]	'Water content in relation to 80% humidity.'
		/// TODO Beschreibung einfügen
		P_Wsat,						// Keyword: Wsat					[kg/m3]	'Water content at saturation.'

		NUM_P
	};

	/*! Material categories.*/
	enum Category{
		//opaque categories
		MC_Coating,					// Keyword: Coating
		MC_Plaster,					// Keyword: Plaster
		MC_Bricks,					// Keyword: Bricks
		MC_NaturalStones,			// Keyword: NaturalStones
		MC_Cementitious,				// Keyword: Cementitious
		MC_Insulations,				// Keyword: Insulations
		MC_BuildingBoards,			// Keyword: BuildingBoards
		MC_Woodbased,					// Keyword: Woodbased
		MC_NaturalMaterials,		// Keyword: NaturalMaterials
		MC_Soils,						// Keyword: Soils
		MC_CladdingSystems,			// Keyword: CladdingSystems
		MC_Foils,						// Keyword: Foils
		MC_Miscellaneous,			// Keyword: Miscellaneous
		NUM_MC
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	Material(){}


	/*! Constructor.
		\param	id
		\param	name display name
		\param conductivity in W/mK
		\param density in kg/m3
		\param specHeatCapa in J/kgK
	*/
	Material( unsigned int id, const QString &name, double conductivity, double density, double specHeatCapa):
		m_id(id),
		m_displayName(name.toStdString())
	{
		m_para[P_Density] = (IBK::Parameter("Density", density, "kg/m3"));
		m_para[P_Conductivity] = (IBK::Parameter("Conductivity", conductivity, "W/mK"));
		m_para[P_HeatCapacity] = (IBK::Parameter("HeatCapacity", specHeatCapa, "J/kgK"));
	}

	VICUS_READWRITE
	VICUS_COMPARE_WITH_ID

	static QString categoryToString(Category c);

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique ID of material. */
	unsigned int					m_id = INVALID_ID;						// XML:A:required

	/*! Display name of material. */
	IBK::MultiLanguageString		m_displayName;							// XML:A

	/*! False color. */
	QColor							m_color;								// XML:A

	/*! Notes. */
	IBK::MultiLanguageString		m_notes;								// XML:E

	/*! Manufacturer. */
	IBK::MultiLanguageString		m_manufacturer;							// XML:E

	/*! Data source. */
	IBK::MultiLanguageString		m_dataSource;							// XML:E

	/*! Material category. */
	Category						m_category = NUM_MC;					// XML:E:required

	/*! List of parameters. */
	IBK::Parameter					m_para[NUM_P];							// XML:E

	/*! Vector of different ids of epd sub elements. */
	std::vector<unsigned int>		m_idEpds;								// XML:E

};

} // namespace VICUS


#endif // VICUS_MaterialH
