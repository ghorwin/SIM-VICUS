#include "SOLFRA_IntegratorErrorControlled.h"

namespace SOLFRA {

IntegratorErrorControlled::IntegratorErrorControlled() :
	m_relTol(1e-5),
	m_absTol(1e-6),
	m_stopTime(0)
{
}


} // namespace SOLFRA

