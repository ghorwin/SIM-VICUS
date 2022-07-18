#ifndef VICUS_POLYLINEH
#define VICUS_POLYLINEH

#include "VICUS_NetworkLine.h"

#include <IBKMK_Vector3D.h>

namespace VICUS {

class PolyLine
{

public:
	PolyLine();

	PolyLine(const std::vector<IBKMK::Vector3D> & newVertexes);

	const std::vector<IBKMK::Vector3D> & vertexes() const;

	void setVertexes(const std::vector<IBKMK::Vector3D> & newVertexes);

	void addVertex(const IBKMK::Vector3D & newVertex);

	void updateLines() const;

	const std::vector<VICUS::NetworkLine>& lines() const;

	bool isValid() const;

private:

	std::vector<IBKMK::Vector3D>	m_vertexes;

	mutable std::vector<VICUS::NetworkLine> m_lines;
};


} // namespace VICUS

#endif // VICUS_POLYLINEH
