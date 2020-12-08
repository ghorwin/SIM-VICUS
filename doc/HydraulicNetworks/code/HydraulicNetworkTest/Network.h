#ifndef NETWORK_H
#define NETWORK_H

#include <vector>
#include <IBKMK_DenseMatrix.h>

class AbstractFlowElement;

class Network {
public:


	/*! Stores connectivity information. */
	struct Node {
		Node() {}
		Node(unsigned int i1, unsigned int i2) {
			m_flowElementIndexes.push_back(i1);
			m_flowElementIndexes.push_back(i2);
		}

		/*! Vector with indexes of connected flow elements. */
		std::vector<unsigned int> m_flowElementIndexes;
	};


	Network();

	void printVars() const;
	void writeNetworkGraph() const;
	void solve();

	/*! Computes system equation (becomes RHS of Newton method). */
	void updateG(const std::vector<double> & y);

	IBKMK::DenseMatrix					m_jacobian;

	std::vector<AbstractFlowElement*>	m_flowElements;
	std::vector<Node>					m_nodes;

	unsigned int						m_nodeCount;
	unsigned int						m_elementCount;

	std::vector<double>					m_massFluxes;
	std::vector<double>					m_nodePressures;

	/*! Vector with unknowns. */
	std::vector<double>					m_y;
	/*! Vector with system function. */
	std::vector<double>					m_G;
};

#endif // NETWORK_H
