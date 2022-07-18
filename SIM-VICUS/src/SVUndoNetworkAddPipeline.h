#ifndef SVUNDOADDPIPELINEH
#define SVUNDOADDPIPELINEH

#include "SVUndoCommandBase.h"

#include <IBKMK_Vector3D.h>

#include <VICUS_Constants.h>
#include <VICUS_NetworkPipe.h>

class SVUndoNetworkAddPipeline: public SVUndoCommandBase {
	Q_DECLARE_TR_FUNCTIONS(SVUndoNetworkAddPipeline)

public:
	SVUndoNetworkAddPipeline(const std::vector<IBKMK::Vector3D> &polyLine, unsigned int pipeId, unsigned int networkId);

	virtual void undo() override;
	virtual void redo() override;

private:
	/*! The coordinates of the edges to be added */
	std::vector<IBKMK::Vector3D>	m_polyLine;
	/*! The pipe id of the edges to be added */
	unsigned int					m_pipeId;
	/*! The network the edges should be added to */
	unsigned int					m_networkId = VICUS::INVALID_ID;
	/*! Stores the ids of added edges */
	std::vector<unsigned int>		m_addedEdgeIds;
	/*! Stores the ids of added nodes */
	std::vector<unsigned int>		m_addedNodeIds;

};

#endif // SVUNDOADDPIPELINEH
