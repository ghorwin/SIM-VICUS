#ifndef VICUS_AcousticSoundAbsorptionPartitionH
#define VICUS_AcousticSoundAbsorptionPartitionH

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"
#include "VICUS_Database.h"
#include "VICUS_AcousticSoundAbsorption.h"

#include <IBK_Parameter.h>

namespace VICUS {

class AcousticSoundAbsorptionPartition {
public:

	/*! Basic parameters. */
	enum para_t {
		/*! Area fraction. */
		P_AreaFraction,			// Keyword: AreaFraction					[---]	'Fraction of surface area.'
		NUM_P
	};
	// *** PUBLIC MEMBER FUNCTIONS ***

	/*! Default c'tor. */
	AcousticSoundAbsorptionPartition(){}


	/*! Simple Constructor with thickness in [m] and material id. */
	AcousticSoundAbsorptionPartition(double areaFraction, unsigned int id):
		m_idSoundAbsorption(id)
	{
		m_para[P_AreaFraction].set("AreaFraction", areaFraction, IBK::Unit("---"));
	}

	/*! Simple Constructor with thickness and material id. */
	AcousticSoundAbsorptionPartition(IBK::Parameter areaFraction, unsigned int id):
		m_idSoundAbsorption(id)
	{
		m_para[P_AreaFraction] = areaFraction;
	}

	VICUS_READWRITE

	/*! Checks if all referenced materials exist and if their parameters are valid. */
	bool isValid(const VICUS::Database<VICUS::AcousticSoundAbsorption> & soundAbsorption) const;

	/*! Inequality operator. */
	bool operator!=(const AcousticSoundAbsorptionPartition & other) const {
		return (m_idSoundAbsorption != other.m_idSoundAbsorption||
				m_para[P_AreaFraction] != other.m_para[P_AreaFraction]);
	}
	/*! Equality operator. */
	bool operator==(const AcousticSoundAbsorptionPartition & other) const { return !operator!=(other); }

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique ID of material. */
	unsigned int					m_idSoundAbsorption = INVALID_ID;	// XML:A:required

	/*! IBK::Parameter. */
	IBK::Parameter					m_para[NUM_P];						// XML:E:required


	/*! Holds error string in order to give users a tooltip in db dialog. */
	mutable std::string				m_errorMsg;
};

} // namespace VICUS

#endif // VICUS_AcousticSoundAbsorptionPartitionH
