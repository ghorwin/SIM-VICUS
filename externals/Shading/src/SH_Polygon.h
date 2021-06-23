#ifndef TH_PolygonH
#define TH_PolygonH

#include <string>

#include <IBKMK_Vector3D.h>
#include <IBK_matrix.h>
#include <IBK_LinearSpline.h>

namespace SH {

class Polygon {
public:
	Polygon(){}

	Polygon(const std::vector<IBKMK::Vector3D> & polyline):
		m_id( (std::numeric_limits<unsigned int>::max)() ),
		m_polyline(polyline)
	{}

	Polygon(unsigned int id, const std::vector<IBKMK::Vector3D> & polyline):
		m_id(id),
		m_polyline(polyline)
	{}


	// *** PUBLIC MEMBER FUNCTIONS ***

	static void writeLine(const std::string &message);

	/*! calculate the surface size of konvex and non konvex polygons*/
	double calcSurfaceSize();

	/*! calcs the normal of a polygon
			positve normal is in rotation of polygon points
			right hand rule
			\param normalize standard value true
	*/
	IBKMK::Vector3D calcNormal(bool normalize = true);

	/*! calculates distance point to plane*/
	double distancePointToPlane(const IBK::point3D<double> & p0);

	/*! Point in Polygon function. Result:
		-1 point not in polyline
		0 point on polyline
		1 point in polyline

		\param	point test point
		Source https://de.wikipedia.org/wiki/Punkt-in-Polygon-Test_nach_Jordan

	*/
	int pointInPolygon(const IBKMK::Vector3D & point);

	/*! Calculates the rotation matrix for an original straight line and a rotation angle. */
	static IBK::matrix<double> rotationMatrixForStraightOrigin(
			const double & cosRotationAngle,
			const IBKMK::Vector3D & nStraightOrigin,
			bool positiveRota);

	/*! Calculates the rotation matrix for an original straight line and a rotation angle. */
	IBK::matrix<double> rotationMatrixForStraightOrigin(
			const double & sinRotationAngle,
			const IBKMK::Vector3D & nStraightOrigin);

	/*! Rotate one Point of the polygon (index) by a rotation matrix. */
	IBKMK::Vector3D rotaMatrixMultiplyVector(const IBK::matrix<double> & rotaM, size_t idx);

	/*! First rotate polyline by rotation matrix. Second shift the polyline. */
	void rotateAndShiftPolyline(const IBK::matrix<double> &rotaM, const IBKMK::Vector3D &translation);

	/*! Rotate the hole polyline. */
	void rotatePolyline(const IBK::matrix<double> &rotaM);

	/*! Shift the polygon for rotation to the translation point.*/
	void rotatePolyline(const IBKMK::Vector3D & rotaAxis, double cosRotaAngle,const IBKMK::Vector3D &translation = IBKMK::Vector3D(0,0,0));

	/*! Changes the direction of the normals by changing the order of the points. */
	void changeNormalDirection();

	/*! Adds a point at the end of the polyline. */
	void operator<<(const IBKMK::Vector3D & other) {
		this->m_polyline.push_back(other);
	}

	// *** PUBLIC MEMBER VARIABLES ***

	unsigned int							m_id;

	std::string								m_displayName;

	std::vector<IBKMK::Vector3D>			m_polyline;

	/*! Linear spline (x = time in seconds, y = z value = percentage of area still visible to the sun) given by shading calculation*/
	IBK::LinearSpline						m_shadingFactors;

private:

	// *** PRIVATE MEMBER FUNCTIONS ***

	/*! remove points in a line*/
	void removePointsInLine();

	/*! remove duplicate points in series */
	void removeDuplicatePoints();
};


} // namespace SH

#endif // TH_PolygonH
