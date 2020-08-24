#ifndef NANDRAD_ModelsH
#define NANDRAD_ModelsH

#include <vector>

#include "NANDRAD_NaturalVentilationModel.h"

namespace NANDRAD {

/*! A container class for all models. */
class Models {
public:
	NANDRAD_READWRITE

	/*! Container for all natural ventilation models. */
	std::vector<NaturalVentilationModel>	m_naturalVentilationModels;
};

} // namespace NANDRAD

#endif // NANDRAD_ModelsH
