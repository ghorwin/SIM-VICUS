#include "SVMaterialTransfer.h"

SVMaterialTransfer::SVMaterialTransfer(const VICUS::Material& mat):
	QtExt::MaterialBase(static_cast<int>(mat.m_id)),
	m_mat(mat)
{}

QString SVMaterialTransfer::name() const
{
	return QString::fromStdString(m_mat.m_displayName.string());
}

QtExt::MaterialCategory SVMaterialTransfer::category() const
{
	switch(m_mat.m_category){
		case VICUS::Material::MC_Soils:				return QtExt::MaterialCategory::SOIL;
		case VICUS::Material::MC_Foils:				return QtExt::MaterialCategory::FOIL;
		case VICUS::Material::MC_Bricks:			return QtExt::MaterialCategory::BRICK;
		case VICUS::Material::MC_Coating:			return QtExt::MaterialCategory::COATING;
		case VICUS::Material::MC_Plaster:			return QtExt::MaterialCategory::PLASTER;
		case VICUS::Material::MC_Woodbased:			return QtExt::MaterialCategory::TIMBER;
		case VICUS::Material::MC_Insulations:		return QtExt::MaterialCategory::INSULATION;
		case VICUS::Material::MC_Cementitious:		return QtExt::MaterialCategory::CONCRETE;
		case VICUS::Material::MC_Miscellaneous:		return QtExt::MaterialCategory::MISCELLANEOUS;
		case VICUS::Material::MC_NaturalStones:		return QtExt::MaterialCategory::NATURAL_STONES;
		case VICUS::Material::MC_NaturalMaterials:	return QtExt::MaterialCategory::NATURAL_MATERIALS;
		case VICUS::Material::MC_BuildingBoards:	return QtExt::MaterialCategory::BUILDING_BOARD;
		case VICUS::Material::MC_CladdingSystems:	return QtExt::MaterialCategory::CLADDING;
		case VICUS::Material::NUM_MC:				return QtExt::MaterialCategory::NUM_CATEGORIES;
		default:									return QtExt::MaterialCategory::NUM_CATEGORIES;
	}
}
