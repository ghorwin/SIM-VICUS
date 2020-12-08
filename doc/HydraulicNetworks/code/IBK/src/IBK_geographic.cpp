#include "IBK_geographic.h"

#include <cmath>

#define PI 3.141592653589793238

namespace IBK {

namespace CONST {
	static const double DEG2RAD = PI / 180.0;
	static const double inverseFlattening = 1.0 / 298.257222101;
	static const double radiusEarth = 6378137;
	static const double phi1 = 35 * DEG2RAD;
	static const double phi2 = 65 * DEG2RAD;
	static const double falseEast = 4000000;
	static const double falseNorth = 2800000;
	static const double falseLatitude = 52 * DEG2RAD;
	static const double falseLongitude = 10 * DEG2RAD;
};

static double calcM(const double & phi, const double & ex) {
	const double var = ex * sin(phi);
	return cos(phi) / std::sqrt((1 - var * var));
}

static double calcT(const double & phi, const double & ex) {
	const double var = ex * sin(phi);
	return tan(PI / 4.0 - phi * 0.5) / std::pow((1 - var)/(1 + var), ex*0.5);
}

static double calcRadius(const double & radius,const double & phi, const double & ex,
								  const double & F,const double & n) {
	return radius * F * std::pow(calcT(phi, ex), n);
}

void transformWSG84ToLambertProjection(double longitude, double latitude, double& lambertEast, double& lambertNorth)  {
	//double R2 = m_radiusEarth * (1 - m_inverseFlattening);	//second earth radius
	double ex = std::sqrt(2 * CONST::inverseFlattening - std::pow(CONST::inverseFlattening, 2));	//eccentricity

	double n = (std::log(calcM(CONST::phi1, ex)) - std::log(calcM(CONST::phi2, ex)))
				/ (std::log(calcT(CONST::phi1, ex)) - std::log(calcT(CONST::phi2, ex)));
	double F = calcM(CONST::phi1, ex) / (n * std::pow(calcT(CONST::phi1, ex), n));

	double r = calcRadius(CONST::radiusEarth, latitude * CONST::DEG2RAD, ex, F, n);
	double rF = calcRadius(CONST::radiusEarth, CONST::falseLatitude, ex, F, n);
	double theta = n * (longitude * CONST::DEG2RAD - CONST::falseLongitude);

	double easting = CONST::falseEast + r * sin(theta);
	double northing = CONST::falseNorth + rF - r * cos(theta);

	lambertEast = easting;
	lambertNorth = northing;
}

void transformLambertProjectionToWSG84(double lambertEast, double lambertNorth, double& longitude, double& latitude) {
	double ex = std::sqrt(2 * CONST::inverseFlattening - std::pow(CONST::inverseFlattening, 2));	//eccentricity

	double latitudeDeg = 0;
	double longitudeDeg = 0;


	double n = (std::log(calcM(CONST::phi1, ex)) - std::log(calcM(CONST::phi2, ex)))
			/ (std::log(calcT(CONST::phi1, ex)) - std::log(calcT(CONST::phi2, ex)));
	double F = calcM(CONST::phi1, ex) / (n * std::pow(calcT(CONST::phi1, ex), n));

	double rF = calcRadius(CONST::radiusEarth, CONST::falseLatitude, ex, F, n);

	double diffEast = lambertEast - CONST::falseEast;
	double diffNorth = rF - (lambertNorth - CONST::falseNorth);
	double rS =sqrt(std::pow(diffEast,2) + std::pow(diffNorth,2));

	if( std::signbit(n))
		rS *= -1;

	double tS = std::pow(rS / (CONST::radiusEarth * F),1/n);
	double thetaS = atan2(diffEast, diffNorth);
	longitudeDeg = thetaS / n + CONST::falseLongitude;

	double oldLati = 800;
	unsigned int i=0;
	const double PI_HALF = PI / 2.0;
	do {
		oldLati = latitudeDeg;
		if(i==0) {
			latitudeDeg = PI_HALF - 2 * atan(tS);
		}
		else {
			double var = ex * sin(latitudeDeg);
			latitudeDeg = PI_HALF - 2 * atan(tS * std::pow((1 - var) / (1 + var), 0.5 * ex));
		}
		++i;

		//cancel loop if no convergence
		if(i>100)
			break;
	} while (std::fabs(oldLati - latitudeDeg) > 0.00001);


	latitude = latitudeDeg / CONST::DEG2RAD;
	longitude = longitudeDeg / CONST::DEG2RAD;
}

} // end namespace
