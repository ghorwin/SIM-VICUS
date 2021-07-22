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

#include <vector>
#include <cstring>

#include "IBKMK_BlockSparseMatrix.h"
#include "IBKMKC_sparse_matrix.h"

#include <IBK_InputOutput.h>

namespace IBKMK {


void BlockSparseMatrix::resize(unsigned int n, unsigned int blocksPerRow,
								unsigned int nSubMatrix, const unsigned int * indices)
{
	m_n = n;
	m_blocksPerRow = blocksPerRow;
	m_blockSize = nSubMatrix*nSubMatrix;
	m_nSubMatrix = nSubMatrix;
	m_data.resize(m_n*m_blocksPerRow*m_blockSize);
	m_index.resize(m_n*m_blocksPerRow);

	// populate the index vector if indices have been passed
	if (indices != NULL)
		std::memcpy((void*)&m_index[0], (const void*)indices, sizeof(unsigned int)*m_n*m_blocksPerRow);
	else {
		for (unsigned int i=0; i<m_n*m_blocksPerRow; ++i)
			m_index[i] = 0;
	}
}


double & BlockSparseMatrix::operator()(unsigned int i, unsigned int j) {
	// check if there is a block at this index, otherwise just print a space
	// determine block matrix coordinates
	unsigned int iBlock = i/m_nSubMatrix; //block-row
	unsigned int jBlock = j/m_nSubMatrix; //block-column
	// coordinates of submatrix
	unsigned int iSub = i%m_nSubMatrix; //submatrix-row
	unsigned int jSub = j%m_nSubMatrix; //submatrix-column
	unsigned int idxSub = m_nSubMatrix*jSub+iSub; //storage place in submatrix
	double * submatrix = block(iBlock,jBlock);
	if (submatrix == NULL)
		throw IBK::Exception(
				IBK::FormatString("Invalid matrix position, element (%1,%2) is not part of matrix pattern.").arg(i).arg(j),
				"[BlockSparseMatrix::operator]");
	return submatrix[idxSub];
}


double BlockSparseMatrix::value(unsigned int i, unsigned int j) const {
	// check if there is a block at this index, otherwise just print a space
	// determine block matrix coordinates
	unsigned int iBlock = i/m_nSubMatrix; //block-row
	unsigned int jBlock = j/m_nSubMatrix; //block-column
	// coordinates of submatrix
	unsigned int iSub = i%m_nSubMatrix; //submatrix-row
	unsigned int jSub = j%m_nSubMatrix; //submatrix-column
	unsigned int idxSub = m_nSubMatrix*jSub+iSub; //storage place in submatrix
	const double * submatrix = block(iBlock,jBlock);
	if (submatrix == NULL)	return 0;
	else					return submatrix[idxSub];
}


double * BlockSparseMatrix::block(unsigned int i, unsigned int j) {
	// search all index values in row i until we find the column j
	IBK_ASSERT(i < m_n);
	IBK_ASSERT(j < m_n);
	// k1 is first element of row i
	unsigned int k1 = i*m_blocksPerRow;
	// k2 is first element of row i+1
	unsigned int k2 = k1 + m_blocksPerRow;
	// loop over all elements of row i
	while (k1 < k2) {
		if (m_index[k1] == (unsigned int)j)
			return &m_data[k1*m_blockSize];
		++k1;
	}
	return NULL;			// column not found in index row, return 0
}


// implementation is the same as above, just with const
const double * BlockSparseMatrix::block(unsigned int i, unsigned int j) const {
	return const_cast<BlockSparseMatrix*>(this)->block(i,j);
}


/*! Multiplies matrix with vector b and stores result in vector res. */
void BlockSparseMatrix::multiply(const double * b, double * res) const {
	ibkmk_blockspmat_eid_multiply(m_n, m_blocksPerRow, m_nSubMatrix, index(), data(), b, res);
}


void BlockSparseMatrix::write(std::ostream & out, double * b, bool eulerFormat, unsigned int width,
							  const char * const matrixLabel, const char * const vectorLabel) const
{
	// re-use generic implementation
	IBK::write_matrix(out, *this, b, eulerFormat, width, matrixLabel, vectorLabel);
}


void BlockSparseMatrix::ilu() {
	ibkmk_blockspmat_eid_ilu(m_n, m_nSubMatrix, m_blocksPerRow, &m_index[0], &m_data[0]);
}


void BlockSparseMatrix::backsolveILU(double * b) const {
	ibkmk_blockspmat_eid_backsolve(m_n, m_nSubMatrix, m_blocksPerRow, &m_index[0], &m_data[0], b);
}


} // namespace IBKMK

