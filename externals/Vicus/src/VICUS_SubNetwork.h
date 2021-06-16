#ifndef SUBNETWORK_H
#define SUBNETWORK_H

#include "VICUS_Constants.h"
#include "VICUS_NetworkElement.h"
#include "VICUS_CodeGenMacros.h"

namespace VICUS {

class SubNetwork
{
public:
	SubNetwork();

private:

	IDType							m_id = INVALID_ID;						// XML:A:required

	std::vector<NetworkElement>		m_elements;								// XML:E


};



} // Namespace VICUS


#endif // SUBNETWORK_H
