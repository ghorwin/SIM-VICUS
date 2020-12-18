#include "VICUS_Material.h"


namespace VICUS {

QString Material::categoryToString(Material::Category c)
{
	switch (c) {
		case MC_Coating:				return tr("Coating");
		case MC_Plaster:				return tr("Plaster");
		case MC_Bricks:					return tr("Bricks");
		case MC_NaturalStones:			return tr("NaturalStones");
		case MC_Cementitious:			return tr("Cementitious");
		case MC_Insulations:			return tr("Insulations");
		case MC_BuildingBoards:			return tr("BuildingBoards");
		case MC_Woodbased:				return tr("Woodbased");
		case MC_NaturalMaterials:		return tr("NaturalMaterials");
		case MC_Soils:					return tr("Soils");
		case MC_CladdingSystems:		return tr("CladdingSystems");
		case MC_Foils:					return tr("Foils");
		case MC_Miscellaneous:			return tr("Miscellaneous");
		default:						return tr("Miscellaneous");
	}
}

}
