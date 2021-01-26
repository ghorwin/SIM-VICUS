#include "NANDRAD_SolarLoadsDistributionModel.h"

#include "NANDRAD_KeywordList.h"

namespace NANDRAD {


void SolarLoadsDistributionModel::initDefaults() {
	m_para[P_RadiationLoadFractionZone].set( KeywordList::Keyword("SolarLoadsDistributionModel::para_t", P_RadiationLoadFractionZone),	 0.5, IBK::Unit("---"));
	// Mind: the next three must add up to 1
	m_para[P_RadiationLoadFractionFloor].set( KeywordList::Keyword("SolarLoadsDistributionModel::para_t", P_RadiationLoadFractionFloor),	 0.5, IBK::Unit("---"));
	m_para[P_RadiationLoadFractionWalls].set( KeywordList::Keyword("SolarLoadsDistributionModel::para_t", P_RadiationLoadFractionWalls),	 0.3, IBK::Unit("---"));
	m_para[P_RadiationLoadFractionCeiling].set( KeywordList::Keyword("SolarLoadsDistributionModel::para_t", P_RadiationLoadFractionCeiling),	 0.2, IBK::Unit("---"));
}


void SolarLoadsDistributionModel::checkParameters() const {
	FUNCID(SolarLoadsDistributionModel::checkParameters);

	// check radiation load fractions
	for (unsigned int i=0; i<4; ++i)
		m_para[P_RadiationLoadFractionZone+i].checkedValue(
					NANDRAD::KeywordList::Keyword("SolarLoadsDistributionModel::para_t", P_RadiationLoadFractionZone+i),
					"---", "---", 0, true, 1, true,
					"Radiation load fraction must be between 0 and 1.");

	// check that the latter three add up to 1
	double fractionSum = m_para[P_RadiationLoadFractionFloor].value +
			m_para[P_RadiationLoadFractionWalls].value +
			m_para[P_RadiationLoadFractionCeiling].value;

	if (!IBK::near_equal(fractionSum, 1.0))
		throw IBK::Exception("Radiation load fractions for walls, ceiling and floor must add up to 1.", FUNC_ID);
}


bool SolarLoadsDistributionModel::operator!=(const SolarLoadsDistributionModel & other) const {
	for (unsigned int i=0; i<NUM_P; ++i)
		if (m_para[i] != other.m_para[i]) return true;

	if (m_distributionType != other.m_distributionType)
		return true;
	return false; // this and other hold the same data
}


} // namespace NANDRAD
