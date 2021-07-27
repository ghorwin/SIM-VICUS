/*	IBK Math Kernel Library
	Copyright (c) 2001-today, Institut fuer Bauklimatik, TU Dresden, Germany

	Written by A. Nicolai, A. Paepcke, H. Fechner, St. Vogelsang
	All rights reserved.

	This file is part of the IBKMK Library.

	Redistribution and use in source and binary forms, with or without modification,
	are permitted provided that the following conditions are met:

	1. Redistributions of source code must retain the above copyright notice, this
	   list of conditions and the following disclaimer.

	2. Redistributions in binary form must reproduce the above copyright notice,
	   this list of conditions and the following disclaimer in the documentation
	   and/or other materials provided with the distribution.

	3. Neither the name of the copyright holder nor the names of its contributors
	   may be used to endorse or promote products derived from this software without
	   specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
	ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
	DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
	ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
	(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
	LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
	ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	This library contains derivative work based on other open-source libraries,
	see LICENSE and OTHER_LICENSES files.

*/

#include "IBKMK_IndexGenerator.h"

#include <iomanip>
#include <algorithm>

namespace IBKMK {

void IndexGenerator::setElements(const std::vector<Element> & elements, IndexGeneratorTypes type) {

	m_elements.clear();
	m_elements = elements;
	m_grid.clear();
	m_grid3D.clear();

	// first pass, determine max i, j, and k values
	unsigned int maxI = 0;
	unsigned int maxJ = 0;
	unsigned int maxK = 0;
	for (unsigned int i=0; i<elements.size(); ++i) {
		maxI = std::max(maxI, elements[i].i);
		maxJ = std::max(maxJ, elements[i].j);
		maxK = std::max(maxK, elements[i].k);
	}

	// resize grid
	IBK_ASSERT(type <= IGT_3D);
	m_type = type;
	switch(type){
		case IGT_1D:
		case IGT_2D:

			m_grid.resize(maxI+1, maxJ+1);

			// initialize grid
			m_grid.fill(NULL);

			// second pass, set element pointers
			for (unsigned int i=0; i<elements.size(); ++i) {
				m_grid(elements[i].i, elements[i].j) = &m_elements[i];
			}

			break;
		case IGT_3D:

			m_grid3D.resize(maxI+1, maxJ+1, maxK+1);

			// initialize grid
			m_grid3D.fill(NULL);

			// second pass, set element pointers
			for (unsigned int i=0; i<elements.size(); ++i) {
				m_grid3D(elements[i].i, elements[i].j, elements[i].k) = &m_elements[i];
			}

			break;
	}
}

enum NumberingOrder {
	XYZ,
	XZY,
	YXZ,
	YZX,
	ZXY,
	ZYX
};

void IndexGenerator::createCartesianGrid(unsigned int xNum, unsigned int yNum, unsigned int zNum, IndexGeneratorTypes preSetType) {

	unsigned int count = xNum*yNum*zNum;
	int type = 0;

	IBK_ASSERT(xNum >= 1);
	IBK_ASSERT(yNum >= 1);
	IBK_ASSERT(zNum >= 1);

	if (xNum > 1) ++type;
	if (yNum > 1) ++type;
	if (zNum > 1) ++type;

	// if we have just an one element we need at least one dimensional
	if (type == 0) type = 1;
	if (type < preSetType)
		type = preSetType;

	std::vector<Element> elements;
	elements.resize(count);

	// determine numbering direction
	NumberingOrder numberingOrder;
	if (xNum > yNum) {
		if (xNum > zNum) {
			if (yNum > zNum) {
				numberingOrder = ZYX;
			}
			else {
				numberingOrder = YZX;
			}
		}
		else {
			// yNum < xNum < zNum
			numberingOrder = YXZ;
		}
	}
	else {
		// xNum < yNum
		if (xNum > zNum) {
			// zNum < xNum < yNum
			numberingOrder = ZXY;
		}
		else {
			// xNum < yNum, xNum < zNum
			if (yNum > zNum) {
				// xNum < zNum < yNum
				numberingOrder = XZY;
			}
			else {
				// xNum < yNum < zNum
				numberingOrder = XYZ;
			}
		}
	}


	unsigned int currentI = 0;
	unsigned int currentJ = 0;
	unsigned int currentK = 0;
	for (unsigned int i=0; i<count; ++i) {
		Element & e = elements[i];
		e.n = i;
		e.i = currentI;
		e.j = currentJ;
		e.k = currentK;

		switch (numberingOrder) {
			case XYZ :
				++currentI;
				if (currentI >= xNum){
					currentI = 0;
					++currentJ;
				}
				if (currentJ >= yNum){
					currentJ = 0;
					++currentK;
				}
				break;

			case XZY :
				++currentI;
				if (currentI >= xNum) {
					currentI = 0;
					++currentK;
				}
				if (currentK >= zNum) {
					currentK = 0;
					++currentJ;
				}
				break;

			case YXZ :
				++currentJ;
				if (currentJ >= yNum) {
					currentJ = 0;
					++currentI;
				}
				if (currentI >= xNum) {
					currentI = 0;
					++currentK;
				}
				break;

			case YZX :
				++currentJ;
				if (currentJ >= yNum) {
					currentJ = 0;
					++currentK;
				}
				if (currentK >= zNum) {
					currentK = 0;
					++currentI;
				}
				break;

			case ZXY :
				++currentK;
				if (currentK >= zNum) {
					currentK = 0;
					++currentI;
				}
				if (currentI >= xNum) {
					currentI = 0;
					++currentJ;
				}
				break;

			case ZYX :
				++currentK;
				if (currentK >= zNum) {
					currentK = 0;
					++currentJ;
				}
				if (currentJ >= yNum) {
					currentJ = 0;
					++currentI;
				}
				break;
		}
	}

	setElements(elements, (IndexGeneratorTypes)type);
}


void IndexGenerator::generateADIMappings(std::vector<unsigned int> & adiX, std::vector<unsigned int> & adiY) const {
	// resize vectors
	adiX.resize(m_elements.size());
	adiY.resize(m_elements.size());
	// number horizontally and create mapping
	unsigned int k = 0;
	for (unsigned int r=0; r<m_grid.rows(); ++r) {
		for (unsigned int c=0; c<m_grid.cols(); ++c) {
			if (m_grid(c,r) != NULL) {
				adiX[k++] = m_grid(c,r)->n;
			}
		}
	}
	// number vertically
	k = 0;
	for (unsigned int c=0; c<m_grid.cols(); ++c) {
		for (unsigned int r=0; r<m_grid.rows(); ++r) {
			if (m_grid(c,r) != NULL) {
				adiY[k++] = m_grid(c,r)->n;
			}
		}
	}
}

void IndexGenerator::generateSparseMatrixMappings( std::vector<unsigned int> & indexArray, unsigned int & elementsPerRow, unsigned int nEquations ) const  {

	unsigned int rowLength = 0;
	unsigned int blockRowLength = 0;
	switch(m_type){
		case IGT_1D:
			rowLength = 3*nEquations;
			blockRowLength = 3*nEquations*nEquations;
			break;
		case IGT_2D:
			rowLength = 5*nEquations;
			blockRowLength = 5*nEquations*nEquations;
			break;
		case IGT_3D:
			rowLength = 7*nEquations;
			blockRowLength = 7*nEquations*nEquations;
			break;
	}

	indexArray.clear();
	indexArray.resize(m_elements.size()*blockRowLength);
	std::fill(indexArray.begin(), indexArray.end(), -1);

	switch(m_type){
		case IGT_1D:
		case IGT_2D: {
			// process grid and store all index entries
			for (unsigned int r=0; r<m_grid.rows(); ++r) {
				for (unsigned int c=0; c<m_grid.cols(); ++c) {
					// we process each element once, currentEIndex holds the index of the current element
					const Element * E = m_grid(c,r);
					if (E == NULL) continue; // empty grid cell, skip
					unsigned int currentEIndex = E->n;

					// special convention, first block column always holds main diagonal element
					for (unsigned int vc=0; vc<nEquations; ++vc) {
						for (unsigned int vr=0; vr<nEquations; ++vr) {
							indexArray[blockRowLength*currentEIndex + vr*rowLength + vc] = currentEIndex*nEquations + vc;
							if (vc == vr && vc > 0) {
								std::swap(indexArray[blockRowLength*currentEIndex + vr*rowLength + vc],
											indexArray[blockRowLength*currentEIndex + vr*rowLength]);
							}
						}
					}
					unsigned int blockIndex = 1; // skip first block, because we have it already set
					// neighbor to the left?
					if (c > 0 && m_grid(c-1,r) != NULL) {
						// set m*m block with element index
						unsigned int eIndexNeighbor = m_grid(c-1,r)->n;
						for (unsigned int vc=0; vc<nEquations; ++vc) {
							for (unsigned int vr=0; vr<nEquations; ++vr) {
								// set block indices
								indexArray[blockRowLength*currentEIndex + vr*rowLength + nEquations*blockIndex + vc] = eIndexNeighbor*nEquations + vc;
							}
						}
						++blockIndex;
					} // neighbor to the left?
					// neighbor to the right?
					if (c+1 < m_grid.cols() && m_grid(c+1,r) != NULL) {
						// set m*m block with element index
						unsigned int eIndexNeighbor = m_grid(c+1,r)->n;
						for (unsigned int vc=0; vc<nEquations; ++vc) {
							for (unsigned int vr=0; vr<nEquations; ++vr) {
								// set block indices
								indexArray[blockRowLength*currentEIndex + vr*rowLength + nEquations*blockIndex + vc] = eIndexNeighbor*nEquations + vc;
							}
						}
						++blockIndex;
					} // neighbor to the right?
					// neighbor to the top?
					if (r > 0 && m_grid(c,r-1) != NULL) {
						// set m*m block with element index
						unsigned int eIndexNeighbor = m_grid(c,r-1)->n;
						for (unsigned int vc=0; vc<nEquations; ++vc) {
							for (unsigned int vr=0; vr<nEquations; ++vr) {
								// set block indices
								indexArray[blockRowLength*currentEIndex + vr*rowLength + nEquations*blockIndex + vc] = eIndexNeighbor*nEquations + vc;
							}
						}
						++blockIndex;
					} // neighbor to the top?
					// neighbor to the bottom?
					if (r +1 < m_grid.rows() && m_grid(c,r+1) != NULL) {
						// set m*m block with element index
						unsigned int eIndexNeighbor = m_grid(c,r+1)->n;
						for (unsigned int vc=0; vc<nEquations; ++vc) {
							for (unsigned int vr=0; vr<nEquations; ++vr) {
								// set block indices
								indexArray[blockRowLength*currentEIndex + vr*rowLength + nEquations*blockIndex + vc] = eIndexNeighbor*nEquations + vc;
							}
						}
						++blockIndex;
					} // neighbor to the bottom?
				} // for (unsigned int c=0; c<m_grid.cols(); ++c) {
			} // for (unsigned int r=0; r<m_grid.rows(); ++r) {

		} break;

		case IGT_3D: {

			// process grid and store all index entries
			for (unsigned int s=0, sEnd=m_grid3D.stacks(); s<sEnd; ++s) {
				for (unsigned int r=0, rEnd=m_grid3D.rows(); r<rEnd; ++r) {
					for (unsigned int c=0, cEnd=m_grid3D.cols(); c<cEnd; ++c) {

						// we process each element once, currentEIndex holds the index of the current element
						const Element * E = m_grid3D(c,r,s);

						// empty grid cell, skip
						if (E == NULL)
							continue;

						unsigned int currentEIndex = E->n;

						// special convention, first block column always holds main diagonal element
						for (unsigned int vc=0; vc<nEquations; ++vc) {
							for (unsigned int vr=0; vr<nEquations; ++vr) {

								indexArray[blockRowLength*currentEIndex + vr*rowLength + vc] = currentEIndex*nEquations + vc;
								if (vc == vr && vc > 0) {
									std::swap(indexArray[blockRowLength*currentEIndex + vr*rowLength + vc],
												indexArray[blockRowLength*currentEIndex + vr*rowLength]);
								}

							} // for (unsigned int vr=0; vr<m; ++vr) {
						} // for (unsigned int vc=0; vc<m; ++vc) {

						unsigned int blockIndex = 1; // skip first block, because we have it already set
						// neighbor to the left?
						if (c > 0 && m_grid3D(c-1,r,s) != NULL) {
							// set m*m block with element index
							unsigned int eIndexNeighbor = m_grid3D(c-1,r,s)->n;
							for (unsigned int vc=0; vc<nEquations; ++vc) {
								for (unsigned int vr=0; vr<nEquations; ++vr) {
									// set block indices
									indexArray[blockRowLength*currentEIndex + vr*rowLength + nEquations*blockIndex + vc] = eIndexNeighbor*nEquations + vc;
								}
							}
							++blockIndex;
						} // neighbor to the left?
						// neighbor to the right?
						if (c+1 < cEnd && m_grid3D(c+1,r,s) != NULL) {
							// set m*m block with element index
							unsigned int eIndexNeighbor = m_grid3D(c+1,r,s)->n;
							for (unsigned int vc=0; vc<nEquations; ++vc) {
								for (unsigned int vr=0; vr<nEquations; ++vr) {
									// set block indices
									indexArray[blockRowLength*currentEIndex + vr*rowLength + nEquations*blockIndex + vc] = eIndexNeighbor*nEquations + vc;
								}
							}
							++blockIndex;
						} // neighbor to the right?
						// neighbor to the top?
						if (r > 0 && m_grid3D(c,r-1,s) != NULL) {
							// set m*m block with element index
							unsigned int eIndexNeighbor = m_grid3D(c,r-1,s)->n;
							for (unsigned int vc=0; vc<nEquations; ++vc) {
								for (unsigned int vr=0; vr<nEquations; ++vr) {
									// set block indices
									indexArray[blockRowLength*currentEIndex + vr*rowLength + nEquations*blockIndex + vc] = eIndexNeighbor*nEquations + vc;
								}
							}
							++blockIndex;
						} // neighbor to the top?
						// neighbor to the bottom?
						if (r +1 < rEnd && m_grid3D(c,r+1,s) != NULL) {
							// set m*m block with element index
							unsigned int eIndexNeighbor = m_grid3D(c,r+1,s)->n;
							for (unsigned int vc=0; vc<nEquations; ++vc) {
								for (unsigned int vr=0; vr<nEquations; ++vr) {
									// set block indices
									indexArray[blockRowLength*currentEIndex + vr*rowLength + nEquations*blockIndex + vc] = eIndexNeighbor*nEquations + vc;
								}
							}
							++blockIndex;
						} // neighbor to the bottom?

						// neighbor to the back?
						if (s > 0 && m_grid3D(c,r,s-1) != NULL) {
							// set m*m block with element index
							unsigned int eIndexNeighbor = m_grid3D(c,r,s-1)->n;
							for (unsigned int vc=0; vc<nEquations; ++vc) {
								for (unsigned int vr=0; vr<nEquations; ++vr) {
									// set block indices
									indexArray[blockRowLength*currentEIndex + vr*rowLength + nEquations*blockIndex + vc] = eIndexNeighbor*nEquations + vc;
								}
							}
							++blockIndex;
						} // neighbor to the back?
						// neighbor to the front?
						if (s +1 < sEnd && m_grid3D(c,r,s+1) != NULL) {
							// set m*m block with element index
							unsigned int eIndexNeighbor = m_grid3D(c,r,s+1)->n;
							for (unsigned int vc=0; vc<nEquations; ++vc) {
								for (unsigned int vr=0; vr<nEquations; ++vr) {
									// set block indices
									indexArray[blockRowLength*currentEIndex + vr*rowLength + nEquations*blockIndex + vc] = eIndexNeighbor*nEquations + vc;
								}
							}
							++blockIndex;
						} // neighbor to the front?


					} // for (unsigned int c=0; c<m_grid3D.cols(); ++c) {
				} // for (unsigned int r=0; r<m_grid3D.rows(); ++r) {
			} // for (unsigned int s=0; s<m_grid3D.stacks(); ++s) {


		} break;
	} // switch

	// sort rows and replace -1 with invalid indices
	for (unsigned int i=0; i<m_elements.size()*nEquations; ++i) {
		unsigned int * rowIndices = (unsigned int*)&indexArray[0] + i*rowLength;
		std::sort(rowIndices, rowIndices + rowLength);
		for (unsigned int j=0; j<rowLength; ++j) {
			if (rowIndices[j] == (unsigned int)-1)
				rowIndices[j] = rowIndices[j-1];
		}
	}

	elementsPerRow = rowLength;
}

void IndexGenerator::generateSparseMatrixMappingsCSR(std::vector<unsigned int> & ia, std::vector<unsigned int> & ja, unsigned int nEquations ) const {

	unsigned int nElements = (unsigned int)m_elements.size();
	// we only know row offset vector size ...
	ia.clear();
	ia.resize(nElements * nEquations + 1, 0);
	// ... but nit the size of column index vector
	ja.clear();

	switch(m_type){
		case IGT_1D:
		case IGT_2D: {

			// get access to the last offset value
			unsigned int rowBlockOffset = 0;
			// process grid and store all index entries
			for (unsigned int currentElemIndex = 0;  currentElemIndex  < nElements; ++ currentElemIndex) {
				// we process each element once, currentEIndex holds the index of the current element
				const Element &elem = m_elements[currentElemIndex];
				// get information about grid row and column
				unsigned int r = elem.j;
				unsigned int c = elem.i;

				std::set<unsigned int> columnIndexOfCurrentBlock;
				// elements internal connection (dependency between divergence and the current element states)
				for (unsigned int vc=0; vc<nEquations; ++vc) {
					columnIndexOfCurrentBlock.insert(currentElemIndex*nEquations + vc);
				}

				// we already stored one element
				unsigned int nColumnsPerBlock = 1;
				// neighbor to the left?
				if (c > 0 && m_grid(c-1,r) != NULL) {
					// set m*m block with element index
					unsigned int elemNeighborIndex = m_grid(c-1,r)->n;
					for (unsigned int vc=0; vc<nEquations; ++vc) {
						// set block indices
						columnIndexOfCurrentBlock.insert(elemNeighborIndex*nEquations + vc);
					}
					// update counter
					++nColumnsPerBlock;
				} // neighbor to the left?

				// neighbor to the right?
				if (c+1 < m_grid.cols() && m_grid(c+1,r) != NULL) {
					// set m*m block with element index
					unsigned int elemNeighborIndex = m_grid(c+1,r)->n;
					for (unsigned int vc=0; vc<nEquations; ++vc) {
						// set block indices
						columnIndexOfCurrentBlock.insert(elemNeighborIndex*nEquations + vc);
					}
					// update counter
					++nColumnsPerBlock;
				} // neighbor to the right?
				// neighbor to the top?
				if (r > 0 && m_grid(c,r-1) != NULL) {
					// set m*m block with element index
					unsigned int elemNeighborIndex = m_grid(c,r-1)->n;
					for (unsigned int vc=0; vc<nEquations; ++vc) {
						// set block indices
						columnIndexOfCurrentBlock.insert(elemNeighborIndex*nEquations + vc);
					}
					// update counter
					++nColumnsPerBlock;
				} // neighbor to the top?
				// neighbor to the bottom?
				if (r +1 < m_grid.rows() && m_grid(c,r+1) != NULL) {
					// set m*m block with element index
					unsigned int elemNeighborIndex = m_grid(c,r+1)->n;
					for (unsigned int vc=0; vc<nEquations; ++vc) {
						// set block indices
						columnIndexOfCurrentBlock.insert(elemNeighborIndex*nEquations + vc);
					}
					// update counter
					++nColumnsPerBlock;
				} // neighbor to the bottom?

				// store indices for each block row
				for(unsigned int vr = 0; vr < nEquations; ++vr) {
					ja.insert(ja.end(),columnIndexOfCurrentBlock.begin(), columnIndexOfCurrentBlock.end());
					// correct row index
					ia[currentElemIndex * nEquations + vr] = rowBlockOffset + vr * nColumnsPerBlock;
				}
				// update offset value
				rowBlockOffset += nEquations * nColumnsPerBlock;
			} // for (unsigned int currentElemIndex = 0;  currentElemIndex  < nElements; ++ currentElemIndex) {

			// fill last element
			ia[nElements * nEquations] = rowBlockOffset;

		} break;

		case IGT_3D: {

			// get access to the last offset value
			unsigned int rowBlockOffset = 0;
			// process grid and store all index entries
			for (unsigned int currentElemIndex = 0;  currentElemIndex  < nElements; ++ currentElemIndex) {

				// we process each element once, currentEIndex holds the index of the current element
				const Element &elem = m_elements[currentElemIndex];
				// get information about grid row and column
				unsigned int r = elem.j;
				unsigned int c = elem.i;
				unsigned int s = elem.k;

				std::set<unsigned int> columnIndexOfCurrentBlock;
				// elements internal connection (dependency between divergence and the current element states)
				for (unsigned int vc=0; vc<nEquations; ++vc) {
					columnIndexOfCurrentBlock.insert(currentElemIndex*nEquations + vc);
				}

				// we already stored one element
				unsigned int nColumnsPerBlock = 1;
				// neighbor to the left?
				if (c > 0 && m_grid3D(c-1,r,s) != NULL) {
					// set m*m block with element index
					unsigned int elemNeighborIndex = m_grid3D(c-1,r,s)->n;
					for (unsigned int vc=0; vc<nEquations; ++vc) {
						// set block indices
						columnIndexOfCurrentBlock.insert(elemNeighborIndex*nEquations + vc);
					}
					// update counter
					++nColumnsPerBlock;
				} // neighbor to the left?

				// neighbor to the right?
				if (c+1 < m_grid3D.cols() && m_grid3D(c+1,r,s) != NULL) {
					// set m*m block with element index
					unsigned int elemNeighborIndex = m_grid3D(c+1,r,s)->n;
					for (unsigned int vc=0; vc<nEquations; ++vc) {
						// set block indices
						columnIndexOfCurrentBlock.insert(elemNeighborIndex*nEquations + vc);
					}
					// update counter
					++nColumnsPerBlock;
				} // neighbor to the right?

				// neighbor to the top?
				if (r > 0 && m_grid3D(c,r-1,s) != NULL) {
					// set m*m block with element index
					unsigned int elemNeighborIndex = m_grid3D(c,r-1,s)->n;
					for (unsigned int vc=0; vc<nEquations; ++vc) {
						// set block indices
						columnIndexOfCurrentBlock.insert(elemNeighborIndex*nEquations + vc);
					}
					// update counter
					++nColumnsPerBlock;
				} // neighbor to the top?

				// neighbor to the bottom?
				if (r +1 < m_grid3D.rows() && m_grid3D(c,r+1,s) != NULL) {
					// set m*m block with element index
					unsigned int elemNeighborIndex = m_grid3D(c,r+1,s)->n;
					for (unsigned int vc=0; vc<nEquations; ++vc) {
						// set block indices
						columnIndexOfCurrentBlock.insert(elemNeighborIndex*nEquations + vc);
					}
					// update counter
					++nColumnsPerBlock;
				} // neighbor to the bottom?


				// neighbor to the back?
				if (s > 0 && m_grid3D(c,r,s-1) != NULL) {
					// set m*m block with element index
					unsigned int elemNeighborIndex = m_grid3D(c,r,s-1)->n;
					for (unsigned int vc=0; vc<nEquations; ++vc) {
						// set block indices
						columnIndexOfCurrentBlock.insert(elemNeighborIndex*nEquations + vc);
					}
					// update counter
					++nColumnsPerBlock;
				} // neighbor to the back?

				// neighbor to the front?
				if (s +1 < m_grid3D.stacks() && m_grid3D(c,r,s+1) != NULL) {
					// set m*m block with element index
					unsigned int elemNeighborIndex = m_grid3D(c,r,s+1)->n;
					for (unsigned int vc=0; vc<nEquations; ++vc) {
						// set block indices
						columnIndexOfCurrentBlock.insert(elemNeighborIndex*nEquations + vc);
					}
					// update counter
					++nColumnsPerBlock;
				} // neighbor to the front?

				// store indices for each block row
				for(unsigned int vr = 0; vr < nEquations; ++vr) {
					ja.insert(ja.end(),columnIndexOfCurrentBlock.begin(), columnIndexOfCurrentBlock.end());
					// correct row index
					ia[currentElemIndex * nEquations + vr] = rowBlockOffset + vr * nColumnsPerBlock;
				}
				// update offset value
				rowBlockOffset += nEquations * nColumnsPerBlock;
			} // for (unsigned int currentElemIndex = 0;  currentElemIndex  < nElements; ++ currentElemIndex) {

			// fill last element
			ia[nElements * nEquations] = rowBlockOffset;

		} break;
	} // switch
}


void IndexGenerator::scaleSparseMatrixIndexVector( const std::vector<unsigned int> & originalIndexArray, unsigned int elementsPerRow,
								   unsigned int blockDim, std::vector<unsigned int> & indexArray)
{
	indexArray.resize(originalIndexArray.size()*blockDim*blockDim);
	// generate index vector by scaling index vector of block-matrix
	unsigned int browlength = blockDim*elementsPerRow;
	unsigned int n = (unsigned int)originalIndexArray.size()/elementsPerRow;
	for (unsigned int i=0; i<n; ++i) {
		// first, expand current block-based index row into scalar matrix index row
		unsigned int bi = i*blockDim;
		const unsigned int * aindex = &originalIndexArray[0] + i*elementsPerRow;
		unsigned int * bindex = &indexArray[0] + bi*browlength;
		for (unsigned int m=0; m<elementsPerRow; ++m) {
			// check for invalid index
			if (m>0 && aindex[m] == aindex[m-1]) {
				// invalid index, simply store same index location as in last cell for all values
				for (unsigned int k=0; k<blockDim; ++k) {
					bindex[m*blockDim + k] = bindex[m*blockDim - 1];
				}
			}
			else {
				// valid index, simply number columns
				for (unsigned int k=0; k<blockDim; ++k) {
					bindex[m*blockDim + k] = aindex[m]*blockDim + k;
				}
			}
		}
		// now multiply this for the remaining blockDim-1 rows
		for (unsigned int r=1; r<blockDim; ++r) {
			for (unsigned int m=0; m<browlength; ++m) {
				bindex[m + r*browlength] = bindex[m];
			}
		}
	}

}


void IndexGenerator::dumpGrid(std::ostream & out, unsigned int width, IndexGeneratorTypes type ) const {

	switch( type ){
		case IGT_1D:
		case IGT_2D: {

				for (unsigned int r=0,rEnd=m_grid.rows(); r<rEnd; ++r) {
					for (unsigned int c=0, cEnd=m_grid.cols(); c<cEnd; ++c) {
						const Element * e = m_grid(c,r);
						if (e != NULL)
							out << std::setw(width) << e->n;
						else
							out << std::setw(width) << " ";
					}
					out << std::endl;
				}

			} break;
		case IGT_3D:

			unsigned int sEnd_1 = m_grid3D.rows()-1;

			for (unsigned int s=0, sEnd=m_grid3D.rows(); s<sEnd; ++s) {
				for (unsigned int r=0,rEnd=m_grid3D.rows(); r<rEnd; ++r) {
					for (unsigned int c=0, cEnd=m_grid3D.cols(); c<cEnd; ++c) {
						const Element * e = m_grid3D(c,r,s);
						if (e != NULL)
							out << std::setw(width) << e->n;
						else
							out << std::setw(width) << " ";
					}
					out << std::endl;
				}

				if (s < sEnd_1 ){
					// more linebreak after each stack
					out << std::endl;
					out << "======" << std::endl;
					out << std::endl;
				}

			}
			break;
	}

}

} // namespace IBKMK
