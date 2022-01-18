#ifndef EP_MaterialH
#define EP_MaterialH

#include <vector>
#include <string>

namespace EP {


/*!
	IDF Material Class
	IDF Material NoMass Class
	CHECK no changes between EP version 8.3 to 9.5.
*/
class Material
{
public:

	enum Roughness{
		R_VeryRough,
		R_Rough,
		R_MediumRough,
		R_MediumSmooth,
		R_Smooth,
		R_VerySmooth,
		NUM_R
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	Material(){}

	Material(std::string name,double thickness, double lambda, double rho, double c):
		m_name(name),
		m_conductivity(lambda),
		m_thickness(thickness),
		m_density(rho),
		m_specHeatCapa(c),
		m_thermalAbsorptance(0.9),
		m_solarAbsorptance(0.6),
		m_visibleAbsorptance(0.6),
		m_roughness(R_MediumRough)
	{}

	/*! Read "Material" and "Material:NoMass" section in IDF. */
	void read(const std::vector<std::string> & str, unsigned int version);

	/*! Write IDF Class. */
	void write(std::string &outStr, unsigned int version) const;

	bool behavesLike(const Material &other) const;

	/*! Compares this instance with another by value and returns true if they equal. */
	bool operator==(const Material & other) const;

	/*! Compares this instance with another by value and returns true if they differ. */
	bool operator!=(const Material & other)  const { return ! operator==(other); }

	// *** PUBLIC MEMBER VARIABLES ***
	std::string					m_name;
	double						m_conductivity;		// W/mK
	double						m_thickness;		// m
	double						m_density;			// kg/m3
	double						m_specHeatCapa;		// J/kgK
	double						m_thermalAbsorptance;	// unitless
	double						m_solarAbsorptance;		// unitless
	double						m_visibleAbsorptance;	// unitless
	Roughness					m_roughness;			// unitless
};
}

#endif // EP_MaterialH
