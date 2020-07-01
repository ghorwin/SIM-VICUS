// IBK Library - Collection of classes, functions and algorithms
//               for development of numeric simulations
//
// Copyright (C) 2007 Andreas Nicolai, Andreas.Nicolai@gmx.net
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.

#ifndef IBK_hardwareH
#define IBK_hardwareH

#if defined(_WIN32)
	#include <windows.h>
#endif

#include <string>
#include <vector>

namespace IBK {

/*! Return a vector of MAC adresses.*/
std::vector<std::string> getMacAddress();

/*! Return CPU ID if possible.*/
void cpuID(unsigned i, unsigned regs[4]    );

/*! Return a CPU vendor string.*/
std::string cpuVendor();

} // namespace IBK

#endif // IBK_hardwareH

