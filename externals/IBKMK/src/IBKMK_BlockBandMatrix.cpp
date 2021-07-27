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

#include "IBKMK_BlockBandMatrix.h"

#include <cstring>

#include <IBK_assert.h>

#include "IBKMK.h"

namespace IBKMK {


// *** Implementation of BlockBandMatrix ***

const BlockBandMatrix & BlockBandMatrix::operator=(const BlockBandMatrix & b) {
	if (m_n == b.m_n && m_m == b.m_m && m_ml == b.m_ml && m_mu == b.m_mu) {
		std::copy(b.m_data.begin(), b.m_data.end(), m_data.begin());
	}
	else {
		m_data = b.m_data;
		m_n = b.m_n;
		m_m = b.m_m;
		m_ml = b.m_ml;
		m_mu = b.m_mu;
		m_blockSize = b.m_blockSize;
	}
	return *this;
}


void BlockBandMatrix::resize(unsigned int n, unsigned int m, unsigned int ml, unsigned int mu) {
	IBK_ASSERT(n > 0);
	IBK_ASSERT(m > 0);
//	IBK_ASSERT(ml >= 0); // this is always true
//	IBK_ASSERT(mu >= 0); // this is always true
	m_n = n;
	m_m = m;
	m_ml = ml;
	m_mu = mu;
	m_blockSize = m_m*m_m;
	unsigned int nBlocks = n*m_blockSize*(ml+mu+1);
	m_data.resize(nBlocks);
	std::memset(&m_data[0], 0, nBlocks*sizeof(double));
}


double * BlockBandMatrix::submatrix(unsigned int i, unsigned int j) {
	// j == i gives column with index ml
	// j != i gives column with index ml + j - i
	// offset = (i*(m_ml+m_mu+1) * m_ml + j - i)*m_blockSize
	//        = (i*(m_ml+m_mu) + m_ml + j)*m_blockSize
	return &m_data[0] + (i*(m_ml+m_mu) + m_ml + j)*m_blockSize;
}


const double * BlockBandMatrix::submatrix(unsigned int i, unsigned int j) const {
	return &m_data[0] + (i*(m_ml+m_mu) + m_ml + j)*m_blockSize;
}


int BlockBandMatrix::lu() {
	switch (m_m) {
		case 2:
		default:
			return ibkmk_blockband_lu(m_n, m_m, m_ml, m_mu, const_cast<double*>(&m_data[0]));
	}
}


void BlockBandMatrix::backsolve(double * b) const {
	switch (m_m) {
		case 2:
		default:
			ibkmk_blockband_backsolve(m_n, m_m, m_ml, m_mu, const_cast<double*>(&m_data[0]), b);
	}
}


void BlockBandMatrix::multiply(const double * b, double * r) const {
	switch (m_m) {
		case 2:
			return ibkmk_blockband_multiply2(m_n, m_ml, m_mu, const_cast<double*>(&m_data[0]), const_cast<double*>(b), r);
		default:
			return ibkmk_blockband_multiply(m_n, m_m, m_ml, m_mu, const_cast<double*>(&m_data[0]), const_cast<double*>(b), r);
	}
}


void BlockBandMatrix::write(std::ostream & out, double * b, bool eulerFormat, unsigned int width,
							const char * const matrixLabel, const char * const vectorLabel) const
{
//	if (width < 1)	width = 10;
//	else 			--width; // subtract one for automatically padded " "
//	if (matlabFormat) {
//#define IBK_WRITE_EULER_FORMAT
//#ifdef IBK_WRITE_EULER_FORMAT
//		out << ">" << matrixLabel << " = [ ";
//		for (unsigned int i=0; i<m_n*m_m; ++i) {
//			int mi = i/m_m;		// block-row
//			for (unsigned int j=0; j<m_n*m_m; ++j) {
//				// determine block matrix coordinates
//				int mj = j/m_m;		// block-column
//				// submatrix coordinates
//				int smi = i % m_m;		// submatrix-row
//				int smj = j % m_m;		// submatrix-column
//				int smidx = smi + m_m*smj;
//				out << std::setw(width) << std::right;
//				// outside of the three bands?
//				if (mj < (int)(mi - m_ml) || mj > (int)(mi + m_mu))
//					out << " 0";
//				else
//					out << submatrix(mi, mj)[smidx];
//				if (j != m_n*m_m-1) out << ", ";
//			}
//			if (i != m_n*m_m-1) out << "; ";
//		}
//		out << "]" << std::endl;
//		// check if we need to write rhs
//		if (b != NULL) {
//			out << ">"<< vectorLabel << " = [ ";
//			for (unsigned int i=0; i<m_n*m_m; ++i) {
//				out << b[i];
//				if (i != m_n*m_m-1) out << "; ";
//			}
//			out << "]" << std::endl ;
//		}
//#endif // IBK_WRITE_EULER_FORMAT
//		// writes matrix in matlab format
////		for (unsigned int i=0; i<m_n*m_m; ++i) {
////			for (unsigned int j=0; j<m_n*m_m; ++j) {
////				// determine block matrix coordinates
////				unsigned int mi = i/m_m;		// block-row
////				unsigned int mj = j/m_m;		// block-column
////				// submatrix coordinates
////				unsigned int smi = i % m_m;		// submatrix-row
////				unsigned int smj = j % m_m;		// submatrix-column
////				unsigned int smidx = smi + m_m*smj;
////				// outside of the three bands?
////				out << std::setw(width) << std::right;
////				if (mi == mj)			out << submatrix(mi, Main)[smidx];
////				else if (mj+1 == mi)	out << submatrix(mi, Lower)[smidx];
////				else if (mj == mi+1)	out << submatrix(mi, Upper)[smidx];
////				else 					out << 0;
////				out << " ";
////			}
////			if (i != m_n*m_m-1)
////				out << ";" << std::endl;
////		}
//	}
//	// dump in screen format
//	else {
//		for (unsigned int i=0; i<m_n*m_m; ++i) {
//			int mi = i/m_m;		// block-row
//			out << "[ ";
//			for (unsigned int j=0; j<m_n*m_m; ++j) {
//				// determine block matrix coordinates
//				int mj = j/m_m;		// block-column
//				// submatrix coordinates
//				int smi = i % m_m;		// submatrix-row
//				int smj = j % m_m;		// submatrix-column
//				int smidx = smi + m_m*smj;
//				out << std::setw(width) << std::right;
//				// outside of the three bands?
//				if (mj < (int)(mi - m_ml) || mj > (int)(mi + m_mu))
//					out << ".";
//				else
//					out << submatrix(mi, mj)[smidx];
//				out << " ";
//			}
//			out << "]";
//			if (b != NULL)
//				out << "  [ " << std::setw(width) << std::right << b[i] << " ]";
//			out << std::endl;
//		}
//		out << std::endl;
//	}
}



} // namespace IBKMK

