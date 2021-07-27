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

#ifndef IBKMK_IndexGeneratorH
#define IBKMK_IndexGeneratorH

#include <iostream>

#include <IBK_matrix.h>
#include <IBK_matrix_3d.h>

namespace IBKMK {

/*! This class can generate index mappings for various matrix
	representations.
	\code
	// create index generator
	IndexGenerator generator;
	// set elements vector
	generator.setElements( elementsVec );
	// generate index mappings for block-based ADI grids
	std::vector<unsigned int>	adiX, adiY;
	generator.generateADIIndexMap(adiX, adiY);

	// generate index array for sparse matrix
	std::vector<unsigned int>	indexVector;
	unsigned int				blockSize = 2;
	generator.generateSparseMatrixIndex(indexVector, blocksize);
	\endcode

	\todo Remove from IBK library, too application-specific

*/
class IndexGenerator {
public:

	/*! This struct maps matrix coordinate indices to a unique element index number.
	*/
	struct Element {

		Element( unsigned int n_=0, unsigned int i_=0, unsigned int j_=0, unsigned int k_=0 ) :
			i(i_),
			j(j_),
			k(k_),
			n(n_)
		{
		}

		unsigned int i; ///< row index
		unsigned int j; ///< column index
		unsigned int k; ///< layer index
		unsigned int n; ///< unique element number
	};

	/*! Determines the matrix type to be enumerated. */
	enum IndexGeneratorTypes{
		IGT_1D = 1, ///< One dimensional index and memory structures are used.
		IGT_2D = 2, ///< Two dimensional index and memory structures are used.
		IGT_3D = 3 ///< Three dimensional index and memory structures are used.
	};

	/*! Populates the index generator with elements and updates the mapping grid. */
	void setElements( const std::vector<Element> & elements, IndexGeneratorTypes type = IGT_2D );

	/*! Generates a vector of elements and sets it via setElements() for cartesian trivial 1D/2D or 3D grids.
		\param xNum Number of grid cells in x-direction, must be >= 1.
		\param yNum Number of grid cells in y-direction, must be >= 1.
		\param zNum Number of grid cells in z-direction, must be >= 1.
		\param type Overrides the results of the dimension deduce algorithm if deduced type is less then the enforced type given by this parameter. In this case an index vector of requested length (3 elements for 1D, 5 elements (2D) or 7 elements (3D)) is created.
	*/
	void createCartesianGrid(unsigned int xNum, unsigned int yNum = 1, unsigned int zNum = 1, IndexGeneratorTypes type = IGT_1D );

	/*! Dumps the grid (after call to setElements) to output stream. */
	void dumpGrid(std::ostream & out, unsigned int width, IndexGeneratorTypes type = IGT_2D ) const;

	/*! Generates ADI mapping vectors.
		\param adiX Vector of size m_elements.size() holding mappings from ADI X numbering to original numbering.
		\param adiY Vector of size m_elements.size() holding mappings from ADI Y numbering to original numbering.
	*/
	void generateADIMappings(std::vector<unsigned int> & adiX, std::vector<unsigned int> & adiY) const;

	/*! Generates sparse matrix index vectors suitable for use with SparseMatrix. Convenience function.
		\param indexArray Array of size m_elements.size()*m*m*5 (2D for now) holding indices.
		\param elementsPerRow Number of elements per row calculated in sparse matrix.
		\param m Block-size to consider.
	*/
	void generateSparseMatrixMappings( std::vector<unsigned int> & indexArray, unsigned int &elementsPerRow, unsigned int m ) const ;

	/*! Generates sparse matrix index vectors suitable for use with SparseMatrixCSR. Convenience function.
		\param rowOffset Position inside column index vector at the beginning of each row (ia).
		\param indexArray Array of size m_elements.size()*m*m*5 (2D for now) holding column indices (ja).
		\param m Block-size to consider.
	*/
	void generateSparseMatrixMappingsCSR(std::vector<unsigned int> & ia, std::vector<unsigned int> & ja, unsigned int m ) const ;

	/*! Transforms an index array so that scalar entries in the original array are expanded to blocks
		of dimension blockDim x blockDim.

		\code
		// A Matrix with pattern:
		//    0  1  2  3  4  5  6  7
		// 0 [1  2     3              ]
		// 1 [4  5  6     7           ]
		// 2 [   8  9  10    11       ]
		// 3 [12    13 14 15    16    ]
		// 4 [   16    17 18 19    20 ]
		// 5 [      21    22 23 24    ]
		// 6 [         25    26 27 28 ]
		// 7 [            29    30 31 ]
		//
		// has the index vector:
		//   [0 1 3 3 3]
		//   [0 1 2 4 4]
		//   [1 2 3 5 5]
		//   [0 2 3 4 6]
		//   [1 3 4 5 7]
		//   [2 4 5 6 6]
		//   [3 5 6 7 7]
		//   [4 6 7 7 7]
		//
		// which is expanded to (when blockDim = 3) the following index vector.
		//
		//   [0  1  2    3  4  5    9  10 11   11 11 11   11 11 11 ]
		//   [0  1  2    3  4  5    9  10 11   11 11 11   11 11 11 ]
		//   [0  1  2    3  4  5    9  10 11   11 11 11   11 11 11 ]
		//   [0  1  2    3  4  5    6  7  8    12 13 14   14 14 14 ]
		//   [0  1  2    3  4  5    6  7  8    12 13 14   14 14 14 ]
		//   [0  1  2    3  4  5    6  7  8    12 13 14   14 14 14 ]
		//   ...
		//   [12 13 14   18 19 20   21 22 23   23 23 23   23 23 23]
		//   [12 13 14   18 19 20   21 22 23   23 23 23   23 23 23]
		//   [12 13 14   18 19 20   21 22 23   23 23 23   23 23 23]

		\endcode
	*/
	static void scaleSparseMatrixIndexVector( const std::vector<unsigned int> & originalIndexArray, unsigned int elementsPerRow,
									   unsigned int blockDim, std::vector<unsigned int> & indexArray);

private:
	/*! Holds element information in arbitrary numbering.
		All elements must have the i, j and k parameters set, the
		remaining parameters are optional.
	*/
	std::vector<Element>				m_elements;

	/*! Map holding materials indices, used to determine neighborhood relationships.
		This matrix is populated in setElements().
	*/
	IBK::matrix<Element*>				m_grid;


	/*! Map holding materials indices, used to determine neighborhood relationships.
		This matrix is populated in setElements() in 3D cases.
	*/
	IBK::matrix_3d<Element*>			m_grid3D;

	/*! cached index generator type */
	IndexGeneratorTypes						m_type;

};

}

#endif // IBKMK_IndexGeneratorH
