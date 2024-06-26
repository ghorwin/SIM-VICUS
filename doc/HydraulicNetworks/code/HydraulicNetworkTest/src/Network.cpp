#include "Network.h"

#include <iostream>

#include "PipeElement.h"
#include "Pump.h"

#include "IBKMK_SparseMatrixPattern.h"

const double JACOBIAN_EPS = 1e-6; // in Pa and scaled kg/s
const double THRESHOLD = 1;
const double MASS_FLUX_SCALE = 1000;


Network::Network() {

	// create a minimalistic network with 3 nodes

#if 0
	m_flowElements.push_back(new Pump(0, 1) );				// 0 - pump, between nodes 0 and 1
	m_flowElements.push_back(new PipeElement(1, 0,2000));	// 1 - pipe, between nodes 1 and 2
//	m_flowElements.push_back(new PipeElement(2, 0,6000));	// 2 - pipe, between nodes 2 and 0


	// sample flow element system functions to get an idea about value range
//	for (unsigned int i=0; i<10; ++i) {
//		std::cout << i*0.8 << "  " << m_flowElements[0]->systemFunction(i*0.08, 0, 0) << "   " << m_flowElements[1]->systemFunction(i*0.08, 0, 0) << std::endl;
//	}
	std::cout << "pump " << m_flowElements[0]->systemFunction(0.7, 0, 1100) << std::endl;
	std::cout << "pipe " << m_flowElements[1]->systemFunction(0.7, 1100, 0) << std::endl;
#else
	m_flowElements.push_back(new Pump(0, 1) );				// 0 - pump, between nodes 0 and 1
	m_flowElements.push_back(new PipeElement(1, 2,1400));	// 1 - pipe, between nodes 1 and 2
	m_flowElements.push_back(new PipeElement(1, 2,2400));	// 2 - second pipe, between nodes 1 and 2
	m_flowElements.push_back(new PipeElement(2, 0,1000));
	m_flowElements.push_back(new PipeElement(2, 3,1400));
	m_flowElements.push_back(new PipeElement(3, 4,2400));
	m_flowElements.push_back(new PipeElement(4, 0,2400));
//	m_flowElements.push_back(new Pump(2, 4) );				// 0 - pump, between nodes 0 and 1
#endif


	// everything else is generic


	unsigned int nodeCount = 0;
	for (AbstractFlowElement * fe : m_flowElements) {
		nodeCount = std::max(nodeCount, fe->m_nInlet);
		nodeCount = std::max(nodeCount, fe->m_nOutlet);
	}

	writeNetworkGraph();

	m_nodes.resize(nodeCount+1);
	for (unsigned int i=0; i<m_flowElements.size(); ++i) {
		AbstractFlowElement * fe = m_flowElements[i];
		m_nodes[fe->m_nInlet].m_flowElementIndexes.push_back(i);
		m_nodes[fe->m_nOutlet].m_flowElementIndexes.push_back(i);
	}

	m_nodeCount = m_nodes.size();
	m_elementCount = m_flowElements.size();
	unsigned int n = m_nodeCount + m_elementCount;

	m_y.resize(n, 0.2*MASS_FLUX_SCALE);
//	m_y[0] = 0;
//	m_y[1] = 1000;
//	m_y[2] = 0;

#if 0
//	m_y[0] = 5000;
//	m_y[1] = 0;
//	m_y[2] = 0.4;
//	m_y[3] = 0.1;

//	m_y[0] = 0;
//	m_y[1] = 0;
//	m_y[2] = 0.1;
//	m_y[3] = 0.1;

	m_y[0] = 0.1;
	m_y[1] = 0.1;
	m_y[2] = 0;
	m_y[3] = 0;
#endif
	m_G.resize(n);
	m_massFluxes.resize(m_elementCount);
	m_nodePressures.resize(m_nodeCount);

	// create jacobian
	jacobianInit();

}

Network::~Network() {
	// delete KLU specific pointer
	if(m_sparseSolver.m_KLUSymbolic !=  nullptr) {
		klu_free_symbolic(&(m_sparseSolver.m_KLUSymbolic), &(m_sparseSolver.m_KLUParas));
		delete m_sparseSolver.m_KLUSymbolic;
	}
	if(m_sparseSolver.m_KLUNumeric !=  nullptr) {
		klu_free_numeric(&(m_sparseSolver.m_KLUNumeric), &(m_sparseSolver.m_KLUParas));
		delete m_sparseSolver.m_KLUNumeric;
	}
}

double WRMSNorm(const std::vector<double> & vec) {
	double resNorm = 0;
	for (unsigned int i=0; i<vec.size(); ++i)
		resNorm += vec[i]*vec[i];
	resNorm /= vec.size();
	resNorm = std::sqrt(resNorm);
	return resNorm;
}


void Network::printVars() const {
	std::cout << "Mass fluxes [kg/s]" << std::endl;
	for (unsigned int i=0; i<m_elementCount; ++i)
		std::cout << "  " << i << "   " << m_y[i]/MASS_FLUX_SCALE  << std::endl;

	std::cout << "Nodal pressures [Pa]" << std::endl;
	for (unsigned int i=0; i<m_nodeCount; ++i)
		std::cout << "  " << i << "   " << m_y[i + m_elementCount] << std::endl;
}

void Network::writeNetworkGraph() const {
	// generate dot graph file for plotting
	std::stringstream strm;
	strm << "digraph {\n";
	for (AbstractFlowElement * fe : m_flowElements) {
		strm << "  " << fe->m_nInlet+1 << " -> " << fe->m_nOutlet+1;
		if (dynamic_cast<Pump*>(fe) != nullptr) {
			strm << "[fontsize=7, label=\"pump\", weight=200, color=red]";
		}
		strm << ";\n";
	}

	strm << "}\n";

	std::ofstream out("graph.gv");
	out << strm.rdbuf();

	// generate with "
}

void Network::solve() {
	unsigned int n = m_nodeCount + m_elementCount;


	std::vector<double> rhs(n);

	// now start the Newton iteration
	int iterations = 100;
	while (--iterations > 0) {
		// evaluate system function for current guess
		updateG();

		// store RHS
		for (unsigned int i=0; i<m_G.size(); ++i)
			rhs[i] = -m_G[i];

		// compose right hand side (mind the minus sign)
		// and evaluate residuals
		double resNorm = WRMSNorm(m_G);
		std::cout << "res = " << resNorm << std::endl;
		if (resNorm < THRESHOLD)
			break;

		// now compose Jacobian with FD quotients

		// perform jacobian update
		jacobianSetup();

		std::cout << "\n\n*** Iter " << 100-iterations  << std::endl;

		printVars();
		jacobianWrite(rhs);

#ifdef RESIDUAL_TEST
		std::vector<double> originalRHS(rhs);
#endif // RESIDUAL_TEST

		// now solve the equation system
		jacobianBacksolve(rhs);

		std::cout << "deltaY" << std::endl;
		for (unsigned int i=0; i<n; ++i)
			std::cout << "  " << i << "   " << rhs[i]  << std::endl;

#ifdef RESIDUAL_TEST
		// check if equation system was solved correctly.
		std::vector<double> originalRHS2(rhs);
		jacobianMultiply(rhs, originalRHS);
		std::cout << "residuals" << std::endl;
		for (unsigned int i=0; i<n; ++i)
			std::cout << "  " << i << "   " << originalRHS[i]-originalRHS2[i]  << std::endl;
#endif // RESIDUAL_TEST

		// and add corrections to m_y
		double max_scale = 1;
#ifdef SCALE_DELTAY
		for (unsigned int i=0; i<n; ++i) {
			double y_next = m_y[i] + rhs[i];
			if (y_next < 0)
				max_scale = std::min(max_scale, (0.01-m_y[i])/(rhs[i]-1e-6) );
		}
#endif
		for (unsigned int i=0; i<n; ++i) {
			m_y[i] += max_scale*rhs[i];
		}
		std::cout << "deltaY (improved)" << std::endl;
		for (unsigned int i=0; i<n; ++i)
			std::cout << "  " << i << "   " << rhs[i]  << std::endl;

		// TODO : add alternative convergence criterion based on rhs norm
	}

	if (iterations > 0)
		std::cout << "\n*** converged" << std::endl;
	else
		std::cerr << "\n***** not converged *****" << std::endl;

	printVars();
}

void Network::jacobianInit() {

	unsigned int n = m_nodeCount + m_elementCount;
	// if sparse solver is  not available use dense matrix
	if(m_solverOptions == LESDense) {
		m_denseSolver.m_jacobian.resize(n);
		m_denseSolver.m_jacobianFactorized.resize(n);
	}
	else {

		IBKMK::SparseMatrixPattern pattern(n);
#ifdef PRESSURES_FIRST
		// nodal equations
		for (unsigned int i=0; i<m_nodeCount; ++i) {
			// now sum up all the mass fluxes in the nodes
			for (unsigned int j=0; j<m_nodes[i].m_flowElementIndexes.size(); ++j) {
				unsigned int feIndex = m_nodes[i].m_flowElementIndexes[j];
				// node is connected to all inflow and outflow elements
				if (!pattern.test(i, m_nodeCount + feIndex))
					pattern.set(i, m_nodeCount + feIndex);
			}
		}

		// nodal constraint to first node
		if (!pattern.test(0, 0))
			pattern.set(0, 0);

		// flow element equations
		for (unsigned int i=0; i<m_elementCount; ++i) {
			const AbstractFlowElement * fe = m_flowElements[i];
			// we need mass flux for pressure loss calculatiopn
			if (!pattern.test(m_nodeCount + i, m_nodeCount + i))
				pattern.set(m_nodeCount + i, m_nodeCount + i);
			// element is connected to inlet and outlet node
			if (!pattern.test(m_nodeCount + i, fe->m_nInlet))
				pattern.set(m_nodeCount + i, fe->m_nInlet);
			if (!pattern.test(m_nodeCount + i, fe->m_nOutlet))
				pattern.set(m_nodeCount + i, fe->m_nOutlet);
		}

#else
		// nodal equations
		for (unsigned int i=0; i<m_nodeCount; ++i) {
			// now sum up all the mass fluxes in the nodes
			for (unsigned int j=0; j<m_nodes[i].m_flowElementIndexes.size(); ++j) {
				unsigned int feIndex = m_nodes[i].m_flowElementIndexes[j];
				// node is connected to all inflow and outflow elements
				if (!pattern.test(i + m_elementCount, feIndex))
					pattern.set(i + m_elementCount, feIndex);
			}
		}

		// nodal constraint to first node
		if (!pattern.test(0 + m_elementCount, 0 + m_elementCount))
			pattern.set(0 + m_elementCount, 0 + m_elementCount);

		// flow element equations
		for (unsigned int i=0; i<m_elementCount; ++i) {
			const AbstractFlowElement * fe = m_flowElements[i];
			// we need mass flux for pressure loss calculatiopn
			if (!pattern.test(i, i))
				pattern.set(i, i);
			// element is connected to inlet and outlet node
			if (!pattern.test(i, fe->m_nInlet + m_elementCount))
				pattern.set(i, fe->m_nInlet + m_elementCount);
			if (!pattern.test(i, fe->m_nOutlet + m_elementCount))
				pattern.set(i, fe->m_nOutlet + m_elementCount);
		}

#endif

		// construct CSR pattern information
		std::vector<unsigned int> ia(n+1);
		std::vector<unsigned int> ja;

		for(unsigned int i = 0; i < n; ++i) {
			// get colors from pattern
			std::vector<unsigned int> cols;
			pattern.indexesPerRow(i, cols);
			// store indexes
			ja.insert(ja.end(), cols.begin(), cols.end());
			// store offset
			ia[i + 1] = (unsigned int) ja.size();
		}

		// generate transpose indes
		std::vector<unsigned int> iaT;
		std::vector<unsigned int> jaT;
		IBKMK::SparseMatrixCSR::generateTransposedIndex(ia, ja, iaT, jaT);
		// resize jacobian
		m_sparseSolver.m_jacobian.resize(n, ja.size(), &ia[0], &ja[0], &iaT[0], &jaT[0]);

		// vector to hold colors associated with individual columns
		std::vector<unsigned int> colarray(n, 0);
		// array to flag used colors
		std::vector<unsigned int> scols(n+1); // must have size = m_n+1 since valid color numbers start with 1

		// loop over all columns
		for (unsigned int i=0; i<n; ++i) {

			// clear vector with neighboring colors
			std::fill(scols.begin(), scols.end(), 0);

			// loop over all rows, that have have entries in this column
			// Note: this currently only works for symmetric matricies
			unsigned int j;
			for (unsigned int jind = iaT[i]; jind < iaT[i + 1]; ++jind) {
				// j always holds a valid row number
				j = jaT[jind];

				// search all columns in this row < column i and add their colors to our "used color set" scol
				unsigned int k;
				for (unsigned int kind = ia[j]; kind < ia[j + 1]; ++kind) {
					k = ja[kind];

					// k now holds column number in row j
					if (k >= i && kind != 0) break; // stop if this column is > our current column i
					// retrieve color of column and mark color as used
					scols[ colarray[k] ] = 1;
				}
			}
			// search lowest unused color
			unsigned int colIdx = 1;
			for (; colIdx < n; ++colIdx)
				if (scols[colIdx] == 0)
					break;
			//IBK_ASSERT(colIdx != m_n); /// \todo check this, might fail when dense matrix is being used!!!
			// set this color number in our colarray
			colarray[i] = colIdx;
			// store color index in colors array
			if (m_sparseSolver.m_jacobianColors.size() < colIdx)
				m_sparseSolver.m_jacobianColors.resize(colIdx);
			m_sparseSolver.m_jacobianColors[colIdx-1].push_back(i); // associate column number with color
		}

		// initialize KLU
		klu_defaults(&m_sparseSolver.m_KLUParas);
		// use COLAMD method for reduced fill-ordering
		m_sparseSolver.m_KLUParas.ordering = 1;
		// setup synmbolic matrix factorization
		m_sparseSolver.m_KLUSymbolic = klu_analyze(n, (int*) (&ia[0]),
						   (int*) (&ja[0]), &(m_sparseSolver.m_KLUParas));
	}
}


void Network::jacobianSetup() {

	unsigned int n = m_nodeCount + m_elementCount;
	std::vector<double> Gy(n);

	// store G(y)
	std::copy(m_G.begin(), m_G.end(), Gy.begin());

	if(m_denseSolver.m_jacobian.n() > 0) {

		IBKMK::DenseMatrix &jacobian = m_denseSolver.m_jacobian;
		IBKMK::DenseMatrix &jacobianFac = m_denseSolver.m_jacobianFactorized;
		// loop over all variables
		for (unsigned int j=0; j<n; ++j) {
			// modify y_j by a small EPS
			double eps = JACOBIAN_EPS;
			// for mass fluxes, if y > eps, rather subtract the eps
			if (j > m_nodeCount && m_y[j] > eps)
				eps = -JACOBIAN_EPS;
			m_y[j] += eps;
			// evaluate G(y_mod)
			updateG();
			// loop over all equations
			for (unsigned int i=0; i<n; ++i) {
				// now approximate dG_i/dy_j = [G_i(y_j+eps) - G_i(y_j)] / eps
				jacobian(i,j) = (m_G[i] - Gy[i])/eps;
			}
			// restore y
			m_y[j] -= eps;
		}
		// copy jacobian
		std::copy(jacobian.data().begin(), jacobian.data().end(),
				  jacobianFac.data().begin());
		// factorize matrix
		int res = jacobianFac.lu(); // Note: might be singular!!!
		if (res != 0)
			std::cerr << "Singular matrix" << std::endl;
	}
	// we use a sparse jacobian representation
	else if(m_sparseSolver.m_jacobian.nnz() > 0) {
		IBKMK::SparseMatrixCSR &jacobian = m_sparseSolver.m_jacobian;
		const std::vector<std::vector<unsigned int> > &colors = m_sparseSolver.m_jacobianColors;

		IBK_ASSERT(!colors.empty());

		const unsigned int * iaIdxT = jacobian.iaT();
		const unsigned int * jaIdxT = jacobian.jaT();

		// process all colors individually and modify y in groups
		for (unsigned int i=0; i<colors.size(); ++i) {  // i == color index

			// modify m_yMod[] in all columns marked by color i
			for (unsigned int jind=0; jind<colors[i].size(); ++jind) {
				unsigned int j = colors[i][jind];
				// modify y_j by a small EPS
				double eps = JACOBIAN_EPS;
				// for mass fluxes, if y > eps, rather subtract the eps
				if (j > m_nodeCount && m_y[j] > eps)
					eps = -JACOBIAN_EPS;
				m_y[j] += eps;
			}
			// evaluate G(y_mod)
			updateG();
			// compute Jacobian elements in groups
			for (unsigned int jind=0; jind<colors[i].size(); ++jind) {
				unsigned int j = colors[i][jind];
				// compute finite-differences column j in row i
				double eps = JACOBIAN_EPS;
				// for mass fluxes, if y > eps, rather subtract the eps
				if (j > m_nodeCount && m_y[j] > eps)
					eps = -JACOBIAN_EPS;
				// we compute now all Jacobian elements in the column j
				for (unsigned int k = iaIdxT[j]; k < iaIdxT[j + 1]; ++k) {
					unsigned int rowIdx = jaIdxT[k];
					// now approximate dG_i/dy_j = [G_i(y_j+eps) - G_i(y_j)] / eps
					jacobian(rowIdx,j) = (m_G[rowIdx] - Gy[rowIdx])/eps;
				} // for k

			} // for jind

			// modify m_yMod[] in all columns marked by color i
			for (unsigned int jind=0; jind<colors[i].size(); ++jind) {
				unsigned int j = colors[i][jind];
				// modify y_j by a small EPS
				double eps = JACOBIAN_EPS;
				// for mass fluxes, if y > eps, rather subtract the eps
				if (j > m_nodeCount && m_y[j] > eps)
					eps = -JACOBIAN_EPS;
				m_y[j] -= eps;
			}
		} // for i
		// calculate lu composition for klu object (creating a new pivit ordering)
		m_sparseSolver.m_KLUNumeric = klu_factor((int*) jacobian.ia(),
					(int*) jacobian.ja(),
					 jacobian.data(),
					 m_sparseSolver.m_KLUSymbolic,
					 &(m_sparseSolver.m_KLUParas));
	}
}

void Network::jacobianMultiply(const std::vector<double> &b, std::vector<double> &res) {

	if(m_denseSolver.m_jacobian.n() > 0)
		m_denseSolver.m_jacobian.multiply(&b[0], &res[0]);
	else if(m_sparseSolver.m_jacobian.nnz() > 0)
		m_sparseSolver.m_jacobian.multiply(&b[0], &res[0]);
}


void Network::jacobianBacksolve(std::vector<double> & rhs) {

	// decide which matrix to use
	if(m_denseSolver.m_jacobian.n() > 0) {
		m_denseSolver.m_jacobianFactorized.backsolve(&rhs[0]);
	}
	else if(m_sparseSolver.m_jacobian.nnz() > 0) {
		IBK_ASSERT(m_sparseSolver.m_KLUNumeric != nullptr);
		IBK_ASSERT(m_sparseSolver.m_KLUSymbolic != nullptr);
		unsigned int n = m_nodeCount + m_elementCount;
		/* Call KLU to solve the linear system */
		klu_tsolve(m_sparseSolver.m_KLUSymbolic,
				  m_sparseSolver.m_KLUNumeric,
				  (int) n, 1,
				  &rhs[0],
				  &(m_sparseSolver.m_KLUParas));
	}
}


void Network::jacobianWrite(std::vector<double> & rhs) {

	std::cout << "Jacobian:" << std::endl;

	if(m_denseSolver.m_jacobian.n() > 0)
		m_denseSolver.m_jacobian.write(std::cout, &rhs[0], false, 10);
	else if(m_sparseSolver.m_jacobian.nnz() > 0)
		m_sparseSolver.m_jacobian.write(std::cout, &rhs[0], false, 10);
}

#ifdef PRESSURES_FIRST

void Network::updateG() {

	// extract mass flows
	for (unsigned int i=0; i<m_elementCount; ++i) {
		m_massFluxes[i] = m_y[i+m_nodeCount]/MASS_FLUX_SCALE;
	}
	// first nodal equations
	for (unsigned int i=0; i<m_nodeCount; ++i) {
		m_nodePressures[i] = m_y[i];

		// now sum up all the mass fluxes in the nodes
		double massSum = 0;
		for (unsigned int j=0; j<m_nodes[i].m_flowElementIndexes.size(); ++j) {
			unsigned int feIndex = m_nodes[i].m_flowElementIndexes[j];
			const AbstractFlowElement * fe = m_flowElements[ feIndex ];
			// if the flow element is connected to the node via inlet, the mass goes from node to flow element
			// and hence has a negative sign
			if (fe->m_nInlet == i)
				massSum -= m_massFluxes[feIndex];
			else
				massSum += m_massFluxes[feIndex]; // otherwise flux goes into the node -> positive sign
		}
		// store in system function vector
		m_G[i] = massSum*MASS_FLUX_SCALE; // we'll apply scaling here

	}

	// nodal constraint to first node
	m_G[0] += m_nodePressures[0] - 0; // 0 Pa on first node

	// now evaluate the flow system equations
	for (unsigned int i=0; i<m_elementCount; ++i) {
		const AbstractFlowElement * fe = m_flowElements[i];
		m_G[m_nodeCount + i] = m_flowElements[i]->systemFunction( m_massFluxes[i], m_nodePressures[fe->m_nInlet], m_nodePressures[fe->m_nOutlet]);
	}

}

#else

void Network::updateG() {

	// extract mass flows
	for (unsigned int i=0; i<m_elementCount; ++i) {
		m_massFluxes[i] = m_y[i]/MASS_FLUX_SCALE;
	}
	// first nodal equations
	for (unsigned int i=0; i<m_nodeCount; ++i) {
		m_nodePressures[i] = m_y[i + m_elementCount];

		// now sum up all the mass fluxes in the nodes
		double massSum = 0;
		for (unsigned int j=0; j<m_nodes[i].m_flowElementIndexes.size(); ++j) {
			unsigned int feIndex = m_nodes[i].m_flowElementIndexes[j];
			const AbstractFlowElement * fe = m_flowElements[ feIndex ];
			// if the flow element is connected to the node via inlet, the mass goes from node to flow element
			// and hence has a negative sign
			if (fe->m_nInlet == i)
				massSum -= m_massFluxes[feIndex];
			else
				massSum += m_massFluxes[feIndex]; // otherwise flux goes into the node -> positive sign
		}
		// store in system function vector
		m_G[i + m_elementCount] = massSum*MASS_FLUX_SCALE; // we'll apply scaling here

	}

	// nodal constraint to first node
	m_G[0 + m_elementCount] += m_nodePressures[0] - 0; // 0 Pa on first node

	// now evaluate the flow system equations
	for (unsigned int i=0; i<m_elementCount; ++i) {
		const AbstractFlowElement * fe = m_flowElements[i];
		m_G[i] = m_flowElements[i]->systemFunction( m_massFluxes[i], m_nodePressures[fe->m_nInlet], m_nodePressures[fe->m_nOutlet]);
	}

}

#endif
