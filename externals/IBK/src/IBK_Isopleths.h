#ifndef EM_IsoplethsH
#define EM_IsoplethsH


#include <string>

#include "IBK_LinearSpline.h"
#include "IBK_UnitVector.h"

namespace IBK {

class Isopleths {
public:
	/*! Dividing surface types for mould grow.*/
	enum SurfaceKind {
		SK_Ideal,              ///<  Ideal substrate (nutrient fluid)
		SK_HeavyContaminated,  ///< heavy contaminated surface of each kind
		SK_CleanPorous,        ///< clean surface of a porous material
		SK_CleanInert,         ///< clean surface of an inert material
		SK_Num
	};

	/*! Isopleths for different exposure times and absolut limit.*/
	enum IsoplethKind {
		IK_Germ_1Day,  		   ///< Germination after one day exposure
		IK_Germ_2Days,         ///< Germination after two days exposure
		IK_Germ_4Days,         ///< Germination after 4 days exposure
		IK_Germ_8Days,         ///< Germination after 8 days exposure
		IK_Germ_16Days,        ///< Germination after 16 days exposure
		IK_Germ_LIM,           ///< Limit for germination
		IK_Num
	};

	using IsoFractionResult = std::array<double, Isopleths::IK_Num>;

	/*! Standard constructor. Creates internal array for isopleths and descriptions.*/
	Isopleths();

	/*! Return isopleth of given type. It return an empty spline in case such isopleth doesn't exist.*/
	IBK::LinearSpline isopleth(SurfaceKind surf, IsoplethKind isoKind);

	/*! Return description for isopleth of given type. It return an empty string in case such isopleth doesn't exist.*/
	std::string description(SurfaceKind surf, IsoplethKind isoKind) const;

	/*! Returns true if the point given by temperature T in °C and relative humidity phi in % is above isopleth of given kind.*/
	bool aboveLine(double T, double phi, SurfaceKind surf, IsoplethKind isoKind) const;

	/*! Return the time where the values are above the given isopleth.*/
	double aboveLineTime(double TCurr, double TLast, double phiCurr, double phiLast, double timeCurr, double timeLast,
									SurfaceKind surf, IsoplethKind isoKind) const;

	/*! Return the fraction for calculating germination probability according WTA 6.3 5.3.
		\param TCurr Temperature of current time step in °C
		\param TLast Temperature of last time step in °C
		\param phiCurr Relative humidity of current time step in %
		\param phiLast Relative humidity of last time step in %
		\param timeCurr Time point of current time step in h
		\param timeLast Time point of last time step in h
		\param isoKind Kind of the isopleth
	*/
	double germinationFraction(double TCurr, double TLast, double phiCurr, double phiLast, double timeCurr, double timeLast,
									IsoplethKind isoKind) const;

	IsoFractionResult fractionSumForAllIsopleths(const IBK::UnitVector& time, const IBK::UnitVector& temperature, const IBK::UnitVector& rh);

private:

	std::vector<std::vector<IBK::LinearSpline> >	m_isopleths;		///< Values for isopleths.
	std::vector<std::vector<std::string> >			m_descriptions;		///< Descriptions for isopleths.
};

} // namespace IBK

#endif // IBK_IsoplethsH
