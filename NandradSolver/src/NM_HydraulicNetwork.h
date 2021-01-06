#ifndef HydraulicNetworkH
#define HydraulicNetworkH

#include <string>
#include <vector>

namespace NANDRAD {
	class HydraulicNetwork;
};

namespace NANDRAD_MODEL {

/*! Define a network connection. */
struct HydraulicNetworkElement {
	HydraulicNetworkElement() {}
	HydraulicNetworkElement(unsigned int n_inlet, unsigned int n_outlet) :
		m_nInlet(n_inlet), m_nOutlet(n_outlet) {}

	/*! Index of inlet node. */
	unsigned int m_nInlet;
	/*! Index of outlet node. */
	unsigned int m_nOutlet;
};

/*! Defines a network node including connected elements. */
struct HydraulicNetworkNode {
	HydraulicNetworkNode()  {}
	HydraulicNetworkNode(unsigned int i1, unsigned int i2) {
		m_elementIndexesInlet.push_back(i1);
		m_elementIndexesOutlet.push_back(i2);
		m_elementIndexes.push_back(i1);
		m_elementIndexes.push_back(i2);
	}

	/*! Vector with indexes of inlet flow elements. */
	std::vector<unsigned int> m_elementIndexesInlet;
	std::vector<unsigned int> m_elementIndexesOutlet;
	/*! Complete vector of indexes. */
	std::vector<unsigned int> m_elementIndexes;
};

/*! Defines a hydraulic network including nodes and elements.
	Contrary to the NANDRAD data structure, indexes (not ids) of elements and nodes
	are stored as well as their connecitivy information.
*/
class HydraulicNetwork {
public:

	HydraulicNetwork()  {}

	/*! Setup network from NANDRAD data structure.*/
	void setup(const NANDRAD::HydraulicNetwork &nw);

	/*! Checks network for completeness and uniqueness.
		// TODO : Anne, check if this needs to be called from somewhere...
	*/
	void check(std::string &errmsg) const;

	/*! Sorted list of elements*/
	std::vector<HydraulicNetworkElement>	m_elements;
	/*! Sorted list of Nodes*/
	std::vector<HydraulicNetworkNode>		m_nodes;
};

} // namespace NANDRAD_MODEL

#endif // HydraulicNetworkH
