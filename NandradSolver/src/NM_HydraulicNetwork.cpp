#include "NM_HydraulicNetwork.h"

#include <NANDRAD_HydraulicNetwork.h>

#include <IBK_messages.h>

#include <IBKMK_SparseMatrixPattern.h>

namespace NANDRAD_MODEL {


void HydraulicNetwork::setup(const NANDRAD::HydraulicNetwork & nw) {

	// first register all nodes
	std::set<unsigned int> nodeIds;
	// for this purpose process all hydraulic network elements
	for (const NANDRAD::HydraulicNetworkElement & e : nw.m_elements) {
		nodeIds.insert(e.m_inletNodeId);
		nodeIds.insert(e.m_outletNodeId);
	}

	// now populate the m_flowElements vector of the network solver

	// process all hydraulic network elements and copy index
	for (const NANDRAD::HydraulicNetworkElement & e : nw.m_elements) {
		unsigned int nInlet = std::distance(nodeIds.begin(), nodeIds.find(e.m_inletNodeId));
		unsigned int nOutlet = std::distance(nodeIds.begin(), nodeIds.find(e.m_outletNodeId));
		m_elements.push_back(HydraulicNetworkElement(nInlet, nOutlet) );
	}

	// setup nodes
	// and create fast access connections between nodes and flow elements
	m_nodes.resize(nodeIds.size());
	for (unsigned int i=0; i<m_elements.size(); ++i) {
		HydraulicNetworkElement &fe = m_elements[i];
		// TODO : check inlet must be different from outlet
		m_nodes[fe.m_nInlet].m_elementIndexesInlet.push_back(i);
		m_nodes[fe.m_nOutlet].m_elementIndexesOutlet.push_back(i);
		m_nodes[fe.m_nInlet].m_elementIndexes.push_back(i);
		m_nodes[fe.m_nOutlet].m_elementIndexes.push_back(i);
	}

}


void HydraulicNetwork::check(std::string &errmsg) const {


	// error checks:
	// 1.) no open ends
	// -> all m_nodes[i] must have at least 2 m_flowElementIndexes
	// 2.) no single cycles:
	// -> inlet must be different from outlet
	for (unsigned int i=0; i<m_nodes.size(); ++i) {
		const HydraulicNetworkNode &node = m_nodes[i];
		// error check 1
		if(node.m_elementIndexes.size() == 1){
			errmsg += IBK::FormatString(
					"FlowElement with id %1 is an open end of hydraulic network!")
					 .arg(node.m_elementIndexes[0]).str();
		}
		// error check 2
		std::set<unsigned int> indexes;
		for(unsigned int j = 0; j < node.m_elementIndexes.size(); ++j) {
			unsigned int elementIdx = node.m_elementIndexes[j];
			if(indexes.find(elementIdx) != indexes.end()){
				errmsg += IBK::FormatString(
						"FlowElement with id %1 is an invalid cyclic connection!")
						 .arg(elementIdx).str();
			}
		}
	}


	// 3.) no distinct networks
	// -> each node must connect to any other
	// -> transitive closure of connectivity must form a dense matrix
	IBKMK::SparseMatrixPattern connectivity(m_nodes.size());
#ifdef BIDIRECTIONAL

	for (unsigned int k=0; k<m_elements.size(); ++k) {
		const HydraulicNetworkElement &fe = m_elements[k];
		// TODO : check inlet must be different from outlet
		unsigned int i = fe.m_nInlet;
		unsigned int j = fe.m_nOutlet;
		// set a pattern entry for connected nodes
		if(!connectivity.test(i,j))
			connectivity.set(i,j);
		// as well as for the transposed
		if(!connectivity.test(j,i))
			connectivity.set(j,i);
	}

	// calculate transitive closure
	for(unsigned int k = 0; k < m_nodes.size(); ++k) {
		// set a connection (i,j) for each entry (i,k), (k,j)
		std::vector<unsigned int> rows;
		std::vector<unsigned int> cols;
		connectivity.indexesPerRow(k,cols);
		connectivity.indexesPerRow(k,rows);

		// set all entries (rows[iIdx], cols[jIdx])
		for(unsigned int iIdx = 0; iIdx < rows.size(); ++iIdx) {
			unsigned int i = rows[iIdx];
			for(unsigned int jIdx = 0; jIdx < cols.size(); ++jIdx) {
				unsigned int j = cols[jIdx];
				// set entry
				if(!connectivity.test(i,j))
					connectivity.set(i,j);
				// set symmetric entry
				if(!connectivity.test(j,i))
					connectivity.set(j,i);
			}
		}
	}

#else
	IBKMK::SparseMatrixPattern connectivityTranspose(m_nodes.size());

	for (unsigned int k=0; k<m_elements.size(); ++k) {
		const HydraulicNetworkElement &fe = m_elements[k];
		// TODO : check inlet must be different from outlet
		unsigned int i = fe.m_nInlet;
		unsigned int j = fe.m_nOutlet;
		// set a pattern entry for connected nodes
		if(!connectivity.test(i,j))
			connectivity.set(i,j);
		// as well as for the transposed
		if(!connectivityTranspose.test(j,i))
			connectivityTranspose.set(j,i);
	}

	// calculate transitive closure
	for(unsigned int k = 0; k < m_nodes.size(); ++k) {
		// set a connection (i,j) for each entry (i,k), (k,j)
		std::vector<unsigned int> rows;
		std::vector<unsigned int> cols;
		connectivity.indexesPerRow(k,cols);
		connectivityTranspose.indexesPerRow(k,rows);

		// set all entries (rows[iIdx], cols[jIdx])
		for(unsigned int iIdx = 0; iIdx < rows.size(); ++iIdx) {
			unsigned int i = rows[iIdx];
			for(unsigned int jIdx = 0; jIdx < cols.size(); ++jIdx) {
				unsigned int j = cols[jIdx];
				// set entry
				if(!connectivity.test(i,j))
					connectivity.set(i,j);
				// set symmetric entry
				if(!connectivityTranspose.test(j,i))
					connectivityTranspose.set(j,i);
			}
		}
	}

#endif
	// now assume, that we have a dense matrix pattern for a connected graph
	for(unsigned int i = 0; i < m_nodes.size(); ++i) {
		// count column entries for each row
		std::vector<unsigned int> cols;
		connectivity.indexesPerRow(i,cols);

		// error: missing connections
		if(cols.size() != m_nodes.size()) {
			// isolated nodes are not allowed
			IBK_ASSERT(!cols.empty());

			// find out disjunct network elements
			std::vector<unsigned int> disjunctElements;
			for(unsigned int j = 0; j < cols.size(); ++j) {
				const HydraulicNetworkNode &node = m_nodes[cols[j]];

				for(unsigned int k =0; k < node.m_elementIndexes.size(); ++k)
					disjunctElements.push_back(node.m_elementIndexes[k]);
			}

			// create an error message string
			IBK_ASSERT(!disjunctElements.empty());
			std::string networkStr(IBK::val2string<unsigned int>(disjunctElements[0]));

			for(unsigned int k = 1; k < disjunctElements.size(); ++k)
				networkStr += std::string(",") + IBK::val2string<unsigned int>(disjunctElements[k]);

			errmsg += IBK::FormatString(
					"Network is not completely connected! Distinct network formed by flow elements (%1)!")
					.arg(networkStr).str();
		}
	}
}

} // namespace NANDRAD
