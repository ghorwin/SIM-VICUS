#include "NM_HydraulicNetworkModel.h"

#include <NANDRAD_HydraulicNetwork.h>
#include <NANDRAD_HydraulicNetworkComponent.h>
#include <NANDRAD_KeywordList.h>

#include <IBK_messages.h>

#include <IBKMK_DenseMatrix.h>
#include <IBKMK_SparseMatrixCSR.h>
#include <IBKMK_SparseMatrixPattern.h>

#include <klu.h>

#include "NM_HydraulicNetworkFlowElements.h"

namespace NANDRAD_MODEL {

// *** Pimpl class declaration ***

class HydraulicNetworkModelImpl {
public:
	HydraulicNetworkModelImpl(const std::vector<Element> &elems);
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

	/*! Container for flow element implementation objects.
		Need to be populated before calling setup.
	*/
	std::vector<HydraulicNetworkAbstractFlowElement*>	m_flowElements;
	/*! Network structure. */
	Network												m_network;
	/*! Mass fluxes through all elements*/
	std::vector<double>									m_massFluxes;

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
	/*! Vector with system function. */
	std::vector<double>					m_G;
};


// *** HydraulicNetworkModel members ***

HydraulicNetworkModel::HydraulicNetworkModel(const NANDRAD::HydraulicNetwork & nw,
	unsigned int id, const std::string &displayName) :
	m_id(id), m_displayName(displayName),m_hydraulicNetwork(&nw)
{
	// first register all nodes
	std::set<unsigned int> nodeIds;
	// for this purpose process all hydraulic network elements
	for (const NANDRAD::HydraulicNetworkElement & e : nw.m_elements) {
		nodeIds.insert(e.m_inletNodeId);
		nodeIds.insert(e.m_outletNodeId);
	}

	// now populate the m_flowElements vector of the network solver
	std::vector<Element> elems;
	// process all hydraulic network elements and copy index
	for (const NANDRAD::HydraulicNetworkElement & e : nw.m_elements) {
		unsigned int nInlet = std::distance(nodeIds.begin(), nodeIds.find(e.m_inletNodeId));
		unsigned int nOutlet = std::distance(nodeIds.begin(), nodeIds.find(e.m_outletNodeId));
		elems.push_back(Element(nInlet, nOutlet) );
	}

	// create implementation instance
	m_p = new HydraulicNetworkModelImpl(elems); // we take ownership

}

HydraulicNetworkModel::~HydraulicNetworkModel() {
	delete m_p; // delete pimpl object
}


const Network * HydraulicNetworkModel::network() const {
	return &m_p->m_network;
}

void HydraulicNetworkModel::setup() {
	FUNCID(HydraulicNetworkModel::setup);

	// now populate the m_flowElements vector of the network solver

	// process all hydraulic network elements and instatiate respective flow equation classes
	for (const NANDRAD::HydraulicNetworkElement & e : m_hydraulicNetwork->m_elements) {
		// each of the flow equation elements requires for calculation:
		// - instance-specific parameters from HydraulicNetworkElement e
		// - fluid property object from m_hydraulicNetwork->m_fluid
		// - component definition (via reference from e.m_componentId) and component DB stored
		//   in network

		// retrieve component - this is mandatory

		std::vector<NANDRAD::HydraulicNetworkComponent>::const_iterator it =
				std::find(m_hydraulicNetwork->m_components.begin(),
						  m_hydraulicNetwork->m_components.end(), e.m_componentId);
		if (it == m_hydraulicNetwork->m_components.end()) {
			throw IBK::Exception(IBK::FormatString("Missing component reference in hydraulic network element '%1' (id=%2).")
								 .arg(e.m_displayName).arg(e.m_id), FUNC_ID);
		}

		switch (it->m_modelType) {
			case NANDRAD::HydraulicNetworkComponent::MT_StaticPipe:
			case NANDRAD::HydraulicNetworkComponent::MT_StaticAdiabaticPipe :
			case NANDRAD::HydraulicNetworkComponent::MT_DynamicPipe :
			case NANDRAD::HydraulicNetworkComponent::MT_DynamicAdiabaticPipe :
			{
				// lookup pipe
				std::vector<NANDRAD::HydraulicNetworkPipeProperties>::const_iterator pipe_it =
						std::find(m_hydraulicNetwork->m_pipeProperties.begin(), m_hydraulicNetwork->m_pipeProperties.end(), e.m_pipeId);
				if (pipe_it == m_hydraulicNetwork->m_pipeProperties.end()) {
					throw IBK::Exception(IBK::FormatString("Missing pipe properties reference in hydraulic network element '%1' (id=%2).")
										 .arg(e.m_displayName).arg(e.m_id), FUNC_ID);
				}

				// create hydraulic pipe model
				HNPipeElement * pipeElement = new HNPipeElement(e, *it, *pipe_it, m_hydraulicNetwork->m_fluid);
				// add to flow elements
				m_p->m_flowElements.push_back(pipeElement); // transfer ownership
			} break;

			case NANDRAD::HydraulicNetworkComponent::MT_ConstantPressurePumpModel :
			{
				// create pump model
				HNConstantPressurePump * pumpElement = new HNConstantPressurePump(e, *it, m_hydraulicNetwork->m_fluid);
				// add to flow elements
				m_p->m_flowElements.push_back(pumpElement); // transfer ownership
			} break;

			case NANDRAD::HydraulicNetworkComponent::MT_HeatExchanger :
			{
				// create pump model
				HNFixedPressureLossCoeffElement * hxElement = new HNFixedPressureLossCoeffElement(e, *it, m_hydraulicNetwork->m_fluid);
				// add to flow elements
				m_p->m_flowElements.push_back(hxElement); // transfer ownership
			} break;
			case NANDRAD::HydraulicNetworkComponent::MT_HeatPump :
			case NANDRAD::HydraulicNetworkComponent::MT_GasBoiler :
			case NANDRAD::HydraulicNetworkComponent::MT_ControlValve :
			case NANDRAD::HydraulicNetworkComponent::MT_WaterStorage :
			case NANDRAD::HydraulicNetworkComponent::MT_ComponentConditionSystem :
			case NANDRAD::HydraulicNetworkComponent::MT_Radiator :
			case NANDRAD::HydraulicNetworkComponent::MT_Mixer :
			case NANDRAD::HydraulicNetworkComponent::MT_FMU : {
				throw IBK::Exception(IBK::FormatString("Model type '%1' for HydraulicNetworkComponent "
									 "with id %2 is still not supported")
									.arg(NANDRAD::KeywordList::Keyword(
									"HydraulicNetworkComponent::modelType_t",it->m_modelType))
									.arg(e.m_componentId),FUNC_ID);
			}
			default:{
				throw IBK::Exception(IBK::FormatString("Unsupported model type for "
									"HydraulicNetworkComponent with id %1!")
									.arg(e.m_componentId),FUNC_ID);
			}
		}
	}

	// set

	// setup the equation system
	try {
		m_p->setup();
	} catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, "Error setting up equation system/Jacobian for flow network.", FUNC_ID);
	}
}


void HydraulicNetworkModel::resultDescriptions(std::vector<QuantityDescription> & resDesc) const {
	if(!resDesc.empty())
		resDesc.clear();
	// mass flux vector is a result
	QuantityDescription desc("MassFlux", "kg/s", "Fluid mass flux trough all flow elements", false);
	// deactivate description;
	if(m_p->m_flowElements.empty())
		desc.m_size = 0;
	resDesc.push_back(desc);
}


void HydraulicNetworkModel::resultValueRefs(std::vector<const double *> & res) const {
	if(!res.empty())
		res.clear();
	// mass flux vector is a result quantity
	if(!m_p->m_massFluxes.empty())
		res.push_back(&m_p->m_massFluxes[0]);
}


const double * HydraulicNetworkModel::resultValueRef(const QuantityName & quantityName) const {
	// return vector of mass fluxes
	if(quantityName == std::string("MassFlux")) {
		if(!m_p->m_massFluxes.empty())
			return &m_p->m_massFluxes[0];
		return nullptr;
	}
	return nullptr;
}


void HydraulicNetworkModel::initInputReferences(const std::vector<AbstractModel *> & models) {
	// no inputs for now
}


void HydraulicNetworkModel::inputReferences(std::vector<InputReference> & inputRefs) const {
	// enforce access to internal fluid temperatures
	if(!inputRefs.empty())
		inputRefs.clear();
	// use hydraulic network model to generate mass flux references
	InputReference inputRef;
	inputRef.m_id = id();
	inputRef.m_referenceType = NANDRAD::ModelInputReference::MRT_NETWORK;
	inputRef.m_name = std::string("FluidTemperatures");
	inputRef.m_required = true;
	// register reference
	inputRefs.push_back(inputRef);

}


void HydraulicNetworkModel::setInputValueRefs(const std::vector<QuantityDescription> & resultDescriptions, const std::vector<const double *> & resultValueRefs) {
	IBK_ASSERT(resultValueRefs.size() == 1);
	// copy references into mass flux vector
	m_fluidTemperatures = resultValueRefs[0];
}


void HydraulicNetworkModel::stateDependencies(std::vector<std::pair<const double *, const double *> > & resultInputValueReferences) const {
	// no state dependencies for now
}


int HydraulicNetworkModel::update() {
	FUNCID(HydraulicNetworkModel::update);
	// set all fluid temperatures
	unsigned int offset = 0;
	for(HydraulicNetworkAbstractFlowElement *fe : m_p->m_flowElements) {
		fe->setFluidTemperature(m_fluidTemperatures + offset);
		offset += fe->nInternalStates();
	}

	// re-compute hydraulic network

	IBK_ASSERT(m_p != nullptr);
	try {
		// TODO : check input ref values vs. old input ref values - no change, no recomputation needed
		int res = m_p->solve();
		// signal an error
		if(res != 0)
			return res;

		// TODO : add support for return values (e.g. recoverable convergence errors)
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex,
							 IBK::FormatString("Error solving hydraulic network equations for network #%1 '%2'.")
							 .arg(m_id).arg(m_displayName), FUNC_ID);

	}

	return 0; // signal success
}


// *** HydraulicNetworkModelImpl members ***


const double JACOBIAN_EPS = 1e-6; // in Pa and scaled kg/s
const double THRESHOLD = 1;
const double MASS_FLUX_SCALE = 1000;


HydraulicNetworkModelImpl::HydraulicNetworkModelImpl(const std::vector<Element> &elems)
{
	// copy elements vector
	m_network.m_elements = elems;
	// count number of nodes
	unsigned int nodeCount = 0;
	for (const Element &fe :elems) {
		nodeCount = std::max(nodeCount, fe.m_nInlet);
		nodeCount = std::max(nodeCount, fe.m_nOutlet);
	}

	// create fast access connections between nodes and flow elements
	m_network.m_nodes.resize(nodeCount+1);
	for (unsigned int i=0; i<elems.size(); ++i) {
		const Element &fe = elems[i];
		// TODO : check inlet must be different from outlet
		m_network.m_nodes[fe.m_nInlet].m_elementIndexesOutlet.push_back(i);
		m_network.m_nodes[fe.m_nOutlet].m_elementIndexesInlet.push_back(i);
		m_network.m_nodes[fe.m_nInlet].m_elementIndexes.push_back(i);
		m_network.m_nodes[fe.m_nOutlet].m_elementIndexes.push_back(i);
	}

#if 0
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
	for (HydraulicNetworkAbstractFlowElement * fe : m_flowElements) {
		nodeCount = std::max(nodeCount, fe->m_nInlet);
		nodeCount = std::max(nodeCount, fe->m_nOutlet);
	}

	writeNetworkGraph();

	m_nodes.resize(nodeCount+1);
	for (unsigned int i=0; i<m_flowElements.size(); ++i) {
		HydraulicNetworkAbstractFlowElement * fe = m_flowElements[i];
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
#endif

}

HydraulicNetworkModelImpl::~HydraulicNetworkModelImpl() {
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


void HydraulicNetworkModelImpl::setup() {
	FUNCID(HydraulicNetworkModelImpl::setup);


	// error checks:
	// 1.) no open ends
	// -> all m_nodes[i] must have at least 2 m_flowElementIndexes
	// 2.) no single cycles:
	// -> inlet must be different from outlet
	for (unsigned int i=0; i<m_network.m_nodes.size(); ++i) {
		const Node &node = m_network.m_nodes[i];
		// error check 1
		if(node.m_elementIndexes.size() == 1){
			throw IBK::Exception(IBK::FormatString(
					"FlowElement with id %1 is an open end of hydraulic network!")
					 .arg(node.m_elementIndexes[0]).str(),
					FUNC_ID);
		}
		// error check 2
		std::set<unsigned int> indexes;
		for(unsigned int j = 0; j < node.m_elementIndexes.size(); ++j) {
			unsigned int elementIdx = node.m_elementIndexes[j];
			if(indexes.find(elementIdx) != indexes.end()){
				throw IBK::Exception(IBK::FormatString(
						"FlowElement with id %1 is an invalid cyclic connection!")
						 .arg(elementIdx).str(),
						FUNC_ID);
			}
		}
	}


	// 3.) no distinct networks
	// -> each node must connect to any other
	// -> transitive closure of connectivity must form a dense matrix
	IBKMK::SparseMatrixPattern connectivity(m_network.m_nodes.size());
#ifdef BIDIRECTIONAL

	for (unsigned int k=0; k<m_network.m_elements.size(); ++k) {
		const Element &fe = m_network.m_elements[k];

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
	for(unsigned int k = 0; k < m_network.m_nodes.size(); ++k) {
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
	IBKMK::SparseMatrixPattern connectivityTranspose(m_network.m_nodes.size());

	for (unsigned int k=0; k<m_network.m_elements.size(); ++k) {
		const Element &fe = m_network.m_elements[k];

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
	for(unsigned int k = 0; k < m_network.m_nodes.size(); ++k) {
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
	for(unsigned int i = 0; i < m_network.m_nodes.size(); ++i) {
		// count column entries for each row
		std::vector<unsigned int> cols;
		connectivity.indexesPerRow(i,cols);

		// error: missing connections
		if(cols.size() != m_network.m_nodes.size()) {
			// isolated nodes are not allowed
			IBK_ASSERT(!cols.empty());

			// find out disjunct network elements
			std::vector<unsigned int> disjunctElements;
			for(unsigned int j = 0; j < cols.size(); ++j) {
				const Node &node = m_network.m_nodes[cols[j]];

				for(unsigned int k =0; k < node.m_elementIndexes.size(); ++k)
					disjunctElements.push_back(node.m_elementIndexes[k]);
			}

			// create an error message string
			IBK_ASSERT(!disjunctElements.empty());
			std::string networkStr(IBK::val2string<unsigned int>(disjunctElements[0]));

			for(unsigned int k = 1; k < disjunctElements.size(); ++k)
				networkStr += std::string(",") + IBK::val2string<unsigned int>(disjunctElements[k]);

			throw IBK::Exception(IBK::FormatString(
					"Network is not completely connected! Distinct network formed by flow elements (%1)!")
					.arg(networkStr).str(),
					FUNC_ID);
		}
	}
	// count number of nodes
	m_nodeCount = m_network.m_nodes.size();
	m_elementCount = m_network.m_elements.size();
	IBK::IBK_Message(IBK::FormatString("Nodes:         %1\n").arg(m_nodeCount), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::IBK_Message(IBK::FormatString("Flow elements: %1\n").arg(m_elementCount), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	unsigned int n = m_nodeCount + m_elementCount;

	// set initial conditions
	m_y.resize(n, 10);

	m_G.resize(n);
	m_massFluxes.resize(m_elementCount);
	m_nodalPressures.resize(m_nodeCount);

	// create jacobian
	jacobianInit();
}


double WRMSNorm(const std::vector<double> & vec) {
	double resNorm = 0;
	for (unsigned int i=0; i<vec.size(); ++i)
		resNorm += vec[i]*vec[i];
	resNorm /= vec.size();
	resNorm = std::sqrt(resNorm);
	return resNorm;
}


void HydraulicNetworkModelImpl::printVars() const {
	std::cout << "Mass fluxes [kg/s]" << std::endl;
	for (unsigned int i=0; i<m_elementCount; ++i)
		std::cout << "  " << i << "   " << m_y[i]/MASS_FLUX_SCALE  << std::endl;

	std::cout << "Nodal pressures [Pa]" << std::endl;
	for (unsigned int i=0; i<m_nodeCount; ++i)
		std::cout << "  " << i << "   " << m_y[i + m_elementCount] << std::endl;
}


void HydraulicNetworkModelImpl::writeNetworkGraph() const {
#if 0
	// generate dot graph file for plotting
	std::stringstream strm;
	strm << "digraph {\n";
	for (HydraulicNetworkAbstractFlowElement * fe : m_flowElements) {
		strm << "  " << fe->m_nInlet+1 << " -> " << fe->m_nOutlet+1;
		if (dynamic_cast<Pump*>(fe) != nullptr) {
			strm << "[fontsize=7, label=\"pump\", weight=200, color=red]";
		}
		strm << ";\n";
	}

	strm << "}\n";

	std::ofstream out("graph.gv");
	out << strm.rdbuf();
#endif
}


int HydraulicNetworkModelImpl::solve() {
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
//		std::cout << "res = " << resNorm << std::endl;
		if (resNorm < THRESHOLD)
			break;

		// now compose Jacobian with FD quotients

		// perform jacobian update
		int res = jacobianSetup();
		// error signaled:
		// may be result of a diverging Newton iteration
		// -> regsiter a recoverable error and allow a retry
		if(res != 0)
			return 1;

//		std::cout << "\n\n*** Iter " << 100-iterations  << std::endl;

//		printVars();
//		jacobianWrite(rhs);

#ifdef RESIDUAL_TEST
		std::vector<double> originalRHS(rhs);
#endif // RESIDUAL_TEST

		// now solve the equation system
		res = jacobianBacksolve(rhs);
		// backsolving problems imply coarse structural errors
		if(res != 0)
			return 2;

//		std::cout << "deltaY" << std::endl;
//		for (unsigned int i=0; i<n; ++i)
//			std::cout << "  " << i << "   " << rhs[i]  << std::endl;

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
//		std::cout << "deltaY (improved)" << std::endl;
//		for (unsigned int i=0; i<n; ++i)
//			std::cout << "  " << i << "   " << rhs[i]  << std::endl;

		// TODO : add alternative convergence criterion based on rhs norm
	}

	printVars();

	if (iterations > 0)
		return 0;
	// we register a recoverable error if the system did not converge
	// (and allow a retry with a new guess)
	else
		return 1;

}

void HydraulicNetworkModelImpl::jacobianInit() {

	unsigned int n = m_nodeCount + m_elementCount;
	// if sparse solver is not available use dense matrix
	if (m_solverOptions == LESDense) {
		m_denseSolver.m_jacobian.resize(n);
		m_denseSolver.m_jacobianFactorized.resize(n);
	}
	else {

		IBKMK::SparseMatrixPattern pattern(n);
#ifdef PRESSURES_FIRST
		// nodal equations
		for (unsigned int i=0; i<m_nodeCount; ++i) {
			// now sum up all the mass fluxes in the nodes
			for (unsigned int j=0; j<m_network->m_nodes[i].m_elementIndexes.size(); ++j) {
				unsigned int feIndex = m_network->m_nodes[i].m_elementIndexes[j];
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
			const HydraulicNetworkElement &fe = m_network->m_elements[i];
			// we need mass flux for pressure loss calculatiopn
			if (!pattern.test(m_nodeCount + i, m_nodeCount + i))
				pattern.set(m_nodeCount + i, m_nodeCount + i);
			// element is connected to inlet and outlet node
			if (!pattern.test(m_nodeCount + i, fe.m_nInlet))
				pattern.set(m_nodeCount + i, fe.m_nInlet);
			if (!pattern.test(m_nodeCount + i, fe.m_nOutlet))
				pattern.set(m_nodeCount + i, fe.m_nOutlet);
		}

#else
		// nodal equations
		for (unsigned int i=0; i<m_nodeCount; ++i) {
			// now sum up all the mass fluxes in the nodes
			for (unsigned int j=0; j<m_network.m_nodes[i].m_elementIndexes.size(); ++j) {
				unsigned int feIndex = m_network.m_nodes[i].m_elementIndexes[j];
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
			const Element &fe = m_network.m_elements[i];
			// we need mass flux for pressure loss calculatiopn
			if (!pattern.test(i, i))
				pattern.set(i, i);
			// element is connected to inlet and outlet node
			if (!pattern.test(i, fe.m_nInlet + m_elementCount))
				pattern.set(i, fe.m_nInlet + m_elementCount);
			if (!pattern.test(i, fe.m_nOutlet + m_elementCount))
				pattern.set(i, fe.m_nOutlet + m_elementCount);
		}

#endif

		// construct CSR pattern information
		std::vector<unsigned int> ia(n+1);
		std::vector<unsigned int> ja;

		for(unsigned int i = 0; i < n; ++i) {
			// get colors from pattern
			std::vector<unsigned int> cols;
			pattern.indexesPerRow(i, cols);
			IBK_ASSERT(!cols.empty());
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
		// error may only occur if a wrong network topology was tolerated
		IBK_ASSERT(m_sparseSolver.m_KLUSymbolic != nullptr);
	}
}


int HydraulicNetworkModelImpl::jacobianSetup() {

	unsigned int n = m_nodeCount + m_elementCount;
	std::vector<double> Gy(n);

	// store G(y)
	std::copy(m_G.begin(), m_G.end(), Gy.begin());

	if (m_denseSolver.m_jacobian.n() > 0) {

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
		// singular
		if( res != 0)
			return 1;
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
		// error treatment: singular matrix
		if(m_sparseSolver.m_KLUNumeric == nullptr)
			return 1;
	}
	return 0;
}

void HydraulicNetworkModelImpl::jacobianMultiply(const std::vector<double> &b, std::vector<double> &res) {

	if(m_denseSolver.m_jacobian.n() > 0)
		m_denseSolver.m_jacobian.multiply(&b[0], &res[0]);
	else if(m_sparseSolver.m_jacobian.nnz() > 0)
		m_sparseSolver.m_jacobian.multiply(&b[0], &res[0]);
}


int HydraulicNetworkModelImpl::jacobianBacksolve(std::vector<double> & rhs) {

	// decide which matrix to use
	if(m_denseSolver.m_jacobian.n() > 0) {
		m_denseSolver.m_jacobianFactorized.backsolve(&rhs[0]);
	}
	else if(m_sparseSolver.m_jacobian.nnz() > 0) {
		IBK_ASSERT(m_sparseSolver.m_KLUNumeric != nullptr);
		IBK_ASSERT(m_sparseSolver.m_KLUSymbolic != nullptr);
		unsigned int n = m_nodeCount + m_elementCount;
		/* Call KLU to solve the linear system */
		int res = klu_tsolve(m_sparseSolver.m_KLUSymbolic,
				  m_sparseSolver.m_KLUNumeric,
				  (int) n, 1,
				  &rhs[0],
				  &(m_sparseSolver.m_KLUParas));
		// an error occured
		if(res == 0)
			return 1;
	}
	return 0;
}


void HydraulicNetworkModelImpl::jacobianWrite(std::vector<double> & rhs) {

	std::cout << "Jacobian:" << std::endl;

	if(m_denseSolver.m_jacobian.n() > 0)
		m_denseSolver.m_jacobian.write(std::cout, &rhs[0], false, 10);
	else if(m_sparseSolver.m_jacobian.nnz() > 0)
		m_sparseSolver.m_jacobian.write(std::cout, &rhs[0], false, 10);
}

#ifdef PRESSURES_FIRST

void HydraulicNetworkModelImpl::updateG() {

	// extract mass flows
	for (unsigned int i=0; i<m_elementCount; ++i) {
		m_massFluxes[i] = m_y[i+m_nodeCount]/MASS_FLUX_SCALE;
	}
	// first nodal equations
	for (unsigned int i=0; i<m_nodeCount; ++i) {
		m_nodePressures[i] = m_y[i];
		// set pressure of all inlets and outlets
		for(unsigned int idx : m_network.m_nodes[i].m_elementIndexesInlet) {
			if(m_massFluxes[idx] < 0) {
				m_outletPressures[idx] = m_nodalPressures[i];
			}
		}
		for(unsigned int idx : m_network.m_nodes[i].m_elementIndexesOutlet) {
			if(m_massFluxes[idx] >= 0) {
				m_outletPressures[idx] = m_nodalPressures[i];
			}
		}

		// now sum up all the mass fluxes in the nodes
		double massSum = 0;
		for (unsigned int j=0; j<m_network.m_nodes[i].m_elementIndexes.size(); ++j) {
			unsigned int feIndex = m_network.m_nodes[i].m_elementIndexes[j];
			const Element &fe = m_network.m_elements[ feIndex ];
			// if the flow element is connected to the node via inlet, the mass goes from node to flow element
			// and hence has a negative sign
			if (fe.m_nInlet == i)
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
		const Element &fe = m_network.m_elements[i];
		m_G[m_nodeCount + i] = m_flowElements[i]->systemFunction( m_massFluxes[i], m_nodePressures[fe->m_nInlet], m_nodePressures[fe->m_nOutlet]);
	}

}

#else

void HydraulicNetworkModelImpl::updateG() {

	// extract mass flows
	for (unsigned int i=0; i<m_elementCount; ++i) {
		m_massFluxes[i] = m_y[i]/MASS_FLUX_SCALE;
	}
	// first nodal equations
	for (unsigned int i=0; i<m_nodeCount; ++i) {
		m_nodalPressures[i] = m_y[i + m_elementCount];

		// now sum up all the mass fluxes in the nodes
		double massSum = 0;
		for (unsigned int j=0; j<m_network.m_nodes[i].m_elementIndexes.size(); ++j) {
			unsigned int feIndex = m_network.m_nodes[i].m_elementIndexes[j];
			const Element &fe = m_network.m_elements[ feIndex ];
			// if the flow element is connected to the node via inlet, the mass goes from node to flow element
			// and hence has a negative sign
			if (fe.m_nInlet == i)
				massSum -= m_massFluxes[feIndex];
			else
				massSum += m_massFluxes[feIndex]; // otherwise flux goes into the node -> positive sign
		}
		// store in system function vector
		m_G[i + m_elementCount] = massSum*MASS_FLUX_SCALE; // we'll apply scaling here

	}

	// nodal constraint to first node
	m_G[0 + m_elementCount] += m_nodalPressures[0] - 0; // 0 Pa on first node

	// now evaluate the flow system equations
	for (unsigned int i=0; i<m_elementCount; ++i) {
		const Element &fe = m_network.m_elements[i];
		m_G[i] = m_flowElements[i]->systemFunction( m_massFluxes[i], m_nodalPressures[fe.m_nInlet], m_nodalPressures[fe.m_nOutlet]);
	}

}

#endif

} // namespace NANDRAD
