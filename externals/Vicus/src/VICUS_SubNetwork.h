#ifndef SUBNETWORK_H
#define SUBNETWORK_H

#include "VICUS_AbstractDBElement.h"

#include "VICUS_Constants.h"
#include "VICUS_NetworkElement.h"
#include "VICUS_CodeGenMacros.h"
#include "VICUS_Database.h"

namespace VICUS {

/*! Defines the structure of a sub-network (e.g. building heat exchanger with control value) that
	can be instantiated several times in different nodes.
	SubNetwork objects are referenced by Node objects.

	A sub-network is a template for a structure/network of individual flow elements that is used many times
	in the global network. Individual instances of a sub-network only differ in the heat exchange parametrization,
	e.g. energy loss/gain in buildings.

	There can be only one element in a sub-network with heat exchange.
*/
class SubNetwork : public AbstractDBElement {
public:
	SubNetwork();

	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE
	VICUS_COMPARE_WITH_ID

	/*! Checks if all referenced materials exist and if their parameters are valid. */
	bool isValid(const VICUS::Database<VICUS::NetworkHeatExchange> & heatExchangeParameters) const;

	/*! Comparison operator */
	ComparisonResult equal(const AbstractDBElement *other) const override;

private:
	/*! Unique ID of the sub-network. */
	IDType							m_id = INVALID_ID;						// XML:A:required

	/*! Defines sub-network through elements, connected by implicitely numbered internal nodes.
		Nodes with ID 0 represent inlet and outlet node (there must be only one ID 0 inlet and outlet node!).
	*/
	std::vector<NetworkElement>		m_elements;								// XML:E

	/*! Stores index of element with heat exchange parameterization. INVALID_ID means no heat exchange. */
	unsigned int					m_heatExchangeElementIdx = INVALID_ID;

};



} // Namespace VICUS


#endif // SUBNETWORK_H
