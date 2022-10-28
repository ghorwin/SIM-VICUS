#include "NANDRAD_HydraulicNetworkNode.h"

namespace NANDRAD {

HydraulicNetworkNode::HydraulicNetworkNode()
{

}

HydraulicNetworkNode::HydraulicNetworkNode(unsigned int id, const double & height):
	m_id(id),
	m_height(height)
{
}

} // namespace NANDRAD
