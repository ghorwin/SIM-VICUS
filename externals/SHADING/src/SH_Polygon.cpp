#include "SH_Polygon.h"

#include "IBK_math.h"


#include <fstream>

namespace SH {

double Polygon::calcSurfaceSize()
{
	FUNCID(Polygon::calcSurfaceSize);
	removePointsInLine();
	IBKMK::Vector3D surfSize = m_polyline.back().crossProduct(m_polyline[0]);

	for (size_t i=0; i<m_polyline.size()-1; ++i) {
		surfSize += m_polyline[i].crossProduct(m_polyline[i+1]);
	}
	return surfSize.magnitude()*0.5;
}

IBKMK::Vector3D Polygon::calcNormal(bool normalize)
{
	FUNCID(Polygon::calcNormal);

	if(m_polyline.size()<3)
		throw	IBK::Exception(IBK::FormatString( "Polyline has less then 3 Points." ), FUNC_ID);

	removePointsInLine();
	//Newell Verfahren zum Bestimmen der Normalen
	//n x + n y + n z + D = 0

	IBK::point3D<double> normal;
	for (size_t i=0; i<m_polyline.size(); ++i) {
		size_t polySize = m_polyline.size();
		size_t i1 = (i+1)%polySize;

		normal.m_x += (m_polyline[i].m_z+m_polyline[i1].m_z)*(m_polyline[i].m_y-m_polyline[i1].m_y);
		normal.m_y += (m_polyline[i].m_x+m_polyline[i1].m_x)*(m_polyline[i].m_z-m_polyline[i1].m_z);
		normal.m_z += (m_polyline[i].m_y+m_polyline[i1].m_y)*(m_polyline[i].m_x-m_polyline[i1].m_x);
	}

	if (normalize && IBKMK::Vector3D() != normal)
	{
		IBKMK::Vector3D n = normal;
		n.normalize();
		return n;
	}

	return normal;
}

void Polygon::writeLine(const std::string &message){

	std::ofstream logFile;
	logFile.open("c:/temp/logHotel.txt", std::ios_base::app);
	logFile << message << std::endl;
	logFile.close();
}

double Polygon::distancePointToPlane(const IBK::point3D<double> & p0)
{
	const IBKMK::Vector3D n = this->calcNormal();
	double d =(n.m_x*m_polyline[0].m_x + n.m_y*m_polyline[0].m_y +n.m_z*m_polyline[0].m_z);
	double dist = (n.m_x* p0.m_x + n.m_y* p0.m_y + n.m_z* p0.m_z - d) / n.magnitude();
	return dist;
}


int crossProdTest(IBKMK::Vector3D a, IBKMK::Vector3D b, IBKMK::Vector3D c){

	if(b.m_y> c.m_y){
		IBKMK::Vector3D temp;
		temp = c;
		c=b;
		b=temp;
	}

	if (a.m_y <= b.m_y || a.m_y > c.m_y) {
		return 1;
	}

	double delta = (b.m_x - a.m_x) * (c.m_y - a.m_y) -(b.m_y - a.m_y) * (c.m_x - a.m_x);
	if(delta > 0)			return	1;
	else if(delta < 0)		return	-1;
	else					return	0;
}

int Polygon::pointInPolygon(const IBKMK::Vector3D &point)
{
	//rotate polyline to a coordinate-plane

	//get normal of polygon
	IBKMK::Vector3D n = calcNormal();
	IBKMK::Vector3D zAxis(0,0,1);

	double cosAlpha = n.scalarProduct(zAxis);
	//überlegen wenn cosAlpha 0 dann keine Rotation, 180° Rotation auch nicht nötig
	IBKMK::Vector3D rotaAxis = n.crossProduct(zAxis);

	//nur wenn ein Vector entsteht muss gedreht werden, bei Null-Vector keine Drehung nötig
	IBK::matrix<double> rotaMatrix(3,3,0), backRotaMatrix(3,3,0);
	std::vector<IBKMK::Vector3D>	originalPolyline = m_polyline;
	IBKMK::Vector3D pRota(point);
	if(rotaAxis.magnitude()>0.1){
		//rotationsmatrix
		rotaMatrix = rotationMatrixForStraightOrigin(cosAlpha, rotaAxis, true);
		backRotaMatrix = rotationMatrixForStraightOrigin(cosAlpha, rotaAxis, false);
		//add point for rotation
		m_polyline.push_back(point);

		//m_polyline rotieren
		rotatePolyline(rotaMatrix);

		pRota = m_polyline.back();

		//delete point
		m_polyline.pop_back();
	}


	double t=-1;
	for (size_t i=0; i<m_polyline.size(); ++i) {
		t *= crossProdTest(pRota, m_polyline[i], m_polyline[(i+1)%m_polyline.size()]);
		if(t==0)
			break;
	}
	this->m_polyline.swap(originalPolyline);
	return  t;
}

IBK::matrix<double> Polygon::rotationMatrixForStraightOrigin(const double & cosRotationAngle,
															 const IBKMK::Vector3D & nStraightOrigin,
															 bool positiveRota)
{
	IBKMK::Vector3D nOri =nStraightOrigin;

	IBK::matrix<double> rotaM(3,3,0);

	//no rotation
	if(nOri == IBKMK::Vector3D(0,0,0)){
		//return Identity matrix
		rotaM(0,0) = rotaM(1,1) = rotaM(2,2) = 1;
		return rotaM;
	}
	else
		//Rotation
		nOri.normalize();

	int sign = 1;
	if(!positiveRota)
		sign =-1;
	double sinRotationAngle = sign * IBK::f_sqrt(std::max<double>(0, 1- cosRotationAngle*cosRotationAngle));

	double tmpVal1 = 1-cosRotationAngle;

	rotaM(0,0) = nOri.m_x * nOri.m_x * tmpVal1 + cosRotationAngle;
	rotaM(1,0) = nOri.m_x * nOri.m_y * tmpVal1 - nOri.m_z * sinRotationAngle;
	rotaM(2,0) = nOri.m_x * nOri.m_z * tmpVal1 + nOri.m_y * sinRotationAngle;

	rotaM(0,1) = nOri.m_x * nOri.m_y * tmpVal1 + nOri.m_z * sinRotationAngle;
	rotaM(1,1) = nOri.m_y * nOri.m_y * tmpVal1 + cosRotationAngle;
	rotaM(2,1) = nOri.m_y * nOri.m_z * tmpVal1 - nOri.m_x * sinRotationAngle;

	rotaM(0,2) = nOri.m_x * nOri.m_z * tmpVal1 - nOri.m_y * sinRotationAngle;
	rotaM(1,2) = nOri.m_y * nOri.m_z * tmpVal1 + nOri.m_x * sinRotationAngle;
	rotaM(2,2) = nOri.m_z * nOri.m_z * tmpVal1 + cosRotationAngle;

	return rotaM;

}

IBK::matrix<double> Polygon::rotationMatrixForStraightOrigin(const double & sinRotationAngle, const IBKMK::Vector3D & nStraightOrigin)
{
	double cosRotationAngle = IBK::f_sqrt(1- std::pow(sinRotationAngle,2));

	return rotationMatrixForStraightOrigin(cosRotationAngle, nStraightOrigin, true);
}

IBKMK::Vector3D Polygon::rotaMatrixMultiplyVector(const IBK::matrix<double> & rotaM, size_t idx)
{
	if(idx >= m_polyline.size())
		throw IBK::Exception(IBK::FormatString("Index is outside of polyline size."), "Polygon::rotaMatrixMultiplyVector");
	IBKMK::Vector3D &point = m_polyline[idx];
	IBKMK::Vector3D newPoint;
	newPoint.m_x = rotaM(0,0) * point.m_x + rotaM(1,0) * point.m_y + rotaM(2,0) * point.m_z;
	newPoint.m_y = rotaM(0,1) * point.m_x + rotaM(1,1) * point.m_y + rotaM(2,1) * point.m_z;
	newPoint.m_z = rotaM(0,2) * point.m_x + rotaM(1,2) * point.m_y + rotaM(2,2) * point.m_z;

	return newPoint;
}

void Polygon::rotateAndShiftPolyline(const IBK::matrix<double> & rotaM, const IBKMK::Vector3D & translation)
{
	rotatePolyline(rotaM);
	if(translation!= IBKMK::Vector3D(0,0,0))
		for (auto &point : m_polyline)
			point +=translation;
}

void Polygon::rotatePolyline(const IBK::matrix<double> & rotaM)
{
	for (size_t i=0; i<m_polyline.size(); ++i)
		m_polyline[i] = rotaMatrixMultiplyVector(rotaM, i);
}

void Polygon::rotatePolyline(const IBKMK::Vector3D & rotaAxis, double cosRotaAngle, const IBKMK::Vector3D & translation){
	//if rotation axis is not in 0,0,0
	//shift the axis to point(0,0,0)
	//also shift polyline
	if(translation!= IBKMK::Vector3D(0,0,0))
		for (auto &point : m_polyline)
			point -=translation;
	IBK::matrix<double> m1=rotationMatrixForStraightOrigin(cosRotaAngle,rotaAxis, true);
	//rotate polyline
	rotatePolyline(m1);
	//shift back polyline
	if(translation!= IBKMK::Vector3D(0,0,0))
		for (auto &point : m_polyline)
			point +=translation;

}

void Polygon::changeNormalDirection()
{
	std::vector<IBKMK::Vector3D> newVec;
	while (!m_polyline.empty()) {
		newVec.push_back(m_polyline.back());
		m_polyline.pop_back();
	}
	m_polyline.swap(newVec);
}

void Polygon::removePointsInLine()
{
	FUNCID(Polygon::removePointsInLine);

	removeDuplicatePoints();

	bool pointsInLine = true;
	size_t i=0;
	while(pointsInLine){

		size_t polySize = m_polyline.size();
		size_t i1 = (i+1)%polySize;
		size_t i2 = (i+2)%polySize;

		IBKMK::Vector3D vA = IBKMK::Vector3D(m_polyline[i1]) - m_polyline[i];
		IBKMK::Vector3D vB = IBKMK::Vector3D(m_polyline[i2]) - m_polyline[i];
		vA.normalize();
		vB.normalize();

		if(vA == vB)
			m_polyline.erase(m_polyline.begin()+i1);
		else
			++i;
		if(i1==0)
			pointsInLine=false;
	}

	if(m_polyline.size()<3)
		throw IBK::Exception( IBK::FormatString("Polyline has less then 3 Points after removing points on a line."), FUNC_ID);
}

void Polygon::removeDuplicatePoints()
{
	FUNCID(Polygon::removeDuplicatePoints);

	bool duplicates = true;
	size_t i=0;
	while (duplicates) {
		size_t polySize = m_polyline.size();
		size_t i1 = (i+1)%polySize;
		if(m_polyline[i] == m_polyline[i1])	//harter verleich aufpassen
			m_polyline.erase(m_polyline.begin()+i1);
		else
			++i;
		if(i1==0)
			duplicates=false;
	}
	if(m_polyline.size()<3)
		throw IBK::Exception( IBK::FormatString("Polyline has less then 3 Points after removing duplicates."), FUNC_ID);
}




}
