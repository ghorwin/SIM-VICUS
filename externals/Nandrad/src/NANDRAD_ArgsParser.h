/*	The NANDRAD data model library.

	Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Anne Paepcke     <anne.paepcke -[at]- tu-dresden.de>

	This library is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This library is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

#ifndef NANDRAD_ArgsParserH
#define NANDRAD_ArgsParserH

#include <IBK_SolverArgsParser.h>

namespace NANDRAD {

/*! Re-implementation of IBK::SolverArgsParser to add NANDRAD-specific options. */
class ArgsParser : public IBK::SolverArgsParser {
public:
	/*! Adds NANDRAD-specific options. */
	ArgsParser();

	/*! Displays NANDRAD-specific help and examples. */
	virtual void printHelp(std::ostream & out) const override;
};

} // namespace NANDRAD

#endif // NANDRAD_ArgsParserH
