#include "Network.h"

#include <iostream>
#include <sstream>
#include <fstream>

#include "PipeElement.h"
#include "Pump.h"

const double JACOBIAN_EPS = 1e-6; // in Pa and scaled kg/s
const double THRESHOLD = 1;
const double MASS_FLUX_SCALE = 1;

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

	m_jacobian.resize(n);
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
	std::vector<double> Gy(n);

	// now start the Newton iteration
	int iterations = 100;
	while (--iterations > 0) {
		// evaluate system function for current guess
		updateG(m_y);

		// store RHS
		for (unsigned int i=0; i<m_G.size(); ++i)
			rhs[i] = -m_G[i];

		// compose right hand side (mind the minus sign)
		// and evaluate residuals
		double resNorm = WRMSNorm(m_G);
		std::cout << "\nres = " << resNorm << std::endl;
		if (resNorm < THRESHOLD)
			break;

		// store G(y)
		std::copy(m_G.begin(), m_G.end(), Gy.begin());

		// now compose Jacobian with FD quotients
		// loop over all variables
		for (unsigned int j=0; j<n; ++j) {
			// modify y_j by a small EPS
			double eps = JACOBIAN_EPS;
			// for mass fluxes, if y > eps, rather subtract the eps
			if (j > m_nodeCount && m_y[j] > eps)
				eps = -JACOBIAN_EPS;
			m_y[j] += eps;
			// evaluate G(y_mod)
			updateG(m_y);
			// loop over all equations
			for (unsigned int i=0; i<n; ++i) {
				// now approximate dG_i/dy_j = [G_i(y_j+eps) - G_i(y_j)] / eps
				m_jacobian(i,j) = (m_G[i] - Gy[i])/eps;
			}
			// restore y
			m_y[j] -= eps;
		}

		std::cout << "\n*** Iter " << 100-iterations  << std::endl;

		printVars();

		std::cout << "Jacobian:" << std::endl;
		m_jacobian.write(std::cout, &rhs[0], false, 10);

#ifdef RESIDUAL_TEST
		IBKMK::DenseMatrix mat(m_jacobian);
		std::vector<double> originalRHS(rhs);
#endif // RESIDUAL_TEST

		// factorize matrix
		int res = m_jacobian.lu(); // Note: might be singular!!!
		if (res != 0)
			std::cerr << "Singular matrix" << std::endl;
		// now solve the equation system
		m_jacobian.backsolve(&rhs[0]);

		std::cout << "deltaY" << std::endl;
		for (unsigned int i=0; i<n; ++i)
			std::cout << "  " << i << "   " << rhs[i]  << std::endl;

#ifdef RESIDUAL_TEST
		// check if equation system was solved correctly.
		std::vector<double> originalRHS2(rhs);
		mat.multiply(&rhs[0], &originalRHS2[0]);

		std::cout << "residuals" << std::endl;
		for (unsigned int i=0; i<n; ++i)
			std::cout << "  " << i << "   " << originalRHS[i]-originalRHS2[i]  << std::endl;
#endif // RESIDUAL_TEST

		// and add corrections to m_y
		double max_scale = 1;
#if SCALE_DELTAY
		for (unsigned int i=0; i<n; ++i) {
			double y_next = m_y[i] + rhs[i];
			if (y_next < 0)
				max_scale = std::min(max_scale, (0.01-m_y[i])/(rhs[i]-1e-6) );
		}
#endif
		for (unsigned int i=0; i<n; ++i) {
			m_y[i] += max_scale*rhs[i];
		}

		// TODO : add alternative convergence criterion based on rhs norm
	}

	if (iterations > 0)
		std::cout << "\n*** converged" << std::endl;
	else
		std::cerr << "\n***** not converged *****" << std::endl;

	printVars();
}

#ifdef PRESSURES_FIRST

void Network::updateG(const std::vector<double> & y) {

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

void Network::updateG(const std::vector<double> & y) {

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
