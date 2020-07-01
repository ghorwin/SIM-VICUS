#ifndef DATAIO_ConstructionLines2DH
#define DATAIO_ConstructionLines2DH

#include <vector>

#include <IBK_Unit.h>
#include <IBK_matrix.h>

namespace DATAIO {

class GeoFile;

/*! Contains data for construction lines in 2D. The lines can be horizontal or vertical.
	horizontal lines:	position is y, begin and end are x-coordinates, xpos is false
	vertical lines:		position is x, begin and end are y-coordinates, xpos is true
*/
struct ConstructionLine2D {
	/*! Constructor for a construction line object.
		\param pos Postion of the line (x for vertical and y for horizontal line.
		\param begin Start of the line (y-coordinate for vertical and x-coordinate for horizontal line).
		\param end End of the line (y-coordinate for vertical and x-coordinate for horizontal line).
	*/
	ConstructionLine2D(double pos, double begin, double end) :
		m_pos(pos),
		m_begin(begin),
		m_end(end)
	{}

	double	m_pos;		///< Line position
	double	m_begin;	///< Start coordinate of the line
	double	m_end;		///< End coordinate of the line

	/*! Not equal operator.*/
	friend bool operator!=(const ConstructionLine2D& lhs, const ConstructionLine2D& rhs) {
		if(lhs.m_pos != rhs.m_pos)
			return true;
		if(lhs.m_begin != rhs.m_begin)
			return true;
		if(lhs.m_end != rhs.m_end)
			return true;

		return false;
	}

	/*! Not equal operator.*/
	friend bool operator==(const ConstructionLine2D& lhs, const ConstructionLine2D& rhs) {
		if(lhs.m_pos != rhs.m_pos)
			return false;
		if(lhs.m_begin != rhs.m_begin)
			return false;
		if(lhs.m_end != rhs.m_end)
			return false;

		return true;
	}
};

/*! Contains vectors for vertical and horizontal construction lines.
   Can be created by using GeoFile::generateConstructionLines().*/
class ConstructionLines2D
{
public:
	/*! Standard constructor. Default for unit is m (id 1 \sa IBK::UnitList). The default unit will be set at x and y coordinate.*/
	ConstructionLines2D(const IBK::Unit& unit = IBK::Unit(1));

	/*! Add a vertical construction line given by x-position and y start and end.
		isBoundary flag set the line kind.
	*/
	void addVerticalLine(double xpos, double ystart, double yend, bool isBoundary);

	/*! Add a vertical construction line given by x-position and y start and end.
		isBoundary flag set the line kind.
	*/
	void addHorizontalLine(double ypos, double xstart, double xend, bool isBoundary);

	/*! Clears all internal vectors.*/
	void clear();

	/*! Clear all horizontal lines.*/
	void clearHorizontal();

	/*! Clear all vertical lines.*/
	void clearVertical();

	/*! Return true if all internal vertical vectors are empty.*/
	bool emptyVertical() const;

	/*! Return true if all internal horizontal vectors are empty.*/
	bool emptyHorizontal() const;

	/*! Return true if all internal vectors are empty.*/
	bool empty() const;

	/*! Converts all data into the given unit. The unit must be a coordinate unit based on m.*/
	void convertUnit(const IBK::Unit& newXUnit, const IBK::Unit& newYUnit);

	/*! Swap horizontal to vertical lines in order to fulfill chart direction definition.*/
	void swapLineDirection();

	/*! Generates construction and boundary lines from the given geometry file. */
	void generate(const GeoFile & geofile);

	/*! Processes all horizontal construction and boundary lines and sets the x values to the given limits.
		This is needed when x-unit is time (even though m_xunit is still m).
	*/
	void adjustHorizontalLineEnds(double xMin, double xMax);

	/*! Comparison function.*/
	bool notEqual(const ConstructionLines2D& rhs) const;

	/*! Not equal operator.*/
	friend bool operator!=(const ConstructionLines2D& lhs, const ConstructionLines2D& rhs) {
		return lhs.notEqual(rhs);
	}

	std::vector<ConstructionLine2D> m_verticalConstructionLines;
	std::vector<ConstructionLine2D> m_horizontalConstructionLines;
	std::vector<ConstructionLine2D> m_verticalBoundaryLines;
	std::vector<ConstructionLine2D> m_horizontalBoundaryLines;

private:

	IBK::Unit						m_xunit;
	IBK::Unit						m_yunit;
};

} // namespace DATAIO

#endif // DATAIO_ConstructionLines2DH
