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

#ifndef NANDRAD_LinearSplineParameterH
#define NANDRAD_LinearSplineParameterH

#include <string>

#include <IBK_LinearSpline.h>
#include <IBK_Unit.h>
#include <IBK_Path.h>

#include "NANDRAD_CodeGenMacros.h"

namespace NANDRAD {

/*!	Class LinearSplineParameter stores a linear spline curve, the corresponding parameter name
	and a unit name of both dependend and independend quantity (xUnit, yUnit).

	Note that the read and write functions perform unit conversions on the values, so that
	the linear spline actually holds values always in Base-SI units (according to the
	IBK Unit-Definition).

	So when reading a linear spline that defines temperatures in C, the actual spline will
	then contain the values in K.
*/
class LinearSplineParameter {
public:

	/*! Interpolation method to be used for this linear spline parameter. */
	enum interpolationMethod_t {
		I_CONSTANT,	// Keyword: constant
		I_LINEAR,	// Keyword: linear
		NUM_I
	};

	/*! How to treat the values in multi-year simulations. */
	enum wrapMethod_t {
		C_CONTINUOUS,	// Keyword: continuous		'Continuous data'
		C_CYCLIC,		// Keyword: cyclic			'Annual cycle'
		NUM_C
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	/*! Default Constructor. */
	LinearSplineParameter(){}

	/*! Constructor with values. */
	LinearSplineParameter(const std::string &name,
						  const interpolationMethod_t intPol,
						  const std::vector<double> &xVals,const std::vector<double> yVals,
						  const IBK::Unit &xUnit, const IBK::Unit &yUnit):
		m_name(name),
		m_interpolationMethod(intPol),
		m_xUnit(xUnit),
		m_yUnit(yUnit)
	{
		m_values.setValues(xVals, yVals);
	}

	/*! Constructor with data file . */
	LinearSplineParameter(const std::string &name,
						  const interpolationMethod_t intPol,
						  const IBK::Path &tsvFilePath):
		m_name(name),
		m_interpolationMethod(intPol),
		m_tsvFile(tsvFilePath)
	{
	}

	void readXML(const TiXmlElement * element);
	TiXmlElement * writeXML(TiXmlElement * parent) const;

	NANDRAD_COMP(LinearSplineParameter)

	/*! Function to check for correct parametrization of linear spline parameters.
		This function checks if the spline is actually defined, i.e. m_name is not empty.
		Then it tests, wether there is a valid tsv-filepath given, reads the according file and sets m_values and the units
		Then it is checked if the x and y value units can be converted to targetX and targetY units. This can be skipped by
		setting skipUnitChecks to true, which may be useful if the target units are not yet known.
		These target units must be SI base units.
		It then converts the input data into the target (base SI) units, using the convert2BaseUnits() function.

		Note: the m_xUnit and m_yUnit members are not modified here!
		Then, the spline is generated, hereby testing for monotonically increasing x-values.
		Finally, the y-value ranges are being checked.

		\note To ensure consistent object state, the x and y units are updated after the value conversion. Hence,
			if you need m_xUnit and m_yUnit to represent input/output units, do not call this function!
	*/
	void checkAndInitialize(const std::string & expectedName,
							const IBK::Unit & targetXUnit, const IBK::Unit & targetYUnit,
							const IBK::Unit & limitYUnit, double minYVal, bool isGreaterEqual,
							double maxYVal, bool isLessEqual, const char * const errmsg, bool skipUnitChecks=false);

	/*! Reads externally referenced tsv-file.
		m_tsvFile is expected to contain a valid absolute path, optionally with trailing ?n column identifier.
	*/
	void readTsv();

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Parameter name (in context of schedules used as scheduled quantity). */
	std::string				m_name;
	/*! Interpolation method to be used when computing values of this spline. */
	interpolationMethod_t	m_interpolationMethod = NUM_I;
	/*! Whether to wrap time around in multi-year simulations (cyclic use) or to assume continuous data. */
	wrapMethod_t			m_wrapMethod = NUM_C;
	/*! Data vectors including linear spline functionality (i.e. interpolation at any given value).
		When this class is used as data container, the x and y values are stored in the units m_xUnit and m_yUnit.
		However, after the call to checkAndInitialize(), the values are stored in the respective Base-SI units
		of the input/output units m_xUnit and m_yUnit. For example, if m_yUnit is 'C' (degree C), then the spline
		holds values in 'K' (degree Kelvin).
		\warning Do not call checkAndInitialize() on original data containers - always create a copy first!
	*/
	IBK::LinearSpline		m_values;
	/*! Unit of the x-values. */
	IBK::Unit				m_xUnit;
	/*! Unit of the y-values. */
	IBK::Unit				m_yUnit;
	/*! Path to tsv-file, the file will be read during checkAndInitialize() call.
		Path may have an appended column identifier, starting with 1 for the first data column after the time column.
	*/
	IBK::Path				m_tsvFile;


private:
	/*! Converts x and y values from display/input/output units (m_xUnit and m_yUnit) to their respective
		base SI units.

		\warning The m_xUnit and m_yUnit will not be changed and thus be inconsistent afterwards with
			the values in the m_values spline (similarly to IBK::Parameter).
	*/
	void convert2BaseUnits();

};

} // namespace NANDRAD

#endif // NANDRAD_LinearSplineParameterH
