/*	Copyright (c) 2001-2017, Institut für Bauklimatik, TU Dresden, Germany

	Written by A. Nicolai, H. Fechner, St. Vogelsang, A. Paepcke, J. Grunewald
	All rights reserved.

	This file is part of the IBK Library.

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


	This library contains derivative work based on other open-source libraries.
	See OTHER_LICENCES and source code headers for details.

*/

#include "IBK_physics.h"

namespace IBK {

const double PI = 3.141592653589793238;
const double BOLTZMANN = 5.67e-08;
const double FARADAY = 96485.3415;
const double R_IDEAL_GAS = 8.314472;
const double DEG2RAD = 0.01745329252;
const double R_VAPOR = 461.89;
const double R_AIR = 287.1;

const double RHO_W = 1000;
const double RHO_AIR = 1.205;
const double RHO_ICE = 916.7;
const double T_DEFAULT = 293.15;
const double T_REF_23 = 296.15;
const double C_WATER = 4180;
const double C_ICE = 2108;
const double C_VAPOR = 2050;
const double C_AIR = 1006;
const double LAMBDA_WATER = 0.556;
const double LAMBDA_ICE = 2.33;
const double LAMBDA_AIR = 0.0262;
const double H_EVAP = 3.08e6;
const double H_FREEZE = -232417;
const double H_FREEZE_0C = 333500;
const double KELVIN_FACTOR = 1.0/(1000.0 * 462.0 * T_DEFAULT);
const double GASPRESS_REF = 101325;
const double GRAVITY = 9.807;
const double MIN_RH = 1e-10;
const double MIN_PC_T = -23.02585093 * RHO_W * R_VAPOR;
const double DV_AIR = 2.662e-5;
const double SIGMA_W = 7.6E-2;

/*! barrier constants */
const double MAX_RESISTANCE = 1e20;


} //namespace IBK
