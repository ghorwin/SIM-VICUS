#ifndef NETWORK_H
#define NETWORK_H

#include <vector>
#include <IBKMK_DenseMatrix.h>
#include <IBKMK_SparseMatrixCSR.h>

#include <klu.h>


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
	  klu_symbolic								*m_KLUSymbolic = nullptr;
	  // numerical matrix values
	  klu_numeric								*m_KLUNumeric = nullptr;
	  // parameters
	  klu_common								m_KLUParas;
	};

	Network();
	~Network();

	void printVars() const;
	void writeNetworkGraph() const;
	void solve();

	/*! Computes system equation (becomes RHS of Newton method). */
	void updateG();

	/*! Initialize jacobian and create analytical structures (pattern,
	 * KLU reordering,..). */
	void jacobianInit();

	/*! Updates jacobian data. */

	void jacobianSetup();

	/*! Multiplies jacobian with b and stores result in res. */
	void jacobianMultiply(const std::vector<double> &b, std::vector<double> &res);

	/*! Solves linear equation system. */
	void jacobianBacksolve(std::vector<double> & rhs);

	/*! Writes jacobian to desctop. */
	void jacobianWrite(std::vector<double> & rhs);

	/*! Flag indicating whether a dense or sparse matrix representation should
		be used*/
	LESSolver							m_solverOptions = LESSparse;
	/*! Structure storing dense jacobian information. */
	DenseSolver							m_denseSolver;
	/*! Stucture storing sparse jacobian and KLU solver information. */
	SparseSolver						m_sparseSolver;

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
