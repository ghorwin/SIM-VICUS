#include "IBK_Isopleths.h"

#include "IBK_assert.h"

#include <array>

namespace IBK {

//const int Isopleths::T_Dirty_Germ_Number = 6;

const std::array<std::string,Isopleths::IK_Num>	T_Dirty_Germ_Kinds = {{ "1 day", "2 days", "4 days", "8 days", "16 days", "LIM" }};
const std::array<int,Isopleths::IK_Num>			T_Dirty_Germ_DaysAbove = {{ 1, 2, 4, 8, 16, 0 }};

const std::string	T_Dirty_Germ_1D_Description = "Germination within 1 day with heavily polluted surface";
const double		T_Dirty_Germ_1D_Min = 6.0;
const double		T_Dirty_Germ_1D_Step = 0.25;
const int			T_Dirty_Germ_1D_Number = 97;

const std::array<double,T_Dirty_Germ_1D_Number>  Phi_Dirty_Germ_1D = {{
								99.998,	99.64,	99.294,	98.96,	98.637,	98.326,	98.025,	97.735,	97.455,	97.185,
								96.924,	96.672,	96.429,	96.194,	95.967,	95.748,	95.537,	95.333,	95.136,	94.946,
								94.763,	94.586,	94.415,	94.25,	94.09,	93.936,	93.788,	93.644,	93.506,	93.373,
								93.244,	93.119,	92.999,	92.883,	92.771,	92.663,	92.558,	92.458,	92.36,	92.266,
								92.176,	92.088,	92.004,	91.922,	91.843,	91.767,	91.694,	91.623,	91.555,	91.489,
								91.425,	91.364,	91.304,	91.247,	91.192,	91.138,	91.087,	91.037,	90.989,	90.942,
								90.897,	90.854,	90.812,	90.772,	90.733,	90.696,	90.659,	90.624,	90.591,	90.558,
								90.527,	90.496,	90.467,	90.439,	90.411,	90.385,	90.359,	90.335,	90.311,	90.288,
								90.266,	90.245,	90.224,	90.204,	90.185,	90.166,	90.148,	90.131,	90.114,	90.098,
								90.083,	90.068,	90.053,	90.039,	90.026,	90.013,	90  }};

const std::string	T_Dirty_Germ_2D_Description = "Germination within 2 days with heavily polluted surface";
const double		T_Dirty_Germ_2D_Min = 3.25;
const double		T_Dirty_Germ_2D_Step = 0.25;
const int			T_Dirty_Germ_2D_Number = 108;

const std::array<double,T_Dirty_Germ_2D_Number>  Phi_Dirty_Germ_2D = {{
								  99.898,	99.363,	98.847,	98.349,	97.869,	97.405,	96.958,	96.527,	96.11,	95.708,
								  95.321,	94.947,	94.586,	94.237,	93.901,	93.577,	93.264,	92.962,	92.671,	92.39,
								  92.119,	91.857,	91.605,	91.361,	91.126,	90.9,	90.681,	90.47,	90.266,	90.069,
								  89.88,	89.697,	89.52,	89.35,	89.185,	89.027,	88.874,	88.726,	88.584,	88.446,
								  88.313,	88.185,	88.062,	87.943,	87.828,	87.717,	87.61,	87.507,	87.407,	87.311,
								  87.218,	87.129,	87.042,	86.959,	86.878,	86.801,	86.726,	86.654,	86.584,	86.517,
								  86.452,	86.389,	86.329,	86.271,	86.214,	86.16,	86.108,	86.057,	86.008,	85.961,
								  85.916,	85.872,	85.83,	85.789,	85.75,	85.712,	85.675,	85.64,	85.606,	85.573,
								  85.541,	85.511,	85.481,	85.452,	85.425,	85.398,	85.373,	85.348,	85.324,	85.301,
								  85.279,	85.258,	85.237,	85.217,	85.198,	85.179,	85.161,	85.144,	85.127,	85.111,
								  85.096,	85.081,	85.066,	85.052,	85.039,	85.026,	85.013,	85.001 }};

const std::string	T_Dirty_Germ_4D_Description = "Germination within 4 days with heavily polluted surface";
const double		T_Dirty_Germ_4D_Min = 1.75;
const double		T_Dirty_Germ_4D_Step = 0.25;
const int			T_Dirty_Germ_4D_Number = 114;

const std::array<double,T_Dirty_Germ_4D_Number>  Phi_Dirty_Germ_4D = {{
								  99.979,	99.501,	98.891,	98.301,	97.732,	97.183,	96.652,	96.139,	95.644,	95.165,
								  94.703,	94.257,	93.826,	93.41,	93.008,	92.619,	92.244,	91.882,	91.532,	91.194,
								  90.868,	90.552,	90.248,	89.954,	89.67,	89.395,	89.13,	88.875,	88.627,	88.389,
								  88.158,	87.935,	87.72,	87.512,	87.312,	87.118,	86.931,	86.75,	86.575,	86.407,
								  86.244,	86.086,	85.934,	85.788,	85.646,	85.509,	85.377,	85.249,	85.126,	85.007,
								  84.891,	84.78,	84.673,	84.569,	84.469,	84.372,	84.279,	84.189,	84.102,	84.017,
								  83.936,	83.858,	83.782,	83.709,	83.638,	83.569,	83.503,	83.44,	83.378,	83.319,
								  83.261,	83.206,	83.152,	83.1,	83.05,	83.002,	82.956,	82.911,	82.867,	82.825,
								  82.784,	82.745,	82.707,	82.671,	82.636,	82.601,	82.568,	82.537,	82.506,	82.476,
								  82.448,	82.42,	82.393,	82.367,	82.342,	82.318,	82.295,	82.273,	82.251,	82.23,
								  82.21,	82.19,	82.171,	82.153,	82.135,	82.118,	82.102,	82.086,	82.071,	82.056,
								  82.042,	82.028,	82.014,	82.002 }};

const std::string	T_Dirty_Germ_8D_Description = "Germination within 8 days with heavily polluted surface";
const double		T_Dirty_Germ_8D_Min = 0.5;
const double		T_Dirty_Germ_8D_Step = 0.25;
const int			T_Dirty_Germ_8D_Number = 119;

const std::array<double,T_Dirty_Germ_8D_Number>  Phi_Dirty_Germ_8D = {{
								  99.911,	99.372,	98.718,	98.084,	97.471,	96.878,	96.304,	95.748,	95.211,	94.69,
								  94.187,	93.699,	93.228,	92.771,	92.329,	91.902,	91.488,	91.088,	90.7,	90.325,
								  89.962,	89.611,	89.271,	88.942,	88.624,	88.315,	88.017,	87.729,	87.449,	87.179,
								  86.917,	86.664,	86.419,	86.182,	85.953,	85.731,	85.516,	85.308,	85.106,	84.912,
								  84.723,	84.541,	84.364,	84.193,	84.028,	83.868,	83.713,	83.563,	83.418,	83.277,
								  83.141,	83.01,	82.883,	82.76,	82.64,	82.525,	82.413,	82.305,	82.201,	82.1,
								  82.002,	81.907,	81.815,	81.726,	81.64,	81.557,	81.477,	81.399,	81.324,	81.251,
								  81.18,	81.112,	81.046,	80.982,	80.92,	80.86 ,	80.802,	80.746,	80.691,	80.639,
								  80.588,	80.539,	80.491,	80.445,	80.4,	80.357,	80.315,	80.275,	80.236,	80.198,
								  80.161,	80.126,	80.091,	80.058,	80.026,	79.995,	79.965,	79.936,	79.907,	79.88,
								  79.854,	79.828,	79.803,	79.779,	79.756,	79.734,	79.712,	79.691,	79.671,	79.651,
								  79.632,	79.614,	79.596,	79.579,	79.562,	79.546,	79.53,	79.515,	79.5 }};

const std::string	T_Dirty_Germ_16D_Description = "Germination within 16 days with heavily polluted surface";
const double		T_Dirty_Germ_16D_Min = 0.0;
const double		T_Dirty_Germ_16D_Step = 0.25;
const int			T_Dirty_Germ_16D_Number = 121;

const std::array<double,T_Dirty_Germ_16D_Number>  Phi_Dirty_Germ_16D = {{
								  99.2,		98.445,	97.715,	97.01,	96.328,	95.669,	95.032,	94.416,	93.821,	93.246,
								  92.69,	92.152,	91.633,	91.131,	90.645,	90.176,	89.723,	89.285,	88.861,	88.452,
								  88.056,	87.673,	87.304,	86.946,	86.601,	86.267,	85.944,	85.632,	85.331,	85.039,
								  84.758,	84.485,	84.222,	83.968,	83.722,	83.484,	83.255,	83.033,	82.818,	82.611,
								  82.41,	82.216,	82.029,	81.848,	81.673,	81.504,	81.34,	81.182,	81.03,	80.882,
								  80.739,	80.601,	80.468,	80.339,	80.215,	80.094,	79.978,	79.865,	79.757,	79.652,
								  79.55,	79.452,	79.357,	79.265,	79.177,	79.091,	79.008,	78.928,	78.851,	78.776,
								  78.703,	78.634,	78.566,	78.501,	78.438,	78.377,	78.318,	78.261,	78.206,	78.152,
								  78.101,	78.051,	78.003,	77.957,	77.912,	77.868,	77.826,	77.786,	77.747,	77.709,
								  77.672,	77.637,	77.602,	77.569,	77.537,	77.506,	77.477,	77.448,	77.42,	77.393,
								  77.367,	77.342,	77.317,	77.294,	77.271,	77.249,	77.228,	77.207,	77.187,	77.168,
								  77.15,	77.132,	77.114,	77.098,	77.081,	77.066,	77.051,	77.036,	77.022,	77.008,
								  76.995 }};

const std::string	T_Dirty_Germ_LIM_Description = "Limit for germination with heavily polluted surface";
const double		T_Dirty_Germ_LIM_Min = 0.0;
const double		T_Dirty_Germ_LIM_Step = 0.25;
const int			T_Dirty_Germ_LIM_Number = 121;

const std::array<double,T_Dirty_Germ_LIM_Number>  Phi_Dirty_Germ_LIM = {{
								  96.85,	96.092,	95.359,	94.65,	93.965,	93.304,	92.664,	92.045,	91.448,	90.87,
								  90.312,	89.772,	89.25,	88.746,	88.258,	87.787,	87.332,	86.892,	86.466,	86.055,
								  85.658,	85.273,	84.902,	84.543,	84.196,	83.861,	83.537,	83.224,	82.921,	82.628,
								  82.345,	82.072,	81.807,	81.552,	81.305,	81.066,	80.836,	80.613,	80.397,	80.189,
								  79.987,	79.793,	79.605,	79.423,	79.247,	79.077,	78.913,	78.754,	78.601,	78.453,
								  78.309,	78.171,	78.037,	77.907,	77.782,	77.661,	77.544,	77.431,	77.322,	77.217,
								  77.115,	77.016,	76.921,	76.829,	76.74,	76.654,	76.57,	76.49,	76.412,	76.337,
								  76.265,	76.194,	76.126,	76.061,	75.998,	75.936,	75.877,	75.82,	75.765,	75.711,
								  75.659,	75.609,	75.561,	75.514,	75.469,	75.426,	75.384,	75.343,	75.303,	75.265,
								  75.229,	75.193,	75.159,	75.126,	75.093,	75.062,	75.032,	75.003,	74.975,	74.948,
								  74.922,	74.897,	74.872,	74.849,	74.826,	74.804,	74.782,	74.762,	74.742,	74.722,
								  74.704,	74.686,	74.668,	74.652,	74.635,	74.62,	74.604,	74.59,	74.576,	74.562,
								  74.549 }};

Isopleths::Isopleths() :
	m_isopleths(std::vector<std::vector<IBK::LinearSpline> >(SK_Num, std::vector<IBK::LinearSpline>(IK_Num))),
	m_descriptions(std::vector<std::vector<std::string> >(SK_Num, std::vector<std::string>(IK_Num)))
{

	std::vector<double> Tvect;
	std::vector<double> Phivect;

	double T0 = T_Dirty_Germ_1D_Min;
	for(int i=0; i<T_Dirty_Germ_1D_Number; ++i) {
		double T = T0 + i * T_Dirty_Germ_1D_Step;
		Tvect.push_back(T);
		Phivect.push_back(Phi_Dirty_Germ_1D[i]);
	}
	m_isopleths[SK_HeavyContaminated][IK_Germ_1Day].setValues(Tvect, Phivect);
	m_descriptions[SK_HeavyContaminated][IK_Germ_1Day] = T_Dirty_Germ_1D_Description;
	Tvect.clear();
	Phivect.clear();

	T0 = T_Dirty_Germ_2D_Min;
	for(int i=0; i<T_Dirty_Germ_2D_Number; ++i) {
		double T = T0 + i * T_Dirty_Germ_2D_Step;
		Tvect.push_back(T);
		Phivect.push_back(Phi_Dirty_Germ_2D[i]);
	}
	m_isopleths[SK_HeavyContaminated][IK_Germ_2Days].setValues(Tvect, Phivect);
	m_descriptions[SK_HeavyContaminated][IK_Germ_2Days] = T_Dirty_Germ_2D_Description;
	Tvect.clear();
	Phivect.clear();

	T0 = T_Dirty_Germ_4D_Min;
	for(int i=0; i<T_Dirty_Germ_4D_Number; ++i) {
		double T = T0 + i * T_Dirty_Germ_4D_Step;
		Tvect.push_back(T);
		Phivect.push_back(Phi_Dirty_Germ_4D[i]);
	}
	m_isopleths[SK_HeavyContaminated][IK_Germ_4Days].setValues(Tvect, Phivect);
	m_descriptions[SK_HeavyContaminated][IK_Germ_4Days] = T_Dirty_Germ_4D_Description;
	Tvect.clear();
	Phivect.clear();

	T0 = T_Dirty_Germ_8D_Min;
	for(int i=0; i<T_Dirty_Germ_8D_Number; ++i) {
		double T = T0 + i * T_Dirty_Germ_8D_Step;
		Tvect.push_back(T);
		Phivect.push_back(Phi_Dirty_Germ_8D[i]);
	}
	m_isopleths[SK_HeavyContaminated][IK_Germ_8Days].setValues(Tvect, Phivect);
	m_descriptions[SK_HeavyContaminated][IK_Germ_8Days] = T_Dirty_Germ_8D_Description;
	Tvect.clear();
	Phivect.clear();

	T0 = T_Dirty_Germ_16D_Min;
	for(int i=0; i<T_Dirty_Germ_16D_Number; ++i) {
		double T = T0 + i * T_Dirty_Germ_16D_Step;
		Tvect.push_back(T);
		Phivect.push_back(Phi_Dirty_Germ_16D[i]);
	}
	m_isopleths[SK_HeavyContaminated][IK_Germ_16Days].setValues(Tvect, Phivect);
	m_descriptions[SK_HeavyContaminated][IK_Germ_16Days] = T_Dirty_Germ_16D_Description;
	Tvect.clear();
	Phivect.clear();

	T0 = T_Dirty_Germ_LIM_Min;
	for(int i=0; i<T_Dirty_Germ_LIM_Number; ++i) {
		double T = T0 + i * T_Dirty_Germ_LIM_Step;
		Tvect.push_back(T);
		Phivect.push_back(Phi_Dirty_Germ_LIM[i]);
	}
	m_isopleths[SK_HeavyContaminated][IK_Germ_LIM].setValues(Tvect, Phivect);
	m_descriptions[SK_HeavyContaminated][IK_Germ_LIM] = T_Dirty_Germ_LIM_Description;
}

IBK::LinearSpline Isopleths::isopleth(SurfaceKind surf, IsoplethKind isoKind) {
	IBK::LinearSpline result;
	if(surf != SK_HeavyContaminated)
		return result;

	IBK_ASSERT(isoKind < IK_Num);

	return m_isopleths[SK_HeavyContaminated][isoKind];
}

std::string Isopleths::description(SurfaceKind surf, IsoplethKind isoKind) const {
	std::string result;
	if(surf != SK_HeavyContaminated)
		return result;

	IBK_ASSERT(isoKind < IK_Num);

	return m_descriptions[SK_HeavyContaminated][isoKind];
}

bool Isopleths::aboveLine(double T, double phi, SurfaceKind surf, IsoplethKind isoKind) const {
	const IBK::LinearSpline& iso = m_isopleths[surf][isoKind];
	if(iso.empty())
		return false;

	double isoPhi = iso.value(T);
	return phi > isoPhi;
}

double Isopleths::aboveLineTime(double TCurr, double TLast, double phiCurr, double phiLast, double timeCurr, double timeLast,
								SurfaceKind surf, IsoplethKind isoKind) const {
	const IBK::LinearSpline& iso = m_isopleths[surf][isoKind];
	if(iso.empty())
		return 0;

	double isoPhiCurr = iso.value(TCurr);
	double isoPhiLast = iso.value(TLast);
	if(phiCurr >= isoPhiCurr && phiLast >= isoPhiLast)
		return timeCurr - timeLast;
	if(phiCurr < isoPhiCurr && phiLast < isoPhiLast)
		return 0;

	double mPhi = (phiCurr - phiLast) / (timeCurr - timeLast);
	double nPhi = (phiLast * timeCurr - phiCurr * timeLast) / (timeCurr - timeLast);
	double mPhiIso = (isoPhiCurr - isoPhiLast) / (timeCurr - timeLast);
	double nPhiIso = (isoPhiLast * timeCurr - isoPhiCurr * timeLast) / (timeCurr - timeLast);
	double timePoint = (nPhiIso - nPhi) / (mPhi - mPhiIso);
	IBK_ASSERT(timePoint >= timeLast);
	IBK_ASSERT(timePoint <= timeCurr);
	if(phiCurr >= isoPhiCurr) {
		return timeCurr - timePoint;
	}
	else {
		return timePoint - timeLast;
	}
}

double Isopleths::germinationFraction(double TCurr, double TLast, double phiCurr, double phiLast, double timeCurr, double timeLast,
								IsoplethKind isoKind) const {
	double deltaTime = aboveLineTime(TCurr, TLast, phiCurr, phiLast, timeCurr, timeLast, SK_HeavyContaminated, isoKind);
	int timeLimit = T_Dirty_Germ_DaysAbove[isoKind];
	if(timeLimit == 0) {
		return deltaTime > 0 ? 1.0 : 0.0;
	}

	double fraction = deltaTime / (timeLimit * 24.0);
	return fraction;
}

Isopleths::IsoFractionResult Isopleths::fractionSumForAllIsopleths(const IBK::UnitVector& time, const IBK::UnitVector& temperature, const IBK::UnitVector& rh) {
	IsoFractionResult res{0};
	IBK_ASSERT(time.size() == temperature.size());
	IBK_ASSERT(time.size() == rh.size());
	IBK_ASSERT(time.m_unit == IBK::Unit("h"));
	IBK_ASSERT(temperature.m_unit == IBK::Unit("C"));
	IBK_ASSERT(rh.m_unit == IBK::Unit("%"));

	for(size_t i=1; i<time.size(); ++i) {
		for(size_t iso=IK_Germ_1Day; iso<IK_Num; ++iso) {
			res[iso] += germinationFraction(temperature[i], temperature[i-1], rh[i], rh[i-1], time[i], time[i-1], static_cast<IsoplethKind>(iso));
		}
	}
	return res;
}

} // namespace EM
