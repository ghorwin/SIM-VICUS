#ifndef SVUNDONETWORKADDNODESH
#define SVUNDONETWORKADDNODESH

#include <SVUndoCommandBase.h>

#include "IBKMK_Vector3D.h"

#include "VICUS_NetworkNode.h"


class SVUndoNetworkAddNodes: public SVUndoCommandBase {
	Q_DECLARE_TR_FUNCTIONS(SVUndoNetworkAddNodes)

public:
	SVUndoNetworkAddNodes(const std::vector<VICUS::NetworkNode> &nodes, unsigned int networkId);

	virtual void undo() override;
	virtual void redo() override;

private:
	/*! The coordinates of the edges to be added */
	std::vector<VICUS::NetworkNode>	m_nodes;
	/*! The network the edges should be added to */
	unsigned int					m_networkId = VICUS::INVALID_ID;
	/*! Stores the ids of added nodes */
	std::vector<unsigned int>		m_addedNodeIds;

};
#endif // SVUNDONETWORKADDNODESH
