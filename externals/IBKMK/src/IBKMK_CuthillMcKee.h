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

#ifndef IBKMK_CuthillMcKeeH
#define IBKMK_CuthillMcKeeH

#include <vector>

#include <IBK_point.h>

namespace IBKMK {

/*! Stores information about a rectilinear computational grid and provides
	grid numbering functionality.
	To use this class first populate the vectors m_nodeConnections and
	m_coordinateMap, then call generate() to generate the grid numbering.
	Afterwards you can call the member function halfBandWidth() to obtain
	information of the maximum half-bandwidth of the generated numbering graph.
	nodeMap() returns a mapping of old to new element numbers.
*/
class Grid {
public:

	/*! Based on original grid numbering, stores for each element the
		indices of its neighboring elements in arbitrary order.
		\code
		// add connections for element 15
		m_nodeConnections[15].push_back(14).
		m_nodeConnections[15].push_back(16).
		m_nodeConnections[15].push_back(22).
		\endcode
	*/
	std::vector< std::vector<unsigned int> >	m_nodeConnections;

	/*! Stores mapping of element number to coordinates in a cartesian grid with
		i,j,k numbering.
		\code
		// store coordinates for element 15
		m_coordinateMap[15].set(1,5,2);
		\endcode
	*/
	std::vector< IBK::point3D<unsigned int> >	m_coordinateMap;

	/*! Performs a full grid numbering with domain detection and domain-specific numbering
		optimization.
		The m_nodeConnections and m_coordinateMap member variables will be changed to match the
		new numbering. The relation between old and new numbering is returned in the mapping
		nodeMap().
	*/
	void generate();

	/*! Returns the old-to-new node number mapping. */
	const std::vector<unsigned int> & nodeMap() const { return m_nodeMap; }
	/*! Returns the half-bandwidth of the system. */
	unsigned int halfBandWidth() const;

private:
	/*! Stores data collected about a certain domain. */
	struct Domain {
		/*! Maximum half bandwidth in domain. */
		unsigned int m_halfBandWidth;
		/*! Minimum degree of nodes encountered in each domain. */
		unsigned int m_minDegree;
		/*! First node number in domain. */
		unsigned int m_firstNode;
		/*! Last node number in domain. */
		unsigned int m_endNode;
	};

	/*! Index order type used for rectilinear numbering. */
	enum IndexOrderType {
		IOT_IJK,
		IOT_IKJ,
		IOT_JIK,
		IOT_JKI,
		IOT_KIJ,
		IOT_KJI
	};

	/*! Performs a Cuthill-McKee-Renumbering.
		The nodes are origially numbered 0...n-1 (named <old node number>).

		The algorithm may number a subset of all nodes, when n is less than m_nodeConnections.size(). In this
		case only the nodeMap values in the index range startLabelIdx ... startLabelIdx+n are updated.

		\param n Number of nodes to re-number (may be less than m_nodeConnections.size())
		\param startNodeIdx Starting node number for numbering (old numbering).
		\param startLabelIdx Starting node number for new numbering. This is typically the first node in a domain
							 when numbering domain-nodes only.
		\param domains Vector of domains populated during numbering.
		\param nodeMap Map with old-to-new node number relations (size m_nodeConnections.size()).

		\note This function will be called to do an initial CMK numbering of the whole
			  grid with domain detection, and afterwards for individual domains.
	*/
	void CMK(unsigned int n,
		unsigned int startNodeIdx,
		unsigned int startLabelIdx,
		std::vector<Domain> & domains,
		std::vector<unsigned int> & nodeMap
		) const;

	/*! Performs an optimized rectilinear grid numbering.
		\param n Number of nodes to re-number (may be less than m_nodeConnections.size())
		\param startNodeIdx Starting node number for numbering (old numbering).
		\param nodeMap Map with old-to-new node number relations (size m_nodeConnections.size()).
		\return Returns half-bandwidth obtained through selected numbering.
	*/
	unsigned int rectilinearNumbering(unsigned int n, unsigned int startNodeIdx, std::vector<unsigned int> & nodeMap) const;

	/*! Node mapping, original numbering as used during construction of m_nodeConnections to
		new numbering.
	*/
	std::vector<unsigned int>	m_nodeMap;

	/*! Vector of domains, generated during first call to CMK which does the domain detection. */
	std::vector<Domain>			m_domains;
};




// below are c-style APIs

struct CMK_Domain {
	/*! Maximum half bandwidth in domain. */
	unsigned int halfBandWidth;
	/*! Minimum degree of nodes encountered in each domain. */
	unsigned int minDegree;
	/*! Last node number in domain. */
	unsigned int endNode;
};


/*! Performs a Cuthill-McKee-Renumbering.
	The nodes are origially numbered 0...n-1 (named <old node number>).

	\param n Number of nodes to re-number.
	\param maxDegree Maximum degree (number of neighbors) of a single node.
	\param iStart Node number to start numbering from.
	\param degree Pointer to array of length n that holds the degree of all nodes.
	\param nodeConnections Pointer to array of length n*maxDegree holding node numbers (old numbering),
							stored in row-major order.
	\param nDomains	Pointer to unsigned int to hold number of independent domains. On input, holds maximum number of allowed domains.
	\param domain Pointer to array of length n to store domain data.
	\param nodeMap Pointer to array of length n to hold mapping between old and new numbering.
	\param hbw Half-bandwidth obtained through CMK numbering (maximum of bandwidth in all domains).

	\code
	unsigned int i = 5; // <old node number>
	unsigned int deg = degree[i] // degree of node
	// get array of node neighbor numbers
	const unsigned int * nodeNeighbors = nodeConnections + i*maxDegree;

	unsigned int newNodeNumber = nodeMap[i];
	\endcode
*/
void CMK(unsigned int n,
	unsigned int maxDegree,
	const unsigned int * degree,
	const unsigned int * nodeConnections,
	unsigned int iStart,
	unsigned int * nDomains,
	struct CMK_Domain * domain,
	unsigned int * nodeMap,
	unsigned int * hbw
	);



} // namespace IBKMK

#endif // IBKMK_CuthillMcKeeH
