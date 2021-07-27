/*	IBK Math Kernel Library
	Copyright (c) 2001-today, Institut fuer Bauklimatik, TU Dresden, Germany

	Written by A. Nicolai, A. Paepcke, H. Fechner, St. Vogelsang
	All rights reserved.

	This file is part of the IBKMK Library.

	Redistribution and use in source and binary forms, with or without modification,
	are permitted provided that the following conditions are met:

	1. Redistributions of source code must retain the above copyright notice, this
	   list of conditions and the following disclaimer.

	2. Redistributions in binary form must reproduce the above copyright notice,
	   this list of conditions and the following disclaimer in the documentation
	   and/or other materials provided with the distribution.

	3. Neither the name of the copyright holder nor the names of its contributors
	   may be used to endorse or promote products derived from this software without
	   specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
	ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
	DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
	ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
	(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
	LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
	ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	This library contains derivative work based on other open-source libraries,
	see LICENSE and OTHER_LICENSES files.

*/

#include "IBKMK_CuthillMcKee.h"


#include <vector>
#include <algorithm>

#include <IBK_messages.h>
#include <IBK_assert.h>
#include <IBK_matrix_3d.h>

namespace IBKMK {


#define INVALID_INDEX (unsigned int)-1

unsigned int Grid::halfBandWidth() const {
	// determine max of all domain-halfbandwidths
	unsigned int hbw = 0;
	for (unsigned int i=0; i<m_domains.size(); ++i)
		hbw = std::max(hbw, m_domains[i].m_halfBandWidth);
	return hbw;
}

void Grid::generate() {
	const char * const FUNC_ID = "[Grid::generate]";

	// 2. Domain dedection -> use CMK algorithm with embedded domain detection
	// 3. For each domain:
	//    a) perform rectiliniear grid numbering
	//    b) select suitable starting points for CMK
	//    c) For each starting point:
	//       - do CMK algorithm within domain
	//       - keep mapping with smallest half-bandwidth (must be smaller than
	//         half-bandwidth of rectilinear numbering


	// *** (2) ***

	// domain detection
	unsigned int nElements = (unsigned int)m_nodeConnections.size();
	std::vector<unsigned int> nodeMap(nElements);
	CMK(nElements, 0, 0, m_domains, nodeMap);

	// apply new numbering
	std::vector<IBK::point3D<unsigned int> > newCoordinateMap(nElements);
	std::vector< std::vector<unsigned int> > newNodeConnections(nElements);
	// e is old numbering/label
	// nodeMap[e] is new numbering/label
	// in m_coordinateMap and m_nodeConnections replace all occurrances of e with nodeMap[e]
	for (unsigned int e=0; e<nElements; ++e) {
		newCoordinateMap[ nodeMap[e] ] = m_coordinateMap[e];
		unsigned int degree = (unsigned int)m_nodeConnections[e].size();
		for (unsigned int n=0; n<degree; ++n) {
			newNodeConnections[ nodeMap[e] ].push_back( nodeMap[m_nodeConnections[e][n] ] );
		}
	}
	// swap with member variables
	m_coordinateMap.swap(newCoordinateMap);
	m_nodeConnections.swap(newNodeConnections);
	m_nodeMap.swap(nodeMap);
	// m_nodeMap stores the mapping of the original numbering to the numbering currently used in
	// m_coordinateMap and m_nodeConnections


	// *** 3 ***
	std::vector<Domain> dummyDomains;

	// initialize neutral nodeMap (no change)
	nodeMap.resize(nElements);
	for (unsigned int n=0; n<nElements; ++n)
		nodeMap[n] = n;

	bool haveBetterBandwidth = false;
	std::vector<unsigned int> bestNodeMap = nodeMap;

	// optimize numbering in each domain
	for (unsigned int d=0; d<m_domains.size(); ++d) {
		IBK::IBK_Message( IBK::FormatString("Optimizing numbering in domain #%1.\n").arg(d+1), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		IBK::MessageIndentor indent; (void)indent;
		unsigned int domainSize = m_domains[d].m_endNode-m_domains[d].m_firstNode+1;
		unsigned int bestHalfBandwidth = m_domains[d].m_halfBandWidth;
		IBK::IBK_Message( IBK::FormatString("Domain element range: %1..%2, size = %3\n")
			.arg(m_domains[d].m_firstNode)
			.arg(m_domains[d].m_endNode)
			.arg(domainSize), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);

		// 3.a) attempt rectilinear numbering in domain
		unsigned int hbw = rectilinearNumbering(domainSize, m_domains[d].m_firstNode, nodeMap);
		// remember generated mapping if better than previous numbering
		if (hbw < bestHalfBandwidth) {
			bestHalfBandwidth = hbw;
			bestNodeMap = nodeMap;
		}

		// 3.b) collect list of minimum degree nodes for CMK numbering optimization
		std::vector<unsigned int> minDegreeNodes;
		for (unsigned int e=m_domains[d].m_firstNode; e<=m_domains[d].m_endNode; ++e) {
			if (m_nodeConnections[e].size() == m_domains[d].m_minDegree)
				minDegreeNodes.push_back(e);
		}

		// 3.c) attempt CMK renumbering for each minDegreeNode
		IBK::IBK_Message( IBK::FormatString("Optimizing CMK numbering starting from node:\n"), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		IBK::MessageIndentor indent2; (void)indent2;
		for (unsigned int n=0; n<minDegreeNodes.size(); ++n) {
			// starting node
			unsigned int startIdx = minDegreeNodes[n];
			CMK(domainSize, startIdx, m_domains[d].m_firstNode, dummyDomains, nodeMap);
			IBK::IBK_Message( IBK::FormatString("%1 -> hbw = %2\n").arg(startIdx).arg(dummyDomains[0].m_halfBandWidth), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
			// if computed bandwith is better than previous, store as best numbering
			if (dummyDomains[0].m_halfBandWidth < bestHalfBandwidth) {
				bestHalfBandwidth = dummyDomains[0].m_halfBandWidth;
				bestNodeMap = nodeMap;
			}
		}
		// if best half bandwidth is less then original domain halfbandwidth, perform numbering
		// only apply numbering if bandwidth is smaller than current bandwidth
		if (bestHalfBandwidth < m_domains[d].m_halfBandWidth) {
			haveBetterBandwidth = true;
			// since we are done with the domain, we can store the best halfbandwidth for this domain
			m_domains[d].m_halfBandWidth = bestHalfBandwidth;
			// re-initialize nodeMap with bestNodeMap
			nodeMap = bestNodeMap;
		}
	}

	// update numbering if new numbering is better
	// in this case, bestNodeMap holds optimal numbering determined for all domains
	if (haveBetterBandwidth) {
		newCoordinateMap = m_coordinateMap;
		newNodeConnections = m_nodeConnections;

		//
		for (unsigned int e=0; e<nElements; ++e) {
			newCoordinateMap[ bestNodeMap[e] ] = m_coordinateMap[e];
			unsigned int degree = (unsigned int)m_nodeConnections[e].size();
			for (unsigned int n=0; n<degree; ++n) {
				newNodeConnections[ bestNodeMap[e] ].push_back( bestNodeMap[ m_nodeConnections[e][n] ] );
			}
			// merge m_nodeMap and nodeMap
			m_nodeMap[e] = bestNodeMap[ m_nodeMap[e] ];
		}
		m_coordinateMap.swap(newCoordinateMap);
		m_nodeConnections.swap(newNodeConnections);
	}
}


void Grid::CMK(unsigned int n,
			   unsigned int startNodeIdx,
			   unsigned int startLabelIdx,
			   std::vector<Domain> & domains,
			   std::vector<unsigned int> & nodeMap) const
{
	const char * const FUNC_ID = "[IBK::Grid::CMK]";

	// cache total number of nodes in grid, n can be less than nTotal
	unsigned int nTotal = (unsigned int)m_nodeConnections.size();
	IBK_ASSERT_X(nodeMap.size() == nTotal, "Invalid size for nodeMap.");

	// create queue vector
	std::vector<unsigned int>	Q(nTotal);

	// initialize node map so that we can search for unused/unlabeled nodes
	for (unsigned int p=startLabelIdx; p<n+startLabelIdx; ++p)
		nodeMap[p] = INVALID_INDEX;

	// populate initial queue

	// we start numbering at the given start index
	// Note: we only use the part of the queue Q[startLabelIdx]..Q[startLabelIdx+n-1]
	unsigned int Qidx = startLabelIdx;
	unsigned int Qlen = 1; // current length of Queue is 1
	Q[Qidx] = startNodeIdx;

	// start with empty domains
	domains.clear();

	// initialize current domain
	Domain currentDomain;
	// minimum degree of node in current domain, initialize with maxDegree
	currentDomain.m_minDegree = 6; // assume maximum of 6 neighbors (3D grid)
	// initialize half-bandwidth in this domain
	currentDomain.m_halfBandWidth = 1;
	currentDomain.m_firstNode = startLabelIdx;

	// number all n elements, k is the new node number
	for (unsigned int k = startLabelIdx; k<n+startLabelIdx; ++k) {

		// if queue is empty and we still have nodes (k<n) registert end of domain and search new domain
		if (Qlen == 0) {
			// register domain end
			currentDomain.m_endNode = k-1;
			// add domain to list of domains
			domains.push_back(currentDomain);

			IBK_Message(IBK::FormatString("Starting new domain after %1 nodes.\n").arg(k),
						IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);

			// set new starting node of domain, new domain starts at next k
			currentDomain.m_firstNode = k;
			// reinitialize minimum degree with maxDegree for the next domain
			currentDomain.m_minDegree = 6;
			// reinitialize half-bandwidth in this domain
			currentDomain.m_halfBandWidth = 1;

			// search for next unused node index and re-initialize queue
			for (unsigned int p=0; p<n; ++p) {
				if (nodeMap[p] == INVALID_INDEX) {
					Q[Qidx] = p;
					Qlen = 1;
					break;
				}
			}
		}

		// get next node number from queue
		unsigned int i = Q[Qidx++]; // increase queue index
		Q[Qidx-1] = INVALID_INDEX; // only to assist in debugging
		--Qlen; // decrease queue length

		// relabel node and store node mapping
		nodeMap[i] = k;

		// create convenience pointer to node numbers connected to this node
		const unsigned int * nodeConnect = &(m_nodeConnections[i])[0];

		// get degree of this node
		unsigned int deg = (unsigned int)m_nodeConnections[i].size();
		// update minimum degree
		currentDomain.m_minDegree = std::min(deg, currentDomain.m_minDegree);

		// storage vector for added nodes in each loop, first index degree, second index node number
		std::vector<std::pair<unsigned int, unsigned int> >	Qadd;
		// loop over all neighboring elements
		for (unsigned int j=0; j<deg; ++j) {
			// get node number of neighboring node (nnn == old numbering)
			unsigned int nnn = nodeConnect[j];

			// check if this node has already been renumbered
			if (nodeMap[nnn] != INVALID_INDEX) {
				// compute maximum half bandwidth

				int halfBandwidth = nodeMap[nnn] - k;
				if (halfBandwidth < 0)
					halfBandwidth = -halfBandwidth;
				currentDomain.m_halfBandWidth = std::max(currentDomain.m_halfBandWidth, (unsigned int)halfBandwidth);
				continue; // skip this node
			}

			// check if this node is already in the queue
			unsigned int r = Qidx;
			for (;r<Qidx+Qlen; ++r) {
				if (Q[r] == nnn)
					break;
			}
			if (r != Qidx+Qlen)
				continue;

			// store node number and corresponding degree of this node in local queue
			unsigned int degNNN = (unsigned int)m_nodeConnections[nnn].size();
			Qadd.push_back(std::make_pair(degNNN, nnn) );
		}
		// if node did not have any unnumbered neighboring nodes, continue with next node in queue
		if (Qadd.empty())
			continue;

		// sort nodes according to degree (standard comparison operator of pair)
		std::sort(Qadd.begin(), Qadd.end());

		// add nodes to global queue: Qidx points to first element in queue,
		// Qidx + Qlen points to first element _after_ the current queue
		for (unsigned int p=0; p<Qadd.size(); ++p) {
			Q[Qidx+Qlen+p] = Qadd[p].second;
		}
		// increase queue length
		Qlen += (unsigned int)Qadd.size();
	}

	// register domain end
	currentDomain.m_endNode = startLabelIdx + n - 1;
	// add domain to list of domains
	domains.push_back(currentDomain);
}
//---------------------------------------------------------------------------


unsigned int Grid::rectilinearNumbering(unsigned int n, unsigned int startNodeIdx, std::vector<unsigned int> & nodeMap) const {
	const char * const FUNC_ID = "[Grid::rectilinearNumbering]";

	unsigned int minI = (unsigned int)-1;
	unsigned int minJ = (unsigned int)-1;
	unsigned int minK = (unsigned int)-1;
	unsigned int maxI = 0;
	unsigned int maxJ = 0;
	unsigned int maxK = 0;

	// loop over all elements in selected range
	for (unsigned int e=startNodeIdx; e<startNodeIdx+n; ++e) {
		unsigned int i = m_coordinateMap[e].m_x;
		unsigned int j = m_coordinateMap[e].m_y;
		unsigned int k = m_coordinateMap[e].m_z;
		// determine min/max indices in grid
		minI = std::min(minI, i);
		maxI = std::max(maxI, i);
		minJ = std::min(minJ, j);
		maxJ = std::max(maxJ, j);
		minK = std::min(minK, k);
		maxK = std::max(maxK, k);
	}

	unsigned int col_count		= maxI - minI + 1;
	unsigned int row_count		= maxJ - minJ + 1;
	unsigned int stack_count	= maxK - minK + 1;

	// Create map to identify element number by coordinates.
	// Indices in map are _offsets_ to (minI, minJ, minK).
	IBK::matrix_3d<unsigned int> coordinateToNumberMap(col_count, row_count, stack_count);
	coordinateToNumberMap.fill( INVALID_INDEX);
	for (unsigned int e=startNodeIdx; e<startNodeIdx+n; ++e) {
		unsigned int iOffset = m_coordinateMap[e].m_x - minI;
		unsigned int jOffset = m_coordinateMap[e].m_y - minJ;
		unsigned int kOffset = m_coordinateMap[e].m_z - minK;
		coordinateToNumberMap(iOffset, jOffset, kOffset) = e;
	}

	// smallest must be innerst loop
	IndexOrderType order;
	unsigned int maxBandWidth;
	if (col_count < row_count) {

		if ( stack_count < col_count ) {

			order = IOT_JIK;
			maxBandWidth = col_count * stack_count;

		} else {

			if ( row_count < stack_count ) {

				order = IOT_KJI;
				maxBandWidth = row_count * col_count;

			} else {
				order = IOT_JKI;
				maxBandWidth = stack_count * col_count;

			}

		}

	} else {

		if ( stack_count < row_count ) {

			order = IOT_IJK;
			maxBandWidth = row_count * stack_count;

		} else {

			if ( col_count < stack_count ) {

				order = IOT_KIJ;
				maxBandWidth = col_count * row_count;

			} else {

				order = IOT_IKJ;
				maxBandWidth = stack_count * row_count;

			}
		}
	}


	unsigned int ElementIndexCounter = startNodeIdx;
	switch (order) {

		case IOT_IJK :
			IBK::IBK_Message(IBK::FormatString("Numbering Order: col, row, stack [IJK] (loop order)\n"), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
			// loop over all columns in sketch sk
			for (unsigned int i=0; i<col_count; ++i) {
				// loop over all rows in sketch sk
				for (unsigned int j=0; j<row_count; ++j) {
					// loop over all stacks in sketch sk
					for (unsigned int k=0; k<stack_count; ++k) {
						// skip void elements
						unsigned int e = coordinateToNumberMap(i,j,k);
						if (e == INVALID_INDEX) continue;
						// save mapping of old to new index
						nodeMap[e] = ElementIndexCounter++;
					}
				}
			}
			break;

		case IOT_IKJ :
			IBK::IBK_Message(IBK::FormatString("Numbering Order: col, stack, row [IKJ] (loop order)\n"), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
			// loop over all columns in sketch sk
			for (unsigned int i=0; i<col_count; ++i) {
				// loop over all stacks in sketch sk
				for (unsigned int k=0; k<stack_count; ++k) {
					// loop over all rows in sketch sk
					for (unsigned int j=0; j<row_count; ++j) {
						// skip void elements
						unsigned int e = coordinateToNumberMap(i,j,k);
						if (e == INVALID_INDEX) continue;
						// save mapping of old to new index
						nodeMap[e] = ElementIndexCounter++;
					}
				}
			}
			break;
		case IOT_JIK:
			IBK::IBK_Message(IBK::FormatString("Numbering Order: row, col, stack [JIK] (loop order)\n"), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
			// loop over all rows in sketch sk
			for (unsigned int j=0; j<row_count; ++j) {
				// loop over all columns in sketch sk
				for (unsigned int i=0; i<col_count; ++i) {
					// loop over all stacks in sketch sk
					for (unsigned int k=0; k<stack_count; ++k) {
						// skip void elements
						unsigned int e = coordinateToNumberMap(i,j,k);
						if (e == INVALID_INDEX) continue;
						// save mapping of old to new index
						nodeMap[e] = ElementIndexCounter++;
					}
				}
			}
			break;
		case IOT_JKI:
			IBK::IBK_Message(IBK::FormatString("Numbering Order: row, stack, col [JKI] (loop order)\n"), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
			// loop over all rows in sketch sk
			for (unsigned int j=0; j<row_count; ++j) {
				// loop over all stacks in sketch sk
				for (unsigned int k=0; k<stack_count; ++k) {
					// loop over all columns in sketch sk
					for (unsigned int i=0; i<col_count; ++i) {
						// skip void elements
						unsigned int e = coordinateToNumberMap(i,j,k);
						if (e == INVALID_INDEX) continue;
						// save mapping of old to new index
						nodeMap[e] = ElementIndexCounter++;
					}
				}
			}
			break;
		case IOT_KIJ:
			IBK::IBK_Message(IBK::FormatString("Numbering Order: stack, col, row [KIJ] (loop order)\n"), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
			// loop over all stacks in sketch sk
			for (unsigned int k=0; k<stack_count; ++k) {
				// loop over all columns in sketch sk
				for (unsigned int i=0; i<col_count; ++i) {
					// loop over all rows in sketch sk
					for (unsigned int j=0; j<row_count; ++j) {
						// skip void elements
						unsigned int e = coordinateToNumberMap(i,j,k);
						if (e == INVALID_INDEX) continue;
						// save mapping of old to new index
						nodeMap[e] = ElementIndexCounter++;
					}
				}
			}
			break;
		case IOT_KJI:
			IBK::IBK_Message(IBK::FormatString("Numbering Order: stack, row, col [KJI] (loop order)\n"), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
			// loop over all stacks in sketch sk
			for (unsigned int k=0; k<stack_count; ++k) {
				// loop over all rows in sketch sk
				for (unsigned int j=0; j<row_count; ++j) {
					// loop over all columns in sketch sk
					for (unsigned int i=0; i<col_count; ++i) {
						// skip void elements
						unsigned int e = coordinateToNumberMap(i,j,k);
						if (e == INVALID_INDEX) continue;
						// save mapping of old to new index
						nodeMap[e] = ElementIndexCounter++;
					}
				}
			}
			break;
		default:
			break;
	}

	return maxBandWidth;
}
//---------------------------------------------------------------------------




void CMK(unsigned int n,
		 unsigned int maxDegree,
		 const unsigned int * degree,
		 const unsigned int * nodeConnections,
		 unsigned int iStart,
		 unsigned int * nDomains,
		 struct CMK_Domain * domain,
		 unsigned int * nodeMap,
		 unsigned int * hbw)
{
	const char * const FUNC_ID = "[IBK::CMK]";

	/// \todo Q should be a workspace vector passed to this function
	std::vector<unsigned int>	Qvec(n);
	unsigned int * Q = &Qvec[0];

	// initialize node map
	for (unsigned int p=0; p<n; ++p)
		nodeMap[p] = (unsigned int)-1;

	// populate initial queue

	// we start numbering at some given start index.
	unsigned int Qidx = 0;
	unsigned int Qlen = 1;
	Q[0] = iStart;

	// remember maximum number of domains
	unsigned int maxDomains = *nDomains;
	// initialize computed number of domains with 0.
	*nDomains = 0;
	// minimum degree of node in current domain, initialize with maxDegree
	domain[*nDomains].minDegree = maxDegree;
	// initialize half-bandwidth in this domain
	domain[*nDomains].halfBandWidth = 1;

	*hbw = 1;

	// storage vector for added nodes in each loop, first index degree, second index node number
	std::vector<std::pair<unsigned int, unsigned int> >	Qadd(maxDegree);

	// number all n elements, k is the new node number
	for (unsigned int k = 0; k<n; ++k) {

		// if queue is empty and we still have nodes (k<n) registert end of domain and search new domain
		if (Qlen == 0) {
			if (maxDomains > *nDomains) {
				// register domain end
				domain[*nDomains].endNode = k;
				// store maximum of bandwidths so far
				if (domain[*nDomains].halfBandWidth > *hbw)
					*hbw = domain[*nDomains].halfBandWidth;
				// increase domain counter
				++(*nDomains);

				IBK_Message(IBK::FormatString("Starting new domain after %1 nodes.\n").arg(k),
							IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_DEVELOPER);

				// reinitialize minimum degree with maxDegree for the next domain
				domain[*nDomains].minDegree = maxDegree;
				// reinitialize half-bandwidth in this domain
				domain[*nDomains].halfBandWidth = 1;
			}

			// search for next unused node index
			for (unsigned int p=0; p<n; ++p) {
				if (nodeMap[p] == (unsigned int)-1) {
					Q[Qidx] = p;
					Qlen = 1;
					break;
				}
			}
		}

		// get next node number from queue
		unsigned int i = Q[Qidx++]; // increase queue index
		--Qlen; // decrease queue length

		// relabel node and store node mapping
		nodeMap[i] = k;

		// create convenience pointer to node numbers connected to this node
		const unsigned int * nodeConnect = nodeConnections + i*maxDegree;

		// get degree of this node
		unsigned int deg = degree[i];
		// update minimum degree
		domain[*nDomains].minDegree = std::min(deg, domain[*nDomains].minDegree);

		// create counter for all nodes newly added to the queue
		unsigned int addedNodeCount = 0;
		// loop over all neighboring elements
		for (unsigned int j=0; j<deg; ++j) {
			// get node number of neighboring node (nnn - old numbering)
			unsigned int nnn = nodeConnect[j];

			// check if this node has already been renumbered
			if (nodeMap[nnn] != (unsigned int)-1) {
				// compute maximum half bandwidth

				/// \todo fix this formula
				int halfBandwidth = nodeMap[nnn] - k;
				if (halfBandwidth < 0)
					halfBandwidth = -halfBandwidth;
				domain[*nDomains].halfBandWidth = std::max(domain[*nDomains].halfBandWidth, (unsigned int)halfBandwidth);
				continue; // skip this node
			}

			// check if this node is already in the queue
			unsigned int r = Qidx;
			for (;r<Qidx+Qlen; ++r)
				if (Q[r] == nnn)
					break;
			if (r != Qidx+Qlen)
				continue;

			// store node and degree of this node in local queue
			Qadd[addedNodeCount].first = degree[nnn];
			Qadd[addedNodeCount].second = nnn;

			++addedNodeCount; // increase node counter
		}
		// if node did not have any unnumbered neighboring nodes, continue with next node in queue
		if (addedNodeCount == 0)
			continue;

		// sort nodes according to degree
		std::sort(Qadd.begin(), Qadd.begin()+addedNodeCount);

		// add nodes to global queue
		for (unsigned int p=0; p<addedNodeCount; ++p) {
			Q[Qidx+Qlen+p] = Qadd[p].second;
		}
		Qlen += addedNodeCount;

	}

	// register domain end
	domain[*nDomains].endNode = n;
	// store maximum of bandwidths so far
	if (domain[*nDomains].halfBandWidth > *hbw)
		*hbw = domain[*nDomains].halfBandWidth;
	// increase domain counter
	++(*nDomains);
}

} // namespace IBKMK
