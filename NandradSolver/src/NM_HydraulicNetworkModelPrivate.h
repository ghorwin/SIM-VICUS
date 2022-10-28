#include "NM_HydraulicNetworkModel.h"

//#include <NANDRAD_HydraulicNetwork.h>
//#include <NANDRAD_HydraulicNetworkComponent.h>
//#include <NANDRAD_KeywordList.h>

//#include <IBK_messages.h>
//#include <IBK_Exception.h>

#include <IBKMK_DenseMatrix.h>
#include <IBKMK_SparseMatrixCSR.h>

#include <klu.h>

#include "NM_HydraulicNetworkFlowElements.h"

namespace NANDRAD_MODEL {


/*! Define a network connection. */
struct Element {
	Element() {}
	Element(unsigned int i1, unsigned int i2) :
		m_nodeIndexInlet(i1), m_nodeIndexOutlet(i2) {}

	/*! Index of inlet node. */
	unsigned int m_nodeIndexInlet;
	/*! Index of outlet node. */
	unsigned int m_nodeIndexOutlet;
};

/*! Defines a network node including connected elements. */
struct Node {
	Node() {}
	Node(unsigned int i1, unsigned int i2) {
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
	are stored as well as their connectivity information.
*/
struct Network {
	/*! Sorted list of elements */
	std::vector<Element>	m_elements;
	/*! Sorted list of Nodes*/
	std::vector<Node>		m_nodes;
};

// *** Pimpl class declaration ***

class HydraulicNetworkModelImpl {
public:
	HydraulicNetworkModelImpl(const std::vector<Element> &elems, unsigned int referenceElemIdx);
	~HydraulicNetworkModelImpl();

	/*! Initialized solver based on current content of m_flowElements.
		Setup needs to be called whenever m_flowElements vector changes
		(but not, when parameters inside flow elements change!).
	*/
	void setup();
	/*! Solves the flow network equation system.
		You must call setup() before calling solve.
	*/
	int solve();

	/*! Copies current solution in m_y to m_yLast vector. */
	void storeSolution();

	/*! Computes and returns serialization size in bytes. */
	std::size_t serializationSize() const;

	/*! Stores control value at memory*/
	void serialize(void* & dataPtr) const;

	/*! Restores control value from memory.*/
	void deserialize(void* & dataPtr);

	/*! Container for flow element implementation objects.
		Need to be populated before calling setup.
	*/
	std::vector<HydraulicNetworkAbstractFlowElement*>	m_flowElements;
	/*! Index of node with reference pressure. */
	unsigned int										m_pressureRefNodeIdx = NANDRAD::INVALID_ID;
	/*! Reference pressure. */
	double												m_referencePressure = 0.0;
	/*! Network structure. */
	Network												m_network;
	/*! Mass fluxes through all elements*/
	std::vector<double>									m_fluidMassFluxes;
	/*! Container with pressures for inlet node of each flow element.
	*/
	std::vector<double>									m_inletNodePressures;
	/*! Container with pressures for each node of each flow element.
	*/
	std::vector<double>									m_outletNodePressures;
	/*! Container with pressure differences for each flow element.
	*/
	std::vector<double>									m_pressureDifferences;
	/*! Container with absolute pressures for inlet node of each flow element.
	*/
	std::vector<double>									m_inletNodeAbsolutePressures;
	/*! Container with absolute pressures for each outlet node of each flow element.
	*/
	std::vector<double>									m_outletNodeAbsolutePressures;

private:

	enum LESSolver {
		LESDense,
		LESSparse
	};

	/*! Structure for dense jacobian */
	struct DenseSolver {
		// jacobian
		IBKMK::DenseMatrix						m_jacobian;
		// jacobian after factorization
		IBKMK::DenseMatrix						m_jacobianFactorized;
	};

	/*! Structure for sparse jacobian and KLU-specific data */
	struct SparseSolver {
		// jacobian
		IBKMK::SparseMatrixCSR					m_jacobian;
		// jacobian coloring
		std::vector<std::vector<unsigned int> >	m_jacobianColors;
		// KLU members:
		// symbolic matrix factorization
		klu_symbolic							*m_KLUSymbolic = nullptr;
		// numerical matrix values
		klu_numeric								*m_KLUNumeric = nullptr;
		// parameters
		klu_common								m_KLUParas;
	};

	void printVars() const;
	void writeNetworkGraph() const;

	/*! Computes system equation (becomes RHS of Newton method). */
	void updateG();

	/*! Initialize jacobian and create analytical structures (pattern, KLU reordering,..). */
	void jacobianInit();

	/*! Updates jacobian data and returns 1, if an error occured, otherwise 0. */

	int jacobianSetup();

	/*! Multiplies jacobian with b and stores result in res. */
	void jacobianMultiply(const std::vector<double> &b, std::vector<double> &res);

	/*! Solves linear equation system and returns 1, if an error occured, otherwise 0. */
	int jacobianBacksolve(std::vector<double> & rhs);

	/*! Writes jacobian to desctop. */
	void jacobianWrite(std::vector<double> & rhs);

	/*! Flag indicating whether a dense or sparse matrix representation should
		be used*/
	LESSolver							m_solverOptions = LESSparse;
	/*! Structure storing dense jacobian information. */
	DenseSolver							m_denseSolver;
	/*! Stucture storing sparse jacobian and KLU solver information. */
	SparseSolver						m_sparseSolver;

	unsigned int						m_nodeCount;
	unsigned int						m_elementCount;

	std::vector<double>					m_nodalPressures;

	/*! Vector with unknowns. */
	std::vector<double>					m_y;
	/*! Vector with solution obtained after last integration step was completed, used
		to re-initialize solution for next step.
	*/
	std::vector<double>					m_yLast;
	/*! Vector with system function. */
	std::vector<double>					m_G;
};


} // namespace NANDRAD
