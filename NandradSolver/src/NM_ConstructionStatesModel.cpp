/*	NANDRAD Solver Framework and Model Implementation.

	Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Anne Paepcke     <anne.paepcke -[at]- tu-dresden.de>

	This library is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 3 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.
*/

#include "NM_ConstructionStatesModel.h"

#include <algorithm>

#include <IBK_messages.h>

#include <NANDRAD_ConstructionInstance.h>
#include <NANDRAD_ConstructionType.h>
#include <NANDRAD_SolverParameter.h>
#include <NANDRAD_SimulationParameter.h>

#include "NM_KeywordList.h"

namespace NANDRAD_MODEL {


/*! A grid generation utility class.
	Written by Andreas Nicolai for THERAKLES from IBK/TU Dresden.
*/
class Mesh {
public:
	/*! Defines mesh type. */
	enum MeshType {
		Uniform,
		TanHSingle,
		TanHDouble
	};

	/*! Constructor, used to define a mesh type and its properties. */
	Mesh(const MeshType type, const double stretch = 1, const double ratio=1);

	/*! Populates the vector ds_vec with normalized element width based on the current
		mesh settings. The sum in vector ds_vec will be always 1. If a single-sided
		grid is used, element 0 of the vector will have the smallest size. */
	void generate(unsigned int n, std::vector<double> & ds_vec);

	/*! Generates a grid between coordinates x1 and x2 and stores the new element widths
		in vector dx_vec, and the element's center coordinates in vector x_vec. */
	void generate(const unsigned int n, const double x1, const double x2,
		std::vector<double> & dx_vec, std::vector<double> & x_vec);

	/*! Type of mesh.
		\sa MeshType. */
	MeshType t;

	/*! Mesh stretch factor/density (larger values - coarser grid). */
	double d;

	/*! Ratio between boundary element widths dx_0 / dx_{n-1}. */
	double r;
};


void ConstructionStatesModel::setup(const NANDRAD::ConstructionInstance & con,
									const NANDRAD::SimulationParameter & simPara,
									const NANDRAD::SolverParameter & solverPara)
{
	FUNCID(ConstructionStatesModel::setup);

	// cache pointers to input data structures
	m_con = &con;
	m_simPara = &simPara;
	m_solverPara = &solverPara;

	// we initialize only those variables needed for state calculation:
	// - grid and grid-element to layer association
	// - initial conditions/states

	// *** grid generation

	generateGrid();
	IBK::IBK_Message(IBK::FormatString("Construction is discretized with %1 elements.\n")
					 .arg(m_elements.size()), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
}


void ConstructionStatesModel::resultDescriptions(std::vector<QuantityDescription> & resDesc) const {

}


void ConstructionStatesModel::resultValueRefs(std::vector<const double *> & res) const {

}

const double * ConstructionStatesModel::resultValueRef(const QuantityName & quantityName) const {
	return nullptr;

}


void ConstructionStatesModel::yInitial(double * y) const {

}


int ConstructionStatesModel::update(const double * y) {

}


void ConstructionStatesModel::generateGrid() {
	FUNCID(ConstructionStatesModel::generateGrid);

	// content checks of construction type and construction instance have been done already

	// get pointer to construction type
	const NANDRAD::ConstructionType * conType = m_con->m_constructionType;
	// number of material layers
	size_t nLayers = conType->m_materialLayers.size();

	// Resize layer offsets
	m_materialLayerElementOffset.resize(nLayers,0);

	// for either cases (density == 0 and all others):
	// - discretize all material layers into several elements
	// vector containing all element widths
	std::vector<double> dx_vec;
	// vector containing all element midpoints
	std::vector<double> x_vec;
	// layer width
	double dLayer;
	// current total material width
	double x = 0;

	// retrieve parameters regulating grid generation
	double stretch = m_solverPara->m_para[NANDRAD::SolverParameter::SP_DISCRETIZATION_STRECH_FACTOR].value;
	double minDX = m_solverPara->m_para[NANDRAD::SolverParameter::SP_DISCRETIZATION_MIN_DX].value;
	unsigned int maxElementsPerLayer = 10;

	// case: density is set to 0
	// internal layers are used as elements directly
	// boundary layers are split into 2 elements
	if (stretch == 0.0) {
		// loop over all material layers
		for (unsigned int i=0; i<nLayers; ++i) {
			// update element offset
			m_materialLayerElementOffset[i] = dx_vec.size();
			// material layer width, valid thickness has been tested already
			dLayer = conType->m_materialLayers[i].m_thickness;
			// boundary material layers are split into 2 equal sizes elements
			if (i == 0 || i == nLayers - 1) {
				dx_vec.push_back(dLayer/2);
				dx_vec.push_back(dLayer/2);
				x_vec.push_back(x + dLayer/4);
				x_vec.push_back(x + dLayer*3.0/4);
			}
			else {
				// internal material layers are used as elements directly
				dx_vec.push_back(dLayer);
				x_vec.push_back(x + dLayer/2);
			}
			// update current material width
			x += dLayer;
		}
	}

	// Case: m_density == 1, attempt equidistant discretization with approximately
	// element width as set by DiscMinDx parameter
	else {
		if (stretch == 1.0) {
			// loop over all material layers
			for (unsigned int i=0; i<nLayers; ++i) {
				// update element offset
				m_materialLayerElementOffset[i] = dx_vec.size();
				// material layer width, valid thickness has been tested already
				dLayer = conType->m_materialLayers[i].m_thickness;
				// calculate number of discretized elements for current material layer
				double n_x = dLayer / minDX;
				// compute the size of the last element
				double x_last = dLayer - (n_x - 1) * minDX;
				double dx_new;
				// if the last element gets very small, neglect this element
				if (x_last < minDX)
					n_x -= 1;
				// final element width
				dx_new = dLayer / n_x;
				// save elements in dx_vec and x_vec, and update current material width
				for (unsigned int e=0; e<n_x; ++e){
					dx_vec.push_back(dx_new);
					x_vec.push_back(x + dx_new/2);
					x += dx_new;
				}
			}
		}
		// all other cases: use stretching function to get variable discretization grid
		else {
			Mesh grid(Mesh::TanHDouble, stretch); // double-sided grid

			// determine required min dx by enforcing at least 3 elements per layer
			double rmin_dx = 1;
			for (unsigned int i=0; i<nLayers; ++i) {
				rmin_dx = std::min(rmin_dx, conType->m_materialLayers[i].m_thickness/3.0); // three elements per layer
			}
			// TODO : adjust min_dx or skip layer if very thin

			std::vector<double> xElem;
			std::vector<double> dxElem;
			// loop over all layers
			for (unsigned int i=0; i<nLayers; ++i) {
				// store offset of element number for the current layer
				// m_materialLayerElementOffset[i] = xElem.size();
				m_materialLayerElementOffset[i] = dx_vec.size();

				unsigned int n = 2;	// start with 2 elements per layer
				dLayer = conType->m_materialLayers[i].m_thickness;
				// repeatedly refine grid for this layer until our minimum element width at the boundary is no longer exceeded
				do {
					++n;
					grid.generate(n, x, x + dLayer, dxElem, xElem);
					if (dxElem[0] <= 1.1*minDX) break;
				} while (n < maxElementsPerLayer); // do not go beyond maximum element count
				if (n >= maxElementsPerLayer) {
					Mesh grid2(grid);
					do {
						grid2.d *= 1.2;
						grid2.generate(n, x, x + dLayer, dxElem, xElem);
					} while (dxElem[0] > 1.1*minDX && n < maxElementsPerLayer*2); // do not go beyond maximum element count
					IBK::IBK_Message( IBK::FormatString("Maximum number of elements per layer is reached in material "
														"layer #%1 (d=%2m), stretch factor increased to %3")
									  .arg(i+1).arg(dLayer).arg(grid2.d), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
				}
				// insert into into global discretization vector
				x_vec.insert(x_vec.end(), xElem.begin(), xElem.end() );
				dx_vec.insert(dx_vec.end(), dxElem.begin(), dxElem.end() );
				x += dLayer;
			}
		}
	}

	// total number of discretized elements
	m_nElements = dx_vec.size();
	// total number of unkowns
	if (m_moistureBalanceEnabled)
		m_n = m_nElements*2;
	else
		m_n = m_nElements;
	// total construction width
	m_constructionWidth = x;

	m_materialLayerElementOffset.push_back(m_nElements);

	// compute weight factors for coefficient averaging
	for (unsigned int i=0; i<m_nElements; ++i) {
		double wL, wR;
		// left weight factors
		if (i == 0)
			// left boundary element
			wL = 1;
		else
			// internal elements
			wL = dx_vec[i]/(dx_vec[i-1] + dx_vec[i]);
		// right weight factors
		if (i == m_nElements-1)
			// right boundary element
			wR = 1;
		else
			// internal elements
			wR = dx_vec[i]/(dx_vec[i] + dx_vec[i+1]);
		// add to element vector
		m_elements.push_back(Element(i, x_vec[i], dx_vec[i], wL, wR));
	}
}


// Mesh implementation

Mesh::Mesh(MeshType type, double stretch, double ratio)
	: t(type), d(stretch), r(ratio)
{
}

void Mesh::generate(unsigned int n, std::vector<double> & ds_vec) {
	double s = 0;
	ds_vec.resize(n);
	for (unsigned int i=1; i<n; ++i) {
		double xi = double(i)/n;
		double s_next;
		switch (t) {
			case Uniform :
				s_next = xi;
				break;

			case TanHSingle :
				s_next = 1 + tanh(d*(xi-1))/tanh(d);
				break;

			case TanHDouble :
			{
				double A = r*r;
				double u = 0.5*(1 + tanh(d*(xi-0.5))/tanh(d/2));
				s_next = u/(A + (1-A)*u);
			} break;
		}
		ds_vec[i-1] = s_next-s;
		s = s_next;
	}
	// compute last element such, that sum(ds) = 1
	ds_vec[n-1] = 1 - s;
}

void Mesh::generate(const unsigned int n, const double x1, const double x2,
	std::vector<double> & dx_vec, std::vector<double> & x_vec)
{
	generate(n, dx_vec); 	// dx_vec now holds normalized element widths
	x_vec.resize(n);
	double L = x2 - x1; 	// can be negative, if working left to right
	double fabsL = fabs(L);
	double x = x1;
	for (unsigned int i=0; i<n; ++i) {
		double next_x = x + L*dx_vec[i];
		x_vec[i] = 0.5*(x + next_x);
		dx_vec[i] *= fabsL; // scale element widths, all widths are always positive
		x = next_x;
	}
	// if L is negative, we reverse the vector
	if (L < 0) {
		std::reverse(x_vec.begin(), x_vec.end());
		std::reverse(dx_vec.begin(), dx_vec.end());
	}
	// The element's center coordinates are always sorted increasing.
	// with dx_vec[0] = 2*(x_vec[0]-x1)
}


} // namespace NANDRAD_MODEL

