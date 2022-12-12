#include <algorithm>
#include <sstream>
#include <fstream>
#include <limits>
#include <stdexcept>
#include <iostream>

#include "LinearSpline.h"
#include "Helpers.h"

LinearSpline::LinearSpline() :
	m_extrapolationMethod(EM_Constant),
	m_xMin(0),
	m_xMax(0),
	m_xStep(0),
	m_xOffset(0),
	m_valid(false)
{
}

void LinearSpline::eliminateConsecutive(const std::vector<double>& tmp_x,
	const std::vector<double>& tmp_y,
	std::vector<double>& tmp2_x,
	std::vector<double>& tmp2_y)
{
	// eliminate consecutive x values (stem from missing accuracy when reading the file)
	tmp2_x.clear();
	tmp2_y.clear();
	tmp2_x.push_back(tmp_x.front());
	tmp2_y.push_back(tmp_y.front());
	unsigned int skipped_values = 0;
	double last_reported_duplicate = std::numeric_limits<double>::infinity();
	for (unsigned int i = 1, size = static_cast<unsigned int>(tmp_x.size()); i<size; ++i) {
		if (tmp_x[i] != tmp2_x.back()) {
			tmp2_x.push_back(tmp_x[i]);
			tmp2_y.push_back(tmp_y[i]);
		}
		else {
			if (tmp2_x.back() != last_reported_duplicate) {
				last_reported_duplicate = tmp2_x.back();
			}
			++skipped_values;
		}
	}
	// If the spline ends with a zero slope line sets y value to the original end y value
	if (tmp_x.back() == tmp2_x.back() && tmp_y.back() != tmp2_y.back()) {
		tmp2_y.back() = tmp_y.back();
	}
}


unsigned int LinearSpline::size()  const {
	return m_y.size();
}


void LinearSpline::setValues(const std::vector<double> & xvals, const std::vector<double> & yvals) {
	if (xvals.size() != yvals.size())
		throw std::runtime_error("X and Y vector size mismatch.");
	if (xvals.empty())
		throw std::runtime_error("Input vectors are empty.");
	std::vector<double> tmp_x;
	std::vector<double> tmp_y;
	eliminateConsecutive(xvals, yvals, tmp_x, tmp_y);
	if (tmp_x.empty())
		throw std::runtime_error("Input vectors are empty.");
	setValues(tmp_x.begin(), tmp_x.end(), tmp_y.begin());
	std::string errstr;
	m_valid = makeSpline(errstr);
	if (!errstr.empty()) {
		/// \todo Check this, shouldn't this "warning" be handled as exception instead?
		throw std::runtime_error("Error while makeSpline in setValues: " + errstr);
	}
}


void LinearSpline::readTsv(const Path &fpath) {

	// Note: we may have a path like '/data/tsv/values.tsv?3' with column indiator

	Path filename = Path::fromURI(fpath.c_str());
	// extract column identifier, if any
	unsigned int colIndex = 1; // 1 means "first data column" (actually 2nd column in file, with 0-based index 1)
	std::string filenameStr = fpath.str();
	std::size_t pos = filenameStr.find('?');
	if (pos != std::string::npos) {
		try {
			std::string str(filenameStr.substr(pos + 1));
			std::stringstream strm(str);
			if (!(strm >> colIndex))
				throw std::runtime_error("Could not convert " + str + " into value.");
		}
		catch (...) {
			throw std::runtime_error("Malformed file name  " + filenameStr + " (invalid column indicator).");
		}
		filename = filenameStr.substr(0, pos);
	}

	// this is now the path without column indicator
	Path tsvFilePath(filename);

	if (!Path(tsvFilePath).exists())
		throw std::runtime_error("File '"+ tsvFilePath.str() + "' does not exist.");
	std::vector<std::vector<double> > rowData;
	// this is now the path without column indicator
	try {
#if defined(_WIN32) && !defined(__MINGW32__)
		std::ifstream in(tsvFilePath.wstr().c_str());
#else // _WIN32
		std::ifstream in(tsvFilePath.c_str());
#endif
		if (!in)
			throw std::runtime_error("File doesn't exist or cannot open/access file.");

		std::string line;
		std::getline(in, line);
		std::string sepChars;
		sepChars.push_back('\t');
		sepChars.push_back(',');
		std::vector<std::string> captions;
		explode(line, captions, sepChars);

		unsigned int nColumns = captions.size();
		if (nColumns <= colIndex) {
			std::stringstream strm;
			strm << colIndex + 1;
			throw std::runtime_error("File '" + tsvFilePath.str() + "' must have exactly " + strm.str() + " columns.");
			// Mind: column count = 1 (time column) + colIndex
		}
		// extract unit from caption
		std::string timeStr = captions[0];
		std::vector<std::string> tokens;
		// try to exclude index notation
		explode(timeStr, tokens, "[");
		if (tokens.size() < 2) {
			throw std::runtime_error("Missing time unit in file '" + tsvFilePath.str() + "'.");
		}
		std::string timeUnit = tokens[1];
		trim(timeUnit, "] ");
		// check unit
		std::string baseUnit;
		double testVal = 0.0;
		convertToBaseUnit(testVal, baseUnit, timeUnit);
		// error in conversion
		if (baseUnit != "s") {
			throw std::runtime_error("Invalid time unit '" + timeUnit + "in file '" + tsvFilePath.str() + "'.");
		}

		std::string valueStr = captions[colIndex];
		// try to exclude index notation
		explode(valueStr, tokens, "[");
		if (tokens.size() < 2) {
			throw std::runtime_error("Missing value unit in file '" + tsvFilePath.str() + "'.");
		}
		std::string valueUnit = tokens[1];
		trim(valueUnit, "] ");

		while (std::getline(in, line)) {
			if (line.empty() || line.find_first_not_of("\n\r\t ") == std::string::npos)
				continue;
			std::vector<std::string> tokens;
			explode(line, tokens, sepChars);
			// trim tokens
			if (std::find(tokens.begin(), tokens.end(), ",") != tokens.end()) {
				for (unsigned int i = 0; i < tokens.size(); ++i) {
					trim(tokens[i], " \t\r\"");
				}
			}

			// error: wrong column size
			if (tokens.size() != nColumns) {
				throw std::runtime_error("Wrong number of columns!");
			}
			std::vector<double> values(nColumns);
			for (unsigned int i = 0; i<nColumns; ++i) {
				double val = string2val<double>(tokens[i]);
				values[i] = val;
			}
			rowData.push_back(values);
		}
		// convert to column data
		std::vector<double > timeData, colData;
		for (unsigned int j = 0; j<rowData.size(); ++j) {
			convertToBaseUnit(rowData[j][0], baseUnit, timeUnit);
			convertToBaseUnit(rowData[j][colIndex], baseUnit, valueUnit);
			timeData.push_back(rowData[j][0]);
			colData.push_back(rowData[j][colIndex]);
		}
		setValues(timeData, colData);
	}
	catch (std::runtime_error & ex) {
		throw std::runtime_error("Error reading file " + tsvFilePath.str() + ": " + ex.what());
	}
	if (rowData.size() < 2)
		throw std::runtime_error("File " + tsvFilePath.str() + " must have at least 2 rows.");

}


void LinearSpline::clear() {
	*this = LinearSpline();
}


double LinearSpline::value(double x) const {

	if (m_xStep != 0) {
		if (x>m_xMax) {
			// use extrapolation
		}
		if (x<m_xMin) {
			// use extrapolation
		}
		/// \todo Check this implementation!
		// compute offset
		unsigned int i=(unsigned int)((x-m_xOffset)/m_xStep);
		// ensure that offset is in the valid range
		if (i >= m_slope.size())
			return m_y.back();
		return m_y[i] + m_slope[i]*(x - i*m_xStep);
	}

	if (m_x.size() == 1)
		return m_y[0];
	// x value larger than largest x value?
	if (x > m_x.back()) {
		switch (m_extrapolationMethod) {
			case EM_Constant	: return m_y.back();
			case EM_Linear		: return m_y.back() + m_slope.back()*(x-m_x.back());
		}
	}

	// we use lower bound to find the correct interval
	std::vector<double>::const_iterator it = std::lower_bound(m_x.begin(), m_x.end(), x);
	if (it == m_x.begin()) {
		switch (m_extrapolationMethod) {
			case EM_Constant	: return m_y.front();
			case EM_Linear		: return m_y.front() + m_slope.front()*(x-m_x.front());
		}
	}
	// get the index of the lower limit of the current interval
	unsigned int i = static_cast<unsigned int>(std::distance(m_x.begin(), it) - 1);
#ifdef USE_SLOPE
	return m_y[i] + m_slope[i]*(x - m_x[i]);
#else
	double alpha = (x - m_x[i])/(m_x[i+1]-m_x[i]); // thus must be always between 0 and 1
	return m_y[i]*(1-alpha) + m_y[i+1]*alpha;
#endif
}


double LinearSpline::nonInterpolatedValue(double x) const {

	if (m_x.size() == 1)
		return m_y[0];
	// x value larger than largest x value?
	if (x > m_x.back())
		return m_y.back();
	// we use lower bound to find the correct interval
	std::vector<double>::const_iterator it = std::lower_bound(m_x.begin(), m_x.end(), x);
	if (it == m_x.begin())
		return m_y.front();
	// get the index of the lower limit of the current interval
	unsigned int i = static_cast<unsigned int>(std::distance(m_x.begin(), it) - 1);
	// special case: x is a saltus of the non-interpolated spline
	// (than equals x the element m_x[i+1])
	if( i < m_x.size() - 1 && x == m_x[i+1] )
		return m_y[i+1];

	return m_y[i];
}


double LinearSpline::slope(double x) const {
	if (!m_valid)
		throw std::runtime_error("Linear spline not properly initialized!");
	if (m_x.size() == 1)
		return 0;
	// lower_bound -> i = 0...n   -> subtract 1 to get the slope index
	// max i = n-2 for slope
	size_t i = std::lower_bound(m_x.begin(), m_x.end(), x) - m_x.begin();
	if (i <= 1)					return m_slope.front();
	else if (i < m_x.size()-1)	return m_slope[--i];
	else						return m_slope.back();
}

// *** PRIVATE FUNCTIONS ***

bool LinearSpline::makeSpline(std::string & errMsg) {
	m_valid = false;
	m_slope.clear();
	if (m_x.empty() || m_x.size() != m_y.size()) {
		errMsg = std::string("Invalid vector dimensions.");
		return false;
	}
	if (m_x.size() == 1) {
		// special case, constant spline
		m_valid = true;
		return true;
	}
	// check for strict monotonic increasing values, so if we find any
	// adjacent values that fulfil x[i] >= x[i+1], we have a problem
	std::vector<double>::iterator posIt = std::adjacent_find(m_x.begin(), m_x.end(), std::greater_equal<double>());
	if (posIt != m_x.end()) {
		errMsg = std::string("X values are not strictly monotonic increasing.");
		return false;
	}
	for (unsigned int i=1; i<m_x.size(); ++i)
		m_slope.push_back( (m_y[i] - m_y[i-1])/(m_x[i]-m_x[i-1]) );
	m_valid = true;
	return true;
}


LinearSpline::SplineGenerationResults LinearSpline::generate(
		double xMin,
		double xMax,
		double /*absTol*/,
		double /*relTol*/,
		unsigned int maxPoints)
{
	// *** PART 1 - Initialization ***

	// adjust xMin and xMax based on existing m_x values
	if (!m_x.empty()) {
		xMin = std::min(xMin, m_x.front());
		xMax = std::max(xMax, m_x.back());
	}

	if (xMin >= xMax)
		throw std::runtime_error("Invalid interval (xMin must be < than xMax)!");

	// create temporary copy of vectors
	std::vector<double> xvals = m_x;
	std::vector<double> yvals = m_y;
	xvals.reserve(maxPoints);
	yvals.reserve(maxPoints);

	if (xvals.size() > maxPoints) {
		m_x.swap(xvals);
		m_y.swap(yvals);
		return SG_MaxPointsExceeded;
	}
	else {
		m_x.swap(xvals);
		m_y.swap(yvals);
		return SG_Ok;
	}
}

