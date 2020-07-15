#ifndef NANDRAD_MATERIALLAYER_H
#define NANDRAD_MATERIALLAYER_H


#include "NANDRAD_CodeGenMacros.h"

class TiXmlElement;

namespace NANDRAD {


class MaterialLayer
{
public:
	MaterialLayer(){}

	/*! Simple Constructor with thickness in m and id. */
	MaterialLayer(double thickness,unsigned int id):
		m_thickness(thickness),
		m_matId(id)
	{}

	NANDRAD_READWRITE

	/*! Thickness in m. */
	double					m_thickness;				// XML:E

	/*! Material id. */
	unsigned int			m_matId;					// XML:A
};
}

#endif // NANDRAD_MATERIALLAYER_H
