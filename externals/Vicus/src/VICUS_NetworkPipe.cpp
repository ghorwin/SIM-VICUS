#include "VICUS_NetworkPipe.h"


bool VICUS::NetworkPipe::isValid() const
{
	if (m_id == INVALID_ID)
		return false;
	if (m_diameterOutside <= 0 || m_wallThickness <= 0 || m_roughness <= 0 || m_lambdaWall <= 0)
		return false;
	if (m_insulationThickness < 0 || m_lambdaInsulation < 0)
		return false;
	return true;
}

VICUS::AbstractDBElement::ComparisonResult VICUS::NetworkPipe::equal(const VICUS::AbstractDBElement *other) const {
	const NetworkPipe * otherNetPipe = dynamic_cast<const NetworkPipe*>(other);
	if (otherNetPipe == nullptr)
		return Different;

	//first check critical data

	//check parameters
//	for(unsigned int i=0; i<NUM_P; ++i){
//		if(m_para[i] != otherEPD->m_para[i])
//			return Different;
//	}

	if(m_diameterOutside != otherNetPipe->m_diameterOutside ||
			m_wallThickness != otherNetPipe->m_wallThickness||
			m_lambdaWall != otherNetPipe->m_lambdaWall ||
			m_roughness != otherNetPipe->m_roughness ||
			m_insulationThickness != otherNetPipe->m_insulationThickness ||
			m_lambdaInsulation != otherNetPipe->m_lambdaInsulation)
		return Different;

	//check meta data

	if(m_displayName != otherNetPipe->m_displayName ||
			m_color != otherNetPipe->m_color)
		return OnlyMetaDataDiffers;

	return Equal;
}
