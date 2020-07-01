#include "DATAIO_ConstructionLines2D.h"

#include <IBK_assert.h>
#include <IBK_UnitList.h>

#include "DATAIO_GeoFile.h"

namespace DATAIO {

ConstructionLines2D::ConstructionLines2D(const IBK::Unit& unit) :
	m_xunit(unit),
	m_yunit(unit)
{
	IBK_ASSERT(unit.base_id() == IBK::Unit(1).base_id());
}

void ConstructionLines2D::addVerticalLine(double xpos, double ystart, double yend, bool isBoundary) {
	if(isBoundary)
		m_verticalBoundaryLines.push_back(ConstructionLine2D(xpos, ystart, yend));
	else
		m_verticalConstructionLines.push_back(ConstructionLine2D(xpos, ystart, yend));
}

void ConstructionLines2D::addHorizontalLine(double ypos, double xstart, double xend, bool isBoundary) {
	if(isBoundary)
		m_horizontalBoundaryLines.push_back(ConstructionLine2D(ypos, xstart, xend));
	else
		m_horizontalConstructionLines.push_back(ConstructionLine2D(ypos, xstart, xend));
}

void ConstructionLines2D::clearVertical() {
	m_verticalConstructionLines.clear();
	m_verticalBoundaryLines.clear();
}

void ConstructionLines2D::clearHorizontal() {
	m_horizontalConstructionLines.clear();
	m_horizontalBoundaryLines.clear();
}

void ConstructionLines2D::clear() {
	clearVertical();
	clearHorizontal() ;
}

bool ConstructionLines2D::emptyVertical() const {
	return m_verticalConstructionLines.empty() && m_verticalBoundaryLines.empty();
}

bool ConstructionLines2D::emptyHorizontal() const {
	return m_horizontalConstructionLines.empty() && m_horizontalBoundaryLines.empty();
}

bool ConstructionLines2D::empty() const {
	return emptyVertical() && emptyHorizontal();
}

void ConstructionLines2D::convertUnit(const IBK::Unit& newXUnit, const IBK::Unit& newYUnit) {
	if (newXUnit == m_xunit && newYUnit == m_yunit)
		return;

	if (newXUnit.id() != 0 && newXUnit != m_xunit) {

		// only dimension is allowed
		IBK_ASSERT( newXUnit.base_id() == IBK_UNIT_ID_METERS);

		double xfact;
		unsigned int xop;
		m_xunit.relate_to(newXUnit, xfact, xop);

		std::vector<ConstructionLine2D>::iterator endIt = m_verticalConstructionLines.end();
		for( std::vector<ConstructionLine2D>::iterator it=m_verticalConstructionLines.begin(); it!=endIt; ++it) {
			it->m_pos *= xfact;
		}

		endIt = m_verticalBoundaryLines.end();
		for( std::vector<ConstructionLine2D>::iterator it=m_verticalBoundaryLines.begin(); it!=endIt; ++it) {
			it->m_pos *= xfact;
		}

		endIt = m_horizontalConstructionLines.end();
		for( std::vector<ConstructionLine2D>::iterator it=m_horizontalConstructionLines.begin(); it!=endIt; ++it) {
			it->m_begin *= xfact;
			it->m_end *= xfact;
		}

		endIt = m_horizontalBoundaryLines.end();
		for( std::vector<ConstructionLine2D>::iterator it=m_horizontalBoundaryLines.begin(); it!=endIt; ++it) {
			it->m_begin *= xfact;
			it->m_end *= xfact;
		}
		m_xunit = newXUnit;
	}

	if (newYUnit.id() != 0 &&  newYUnit != m_yunit) {
		// only dimension is allowed
		IBK_ASSERT( newYUnit.base_id() == IBK_UNIT_ID_METERS);

		double yfact;
		unsigned int yop;
		m_yunit.relate_to(newYUnit, yfact, yop);

		std::vector<ConstructionLine2D>::iterator endIt = m_verticalConstructionLines.end();
		for( std::vector<ConstructionLine2D>::iterator it=m_verticalConstructionLines.begin(); it!=endIt; ++it) {
			it->m_begin *= yfact;
			it->m_end *= yfact;
		}

		endIt = m_verticalBoundaryLines.end();
		for( std::vector<ConstructionLine2D>::iterator it=m_verticalBoundaryLines.begin(); it!=endIt; ++it) {
			it->m_begin *= yfact;
			it->m_end *= yfact;
		}

		endIt = m_horizontalConstructionLines.end();
		for( std::vector<ConstructionLine2D>::iterator it=m_horizontalConstructionLines.begin(); it!=endIt; ++it) {
			it->m_pos *= yfact;
		}

		endIt = m_horizontalBoundaryLines.end();
		for( std::vector<ConstructionLine2D>::iterator it=m_horizontalBoundaryLines.begin(); it!=endIt; ++it) {
			it->m_pos *= yfact;
		}
		m_yunit = newYUnit;
	}

}

void ConstructionLines2D::swapLineDirection() {
	m_horizontalConstructionLines.swap(m_verticalConstructionLines);
	m_horizontalBoundaryLines.swap(m_verticalBoundaryLines);
}

bool ConstructionLines2D::notEqual(const ConstructionLines2D& rhs) const {
	// first test sizes to make it faster
	if(m_verticalConstructionLines.size() != rhs.m_verticalConstructionLines.size())
		return true;
	if(m_verticalBoundaryLines.size() != rhs.m_verticalBoundaryLines.size())
		return true;
	if(m_horizontalConstructionLines.size() != rhs.m_horizontalConstructionLines.size())
		return true;
	if(m_horizontalBoundaryLines.size() != rhs.m_horizontalBoundaryLines.size())
		return true;

	// now test mor detailed
	if(m_verticalConstructionLines != rhs.m_verticalConstructionLines)
		return true;
	if(m_verticalBoundaryLines != rhs.m_verticalBoundaryLines)
		return true;
	if(m_horizontalConstructionLines != rhs.m_horizontalConstructionLines)
		return true;
	if(m_horizontalBoundaryLines != rhs.m_horizontalBoundaryLines)
		return true;

	return false;
}

/*! Checks if there is an element at coordinates i,j and if it has a different material assigned.
	Helper function to compose construction lines vectors.
	\return Returns 0, if both elements have the same material. Returns 1, if neighboring elements
		exist and has different material. Returns 2, if neighboring elements does not exist (void).
*/
static int gridLineType(unsigned int ni, unsigned int nj, unsigned int myMatNr, const IBK::matrix<int>& elementMatrix, const std::vector<GeoFile::Element> & elementsVec) {
	int neighborN = elementMatrix(ni, nj); // mind operator()(col, row)
	if (neighborN == -1)
		return 2;
	if (elementsVec[static_cast<size_t>(neighborN)].matnr != myMatNr)
		return 1;
	else
		return 0;
}

void ConstructionLines2D::generate(const GeoFile & geofile) {
	// create element matrix
	IBK::matrix<int> elementMatrix;
	elementMatrix.resize((unsigned int)geofile.m_grid.x_coords.size(), (unsigned int)geofile.m_grid.y_coords.size());
	elementMatrix.fill(-1);

	for (unsigned int e=0; e<geofile.m_elementsVec.size(); ++e) {
		const GeoFile::Element & elem = geofile.m_elementsVec[e];
		if ( elem.n != IBK::INVALID_ELEMENT)
			elementMatrix(elem.i, elem.j) = (int)e;
	}

	// Internal map for horizontal material boundary lines.
	// Coordinates correspond to x-column index, and y-gridline index.
	// Value identifies type of gridline: 0 - no grid line, 1 - material-material line, 2-material-void border
	IBK::matrix<int>								horizontalConstructionLineGrid;
	// Internal map for horizontal material boundary lines.
	// Coordinates correspond to x-gridline index, and y-row index (from bottom to top).
	// Value identifies type of gridline: 0 - no grid line, 1 - material-material line, 2-material-void border
	IBK::matrix<int>								verticalConstructionLineGrid;
	horizontalConstructionLineGrid.resize(elementMatrix.cols(), elementMatrix.rows()+1);
	horizontalConstructionLineGrid.fill(0);
	verticalConstructionLineGrid.resize(elementMatrix.cols()+1, elementMatrix.rows());
	verticalConstructionLineGrid.fill(0);

	for (unsigned int e=0; e<geofile.m_elementsVec.size(); ++e) {
		const GeoFile::Element & elem = geofile.m_elementsVec[e];
		if ( elem.n == IBK::INVALID_ELEMENT )
			continue;
		int e_i = elem.i;
		int e_j = elem.j;
		// vertical left line: index e_i, e_j
		if (e_i == 0)					verticalConstructionLineGrid(e_i, e_j) = 2;
		else							verticalConstructionLineGrid(e_i, e_j) = gridLineType(e_i-1, e_j, elem.matnr, elementMatrix, geofile.m_elementsVec);

		// vertical right line: index e_i+1, e_j
		if (e_i == (int)elementMatrix.cols() - 1)	verticalConstructionLineGrid(e_i+1, e_j) = 2;
		else							verticalConstructionLineGrid(e_i+1, e_j) = gridLineType(e_i+1, e_j, elem.matnr, elementMatrix, geofile.m_elementsVec);

		// horizontal bottom line: index e_i, e_j
		if (e_j == 0)					horizontalConstructionLineGrid(e_i, e_j) = 2;
		else							horizontalConstructionLineGrid(e_i, e_j) = gridLineType(e_i, e_j-1, elem.matnr, elementMatrix, geofile.m_elementsVec);

		// horizontal top line: index e_i, e_j+1
		if (e_j == (int)elementMatrix.rows() - 1)	horizontalConstructionLineGrid(e_i, e_j+1) = 2;
		else							horizontalConstructionLineGrid(e_i, e_j+1) = gridLineType(e_i, e_j+1, elem.matnr, elementMatrix, geofile.m_elementsVec);
	}

	clear();

	// now loop through all lines and store consecutive lines into vectors

	for (unsigned int j=0; j<horizontalConstructionLineGrid.rows(); ++j) {
		int p1 = -1; // initialize with -1, so that we know when to start
		int p2 = -1;
		int lastType = 0;
		// process all segments in row
		for (unsigned int i=0; i<horizontalConstructionLineGrid.cols(); ++i) {
			int currentType = horizontalConstructionLineGrid(i, j);

			// type change
			if (currentType != lastType) {
				// save last segment
				if(lastType != 0) {
					if (p1 != -1) { // new gap
						// add segment
						p2 = i;	// extend segment
						double ypos = geofile.m_grid.y_gridCoords[j];
						double x1 = geofile.m_grid.x_gridCoords[p1];
						double x2 = geofile.m_grid.x_gridCoords[p2];
						addHorizontalLine(ypos, x1, x2, lastType == 2);
						p1 = -1; p2 = -1;
					}
				}
				// start new segment
				if(currentType != 0) {
					p1 = i; p2 = i;
				}
				lastType = currentType;
			}
		}
		// add last segment if any
		if (p1 != -1) {
			p2 = horizontalConstructionLineGrid.cols();	// extend segment
			double ypos = geofile.m_grid.y_gridCoords[j];
			double x1 = geofile.m_grid.x_gridCoords[p1];
			double x2 = geofile.m_grid.x_gridCoords[p2];
			addHorizontalLine(ypos, x1, x2, lastType == 2);
		}
	}

	for (unsigned int i=0; i<verticalConstructionLineGrid.cols(); ++i) {
		int p1 = -1; // initialize with -1, so that we know when to start
		int p2 = -1;
		int lastType = 0;
		// process all segments in row
		for (unsigned int j=0; j<verticalConstructionLineGrid.rows(); ++j) {
			int currentType = verticalConstructionLineGrid(i, j);

			// type change
			if (currentType != lastType) {
				// save last segment
				if(lastType != 0) {
					if (p1 != -1) { // new gap
						// add segment
						p2 = j;	// extend segment
						double xpos = geofile.m_grid.x_gridCoords[i];
						double y1 = geofile.m_grid.y_gridCoords[p1];
						double y2 = geofile.m_grid.y_gridCoords[p2];
						addVerticalLine(xpos, y1, y2, lastType == 2);
						p1 = -1; p2 = -1;
					}
				}
				// start new segment
				if(currentType != 0) {
					p1 = j; p2 = j;
				}
				lastType = currentType;
			}
		}
		// add last segment if any
		if (p1 != -1) {
			p2 = verticalConstructionLineGrid.rows();	// extend segment
			double xpos = geofile.m_grid.x_gridCoords[i];
			double y1 = geofile.m_grid.y_gridCoords[p1];
			double y2 = geofile.m_grid.y_gridCoords[p2];
			addVerticalLine(xpos, y1, y2, lastType == 2);
		}
	}
}


void ConstructionLines2D::adjustHorizontalLineEnds(double xMin, double xMax) {
	for (unsigned int i=0; i<m_horizontalBoundaryLines.size(); ++i) {
		m_horizontalBoundaryLines[i].m_begin = xMin;
		m_horizontalBoundaryLines[i].m_end = xMax;
	}
	for (unsigned int i=0; i<m_horizontalConstructionLines.size(); ++i) {
		m_horizontalConstructionLines[i].m_begin = xMin;
		m_horizontalConstructionLines[i].m_end = xMax;
	}
}


#if 0
GeoFile::ConstructionLines3D GeoFile::generateConstructionLines3D() {
	// create element matrix
	IBK::matrix_3d<int> elementMatrix;
	elementMatrix.resize((unsigned int)m_grid.x_coords.size(), (unsigned int)m_grid.y_coords.size(), (unsigned int)m_grid.z_coords.size());
	elementMatrix.fill(-1);

	for (unsigned int e=0; e<m_elementsVec.size(); ++e) {
		const Element & elem = m_elementsVec[e];
		if ( elem.n != IBK::INVALID_ELEMENT)
			elementMatrix(elem.i, elem.j, elem.k) = e;
	}

	// Internal map for horizontal material boundary lines.
	// Coordinates correspond to x-column index,  y-gridline z-stack and indexindex.
	// Value identifies type of gridline: 0 - no grid line, 1 - material-material line, 2-material-void border
	IBK::matrix_3d<int>								horizontalConstructionLineGrid;
	// Internal map for horizontal material boundary lines.
	// Coordinates correspond to x-gridline index and y-row index and z-stack index (from bottom to top).
	// Value identifies type of gridline: 0 - no grid line, 1 - material-material line, 2-material-void border
	IBK::matrix_3d<int>								verticalConstructionLineGrid;
	// Internal map for depth material boundary lines.
	// Coordinates correspond to x-column and y-row index and z-gridline index (from front to back).
	// Value identifies type of gridline: 0 - no grid line, 1 - material-material line, 2-material-void border
	IBK::matrix_3d<int>								depthConstructionLineGrid;

	horizontalConstructionLineGrid.resize(elementMatrix.cols(), elementMatrix.rows()+1, elementMatrix.stacks());
	horizontalConstructionLineGrid.fill(0);
	verticalConstructionLineGrid.resize(elementMatrix.cols()+1, elementMatrix.rows(), elementMatrix.stacks());
	verticalConstructionLineGrid.fill(0);
	depthConstructionLineGrid.resize(elementMatrix.cols(), elementMatrix.rows(), elementMatrix.stacks()+1);
	depthConstructionLineGrid.fill(0);

	for (unsigned int e=0; e<m_elementsVec.size(); ++e) {
		const Element & elem = m_elementsVec[e];
		if ( elem.n == IBK::INVALID_ELEMENT )
			continue;
		int e_i = elem.i;
		int e_j = elem.j;
		int e_k = elem.k;
		// vertical left line: index e_i, e_j, e_k
		if (e_i == 0)					verticalConstructionLineGrid(e_i, e_j, e_k) = 2;
		else							verticalConstructionLineGrid(e_i, e_j, e_k) = gridLineType3D(e_i-1, e_j, e_k, elem.matnr, elementMatrix);

		// vertical right line: index e_i+1, e_j, e_k
		if (e_i == (int)elementMatrix.cols() - 1)	verticalConstructionLineGrid(e_i+1, e_j, e_k) = 2;
		else							verticalConstructionLineGrid(e_i+1, e_j, e_k) = gridLineType3D(e_i+1, e_j, e_k, elem.matnr, elementMatrix);

		// horizontal bottom line: index e_i, e_j, e_k
		if (e_j == 0)					horizontalConstructionLineGrid(e_i, e_j, e_k) = 2;
		else							horizontalConstructionLineGrid(e_i, e_j, e_k) = gridLineType3D(e_i, e_j-1, e_k, elem.matnr, elementMatrix);

		// horizontal top line: index e_i, e_j+1, e_k
		if (e_j == (int)elementMatrix.rows() - 1)	horizontalConstructionLineGrid(e_i, e_j+1, e_k) = 2;
		else							horizontalConstructionLineGrid(e_i, e_j+1, e_k) = gridLineType3D(e_i, e_j+1, e_k, elem.matnr, elementMatrix);

		// depth front line: index e_i, e_j, e_k
		if (e_k == 0)					depthConstructionLineGrid(e_i, e_j, e_k) = 2;
		else							depthConstructionLineGrid(e_i, e_j, e_k) = gridLineType3D(e_i, e_j, e_k-1, elem.matnr, elementMatrix);

		// depth back line: index e_i, e_j, e_k+1
		if (e_k == (int)elementMatrix.stacks() - 1)	depthConstructionLineGrid(e_i, e_j, e_k+1) = 2;
		else							depthConstructionLineGrid(e_i, e_j, e_k+1) = gridLineType3D(e_i, e_j, e_k+1, elem.matnr, elementMatrix);
	}

	ConstructionLines3D constructionLines;

	// now loop through all lines and store consecutive lines into vectors

	for (unsigned int k=0; k<horizontalConstructionLineGrid.stacks(); ++k) {
		for (unsigned int j=0; j<horizontalConstructionLineGrid.rows(); ++j) {
			int p1 = -1; // initialize with -1, so that we know when to start
			int p2 = -1;
			// process all segments in row
			for (unsigned int i=0; i<horizontalConstructionLineGrid.cols(); ++i) {
				if (horizontalConstructionLineGrid(i, j, k) != 0) {
					if (p1 == -1) {	// new segment
						p1 = i; p2 = i;
					}
					else {
						p2 = i;	// extend segment
					}
				}
				else {
					if (p1 != -1) { // new gap
						// add segment
						double ypos = m_grid.y_gridCoords[j];
						double zpos = m_grid.z_gridCoords[k];
						double x1 = m_grid.x_gridCoords[p1];
						double x2 = m_grid.x_gridCoords[p2];
						ConstructionLine3D line = ConstructionLine3D::constructionLine3DHorizontal(ypos, zpos, x1, x2);
						constructionLines.m_horizontalLines.push_back(line);
						p1 = -1; p2 = -1;
					}
				}
			}
			// add last segment if any
			if (p1 != -1) {
				double ypos = m_grid.y_gridCoords[j];
				double zpos = m_grid.z_gridCoords[k];
				double x1 = m_grid.x_gridCoords[p1];
				double x2 = m_grid.x_gridCoords[p2];
				ConstructionLine3D line = ConstructionLine3D::constructionLine3DHorizontal(ypos, zpos, x1, x2);
				constructionLines.m_horizontalLines.push_back(line);
			}
		}
	}

	for (unsigned int k=0; k<verticalConstructionLineGrid.stacks(); ++k) {
		for (unsigned int i=0; i<verticalConstructionLineGrid.cols(); ++i) {
			int p1 = -1; // initialize with -1, so that we know when to start
			int p2 = -1;
			// process all segments in row
			for (unsigned int j=0; j<verticalConstructionLineGrid.rows(); ++j) {
				if (verticalConstructionLineGrid(i, j, k) != 0) {
					if (p1 == -1) {	// new segment
						p1 = j; p2 = j;
					}
					else {
						p2 = j;	// extend segment
					}
				}
				else {
					if (p1 != -1) { // new gap
						// add segment
						double xpos = m_grid.x_gridCoords[i];
						double zpos = m_grid.z_gridCoords[k];
						double y1 = m_grid.y_gridCoords[p1];
						double y2 = m_grid.y_gridCoords[p2];
						ConstructionLine3D line = ConstructionLine3D::constructionLine3DVertical(xpos, zpos, y1, y2);
						constructionLines.m_verticalLines.push_back(line);
						p1 = -1; p2 = -1;
					}
				}
			}
			// add last segment if any
			if (p1 != -1) {
				double xpos = m_grid.x_gridCoords[i];
				double zpos = m_grid.z_gridCoords[k];
				double y1 = m_grid.y_gridCoords[p1];
				double y2 = m_grid.y_gridCoords[p2];
				ConstructionLine3D line = ConstructionLine3D::constructionLine3DVertical(xpos, zpos, y1, y2);
				constructionLines.m_verticalLines.push_back(line);
			}
		}
	}

	for (unsigned int j=0; j<depthConstructionLineGrid.rows(); ++j) {
		for (unsigned int i=0; i<depthConstructionLineGrid.cols(); ++i) {
			int p1 = -1; // initialize with -1, so that we know when to start
			int p2 = -1;
			// process all segments in row
			for (unsigned int k=0; k<depthConstructionLineGrid.stacks(); ++k) {
				if (depthConstructionLineGrid(i, j, k) != 0) {
					if (p1 == -1) {	// new segment
						p1 = k; p2 = k;
					}
					else {
						p2 = k;	// extend segment
					}
				}
				else {
					if (p1 != -1) { // new gap
						// add segment
						double xpos = m_grid.x_gridCoords[i];
						double ypos = m_grid.y_gridCoords[j];
						double z1 = m_grid.z_gridCoords[p1];
						double z2 = m_grid.z_gridCoords[p2];
						ConstructionLine3D line = ConstructionLine3D::constructionLine3DDepth(xpos, ypos, z1, z2);
						constructionLines.m_depthLines.push_back(line);
						p1 = -1; p2 = -1;
					}
				}
			}
			// add last segment if any
			if (p1 != -1) {
				double xpos = m_grid.x_gridCoords[i];
				double ypos = m_grid.y_gridCoords[j];
				double z1 = m_grid.z_gridCoords[p1];
				double z2 = m_grid.z_gridCoords[p2];
				ConstructionLine3D line = ConstructionLine3D::constructionLine3DDepth(xpos, ypos, z1, z2);
				constructionLines.m_depthLines.push_back(line);
			}
		}
	}

	return constructionLines;
}

int GeoFile::gridLineType3D(int ni, int nj, int nk, unsigned int myMatNr, const IBK::matrix_3d<int>& elementMatrix) const {
	int neighborN = elementMatrix(ni, nj, nk); // mind operator()(col, row, stack)
	if (neighborN == -1)
		return 2;
	if (m_elementsVec[neighborN].matnr != myMatNr)
		return 1;
	else
		return 0;
}
#endif

} // namespace DATAIO
