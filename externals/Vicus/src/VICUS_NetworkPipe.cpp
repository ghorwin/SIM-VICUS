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
