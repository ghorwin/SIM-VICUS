/*	DataIO library
	Copyright (c) 2001-2016, Institut fuer Bauklimatik, TU Dresden, Germany

	Written by A. Nicolai, St. Vogelsang
	All rights reserved.

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
*/

#ifndef DATAIO_GeoFileH
#define DATAIO_GeoFileH

#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <istream>
#include <ostream>

#include "DATAIO_ConstructionLines2D.h"

#include <IBK_Color.h>
#include <IBK_NotificationHandler.h>
#include <IBK_Path.h>
#include <IBK_matrix.h>
#include <IBK_matrix_3d.h>


/*! The namespace DATAIO holds all classes and functions related to reading/writing
	and converting DELPHIN simulation output data.
*/
namespace DATAIO {

class DataIO;

/*! Encapsulates the data of a geometry file.
	The geometry class contains all data needed to reconstruction the computational grid
	and its element and sides numbering. Currently, rectilinear grids are used.
	While data files (DataIO containers) can be read without Geometry file information, you need the geometry data
	for any meaningful graphical representation.

	The main page of the DataIO documentation contains an example on how to populate the GeoFile object
	when creating a geometry file, see \ref intro_sec. Some background is given in the publication referenced on
	this page.
*/
class GeoFile {
public:

	// *** TYPES ***

	/*! Coordinate direction of flux across side. */
	enum Direction {
		/*! Flux goes in X direction. */
		X_DIRECTION,
		/*! Flux goes in Y direction. */
		Y_DIRECTION,
		/*! Flux goes in Z direction. */
		Z_DIRECTION,
		/*! Invalid value for orientation_t */
		NUM_DIRECTIONS
	};

	/*! Contains the data entries in the material table of the geometry section. */
	struct Material {
		/*! Constructor. */
		Material() {}
		/*! Convenience constructor to create a material reference.
			\param n Unique material identifier, referenced from Element objects.
			\param c Material color (for display).
			\param nam Descriptive material name.
		*/
		Material(unsigned int n, const IBK::Color& c, const std::string& nam) :
			ID(n), color(c), name(nam) {}
		unsigned int	ID;		///< Unique material identifier, referenced from Element objects.
		IBK::Color		color;	///< Material color (for display).
		std::string		name;	///< Descriptive material name.
	};

	/*! Contains the grid data of the geometry section.
		\code
		// example: retrieve center coordinates of grid cell (colIdx, rowIdx, stackIdx)
		double x = grid.x_coords[colIdx];
		double y = grid.y_coords[rowIdx];
		double z = grid.z_coords[stackIdx];
		\endcode
	*/
	struct Grid {
		std::vector<double>			xwidths;			///< x-widths (column widths) of all elements in the grid in [m].
		std::vector<double>			ywidths;			///< y-widths (row heights) of all elements in the grid in [m].
		std::vector<double>			zwidths;			///< z-widths (stack depths) of all elements in the grid in [m]. If size() of vector == 1 and value[0] == 0 a rotation symmetric 3D grid is defined.
		std::vector<double>			x_coords;			///< x-coordinates of all element centers in [m].
		std::vector<double>			y_coords;			///< y-coordinates of all element centers in [m].
		std::vector<double>			z_coords;			///< z-coordinates of all element centers in [m].
		std::vector<double>			x_gridCoords;		///< x-coordinates of all grid lines parallel to Y/Z Axis in [m].
		std::vector<double>			y_gridCoords;		///< y-coordinates of all grid lines parallel to X/Z Axis in [m].
		std::vector<double>			z_gridCoords;		///< z-coordinates of all grid lines parallel to X/Y Axis in [m].


		/*! A value that identifies the original grid type represented by this geometry file: 0 = 1D, 1 = 2D, 2 = 3D rot sym, 3 = 3D.
			This value is updated based on the content of the Grid data structure (mainly the vectors xwidths, ywidths, zwidths)
			in the function updateConstructionType();
		*/
		int							constructionType;

		/*! Updates the constructionType variable based on current content in Grid.
			This function is called from parseGeometryData() and readBinaryGeometryData().
		*/
		void updateConstructionType();
	};

	/*! Contains the element data in the elements table of the geometry section.
		Kept for compatibility when reading D5 output files in binary mode. Table data is
		stored in records of size Element2D, therefore Element cannot be used.
		\warning Developer warning: if you modify the order/number of data entries reading existing
				 binary data files will not be possible anylonger.
	*/
	struct Element2D {
		unsigned int 	n;		///< Element number.
		double 			x;		///< Element center's x-coordinate in [m].
		double			y;		///< Element center's y-coordinate in [m].
		unsigned int	i;		///< Element's column index.
		unsigned int	j;		///< Element's row index.
		unsigned int	matnr;	///< Material number (the index in materials table).
	};

	/*! Contains the element data in the elements table of the geometry section.
		When reading/writing binary data this data structure is used as record.
		\warning Developer warning: if you modify the order/number of data entries/data types, reading existing
				 binary data files will not be possible anylonger.
	*/
	struct Element {
		Element() :
			n(0),
			x(0.0),
			y(0.0),
			z(0.0),
			i(0),
			j(0),
			k(0),
			matnr(0)
		{}

		/*! Convenience constructor. */
		Element(unsigned int n_, unsigned int i_, unsigned int j_, unsigned int k_, unsigned int matnr_) :
			n(n_),
			i(i_),
			j(j_),
			k(k_),
			matnr(matnr_)
		{}
		unsigned int 	n;		///< Element number.
		double 			x;		///< Element center's x-coordinate in [m].
		double			y;		///< Element center's y-coordinate in [m].
		double			z;		///< Element center's z-coordinate in [m].
		unsigned int	i;		///< Element's column index.
		unsigned int	j;		///< Element's row index.
		unsigned int	k;		///< Element's layer index.
		unsigned int	matnr;	///< Material ID (the index in materials table).
	};

	/*! Contains the sides data in the sides table of the geometry section.
		When reading/writing binary data of D5 output files this data structure is used as record.
		\warning Developer warning: if you modify the order/number of data entries/data types, reading existing
				 binary data files will not be possible anylonger.
	*/
	struct Side2D {
		unsigned int		n;				///< Number of the side.
		double				x;				///< X-Coordinate of the side's center point.
		double				y;				///< Y-Coordinate of the side's center point.
		unsigned int		i;				///< Grid line index in x direction for the side.
		unsigned int		j;				///< Grid line index in y direction for the side.
		bool				vertical;		///< Indicates whether the side is oriented in x (false), y (true) direction.
	};

	/*! Contains the sides data in the sides table of the geometry section.
		When reading/writing binary data files this data structure is used as record.
		\warning Developer warning: if you modify the order/number of data entries/data types, reading existing
				 binary data files will not be possible anylonger.
	*/
	struct Side {
		unsigned int		n;				///< Number of the side.
		double				x;				///< X-Coordinate of the side's center point.
		double				y;				///< Y-Coordinate of the side's center point.
		double				z;				///< Z-Coordinate of the side's center point.
		unsigned int		i;				///< Grid line index in x direction for the side.
		unsigned int		j;				///< Grid line index in y direction for the side.
		unsigned int		k;				///< Grid line index in z direction for the side.
		unsigned int		orientation;	///< Indicates whether the side is oriented in x=0, y=1 or z=2 axis direction. \sa Direction
	};

	/*! Contains data for construction lines in 3D. Lines can be vertical, horizontal or depth (stacked, diagonal etc.).
		horizontal lines:	position is y and z, begin and end are x-coordinates, kind is LK_Vertical_XZ
		vertical lines:		position is x and z, begin and end are y-coordinates, kind is LK_Horizontal_YZ
		depth lines:		position is x and y, begin and end are z-coordinates, kind is LK_Depth_XY
		Ther exist convinience functions for creating specific line types.
	*/
	struct ConstructionLine3D {
		enum LineKind3D {
			LK_Vertical_XZ,		///< Vertical line. Positions are x and z.
			LK_Horizontal_YZ,	///< Horizontal line. Positions are y and z.
			LK_Depth_XY			///< Depth line. Positions are x and y.
		};

		/*! Constructor for a construction line object.
			\param pos1 Postion of the line (x for vertical and depth and y for horizontal line.
			\param pos2 Postion of the line (z for vertical and horizontal and y for depth line.
			\param begin Start of the line (y-coordinate for vertical, x-coordinate for horizontal line and z-coordinate for depth line).
			\param end End of the line (y-coordinate for vertical, x-coordinate for horizontal line and z-coordinate for depth line).
			\param kind Flag for line kind (\sa LineKind3D).
		*/
		ConstructionLine3D(double pos1, double pos2, double begin, double end, LineKind3D kind) :
			m_pos1(pos1),
			m_pos2(pos2),
			m_begin(begin),
			m_end(end),
			m_kind(kind)
		{}

		/*! Convinience function for creating a vertical 3D construction line.*/
		static ConstructionLine3D constructionLine3DVertical(double xpos, double zpos, double ybegin, double yend) {
			return ConstructionLine3D(xpos, zpos, ybegin, yend, LK_Vertical_XZ);
		}

		/*! Convinience function for creating a horizontal 3D construction line.*/
		static ConstructionLine3D constructionLine3DHorizontal(double ypos, double zpos, double xbegin, double xend) {
			return ConstructionLine3D(ypos, zpos, xbegin, xend, LK_Horizontal_YZ);
		}

		/*! Convinience function for creating a depth 3D construction line.*/
		static ConstructionLine3D constructionLine3DDepth(double xpos, double ypos, double zbegin, double zend) {
			return ConstructionLine3D(xpos, ypos, zbegin, zend, LK_Depth_XY);
		}

		double		m_pos1;		///< First line position
		double		m_pos2;		///< Second line position
		double		m_begin;	///< Start coordinate of the line
		double		m_end;		///< End coordinate of the line
		LineKind3D	m_kind;		///< Line kind.
	};

	/*! Contains vectors for vertical, horizontal and depth construction lines. Can be created by using generateConstructionLines3D().*/
	struct ConstructionLines3D {
		std::vector<ConstructionLine3D> m_verticalLines;
		std::vector<ConstructionLine3D> m_horizontalLines;
		std::vector<ConstructionLine3D> m_depthLines;
	};



	// *** PUBLIC MEMBER FUNCTIONS ***


	/*! Default constructor, creates an empty and invalid GeoFile object. */
	GeoFile();

	/*! Function for reading geometry file.
		Throws an IBK::Exception in case of error.
		\param fname	The file path to the geometry file (utf8 encoded).
		\param notify	An optional pointer to a notification object, to be called repeatedly while read() is executed.
	*/
	void read(const IBK::Path & fname, IBK::NotificationHandler * notify = NULL);

	/*! Adjusts filename and extension based on binary flag. ASCII files receive the g6a extension, binary files
		get g6b as file extension. This function expects a filename to be set already, otherwise it
		throws an IBK::Exception with an error message.
	*/
	void adjustFileName();

	/*! Function for writing the geometry file.
		Writes to file with file path stored in m_filename.
		Throws an IBK::Exception in case of an error.
		\param notify	An optional pointer to a notification object, to be called repeatedly while write() is executed.
	*/
	void write(IBK::NotificationHandler * notify = NULL) const;

	/*! Returns the hash code for the current content of the GeoFile instance.
		Hash function uses current content in member variables.
		\return Unique hash code that identifies content of geometry file.
	*/
	unsigned int hashCode();

	/*! Checks if material indexes in Element data structures are always smaller than number of materials in Materials
		table.
		Throws an IBK::Exception if index is out of bounds.
	*/
	void checkForConsistantMaterialIndexes() const;

	/*! Resets everything. */
	void clear();


	// *** PUBLIC DATA MEMBERS ***

	std::vector< Material >				m_matDefs; 				///< The material definition table.
	Grid 								m_grid;	 				///< Grid information.
	std::vector< Element > 				m_elementsVec;			///< Element geometry data.
	std::vector< Side >					m_sidesVec;	 			///< Sides geometry data.
	ConstructionLines2D					m_constructionLines;	///< Construction lines.

	/*! True if the file was read/is to be written in binary mode. */
	bool								m_isBinary;

	/*! Filename of the geometry output file (utf8 encoded), set by read() and needed by write() functions.
		Naming convention:
		\code
			<projectFileName>_<project_file_hash_code>.d6g
		\endcode
	*/
	IBK::Path							m_filename;

	/*! Hashcode for the project file. */
	mutable std::string					m_projectFileHashCode;

private:

	/*! Writes geometry data in ASCII mode.
	*/
	void writeGeometryData(std::ostream& out, IBK::NotificationHandler * notify) const;

	/*! Writes geometry data in binary mode.
	*/
	void writeBinaryGeometryData(std::ostream& out) const;

	/*! Extracts the geometry data out of a list with lines.
		Throws an IBK::Exception in case of error.
		\param lines The list with lines containing the header and geometry section of the file
	*/
	void parseGeometryData(const std::vector<std::string> & lines);

	/*! Reads geometry data from binary output file.

		This function is used to read geometry files or the geometry section in old DELPHIN 5 Data files.
		\param in Input file stream, positioned at the begin of the geometry section.
	*/
	void readBinaryGeometryData(std::istream& in);

	/*! This function is called for DataIO files with version number less than 7, which used
		a column indexes running from top to bottom, yet element.m_y coordinates that goes from bottom to top.
		Also, to increase confusion, in files of this version number, the y-element/row heights are given also
		from top to bottom.
		This function fixes this.
	*/
	void fixYCoordinates();

	/*! Major version number of geometry file.
		The version number is read in read().
		The version number must be set explicitely when readBinaryGeometryData() is directly called.
	*/
	unsigned int m_majorFileVersion;

	// DataIO is a friend so that is can access readBinaryGeometryData
	friend class DataIO;
};

/*! \file DATAIO_GeoFile.h
	\brief Contains declaration for class GeoFile.
*/

} // namespace DATAIO

#endif // DATAIO_GeoFileH
