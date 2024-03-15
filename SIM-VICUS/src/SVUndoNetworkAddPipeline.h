#ifndef SVUNDOADDPIPELINEH
#define SVUNDOADDPIPELINEH

#include "SVUndoCommandBase.h"

#include <IBKMK_Vector3D.h>

#include <VICUS_Constants.h>
#include <VICUS_NetworkPipe.h>
#include <VICUS_Network.h>

class SVUndoNetworkAddPipeline: public SVUndoCommandBase {
	Q_DECLARE_TR_FUNCTIONS(SVUndoNetworkAddPipeline)

public:
	SVUndoNetworkAddPipeline(const std::vector<IBKMK::Vector3D> &polyLine, unsigned int pipeId, unsigned int networkId, bool findIntersections);

	virtual void undo() override;
	virtual void redo() override;

private:
	/*! The coordinates of the edges to be added */
	std::vector<IBKMK::Vector3D>	m_polyLine;
	/*! The pipe id of the edges to be added */
	unsigned int					m_pipeId;
	/*! The network the edges should be added to */
	unsigned int					m_networkId = VICUS::INVALID_ID;
	/*! The original network before modification. */
	VICUS::Network					m_previousNetwork;
	/*! Automatically insert intersections?.*/
	bool							m_findIntersections;
};

#endif // SVUNDOADDPIPELINEH
