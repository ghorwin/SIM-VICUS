#ifndef NANDRAD_SerializationTestH
#define NANDRAD_SerializationTestH

#include <string>

#include <IBK_Flag.h>
#include <IBK_Path.h>
#include <IBK_LinearSpline.h>
#include <IBK_Unit.h>

#include <NANDRAD_Interface.h>
#include <NANDRAD_CodeGenMacros.h>

namespace NANDRAD {

class SerializationTest {
public:

	NANDRAD_READWRITE

	enum test_t {
		t_x1,
		t_x2,
		NUM_test
	};

	int					m_id1;				// XML:A
	unsigned int		m_id2;				// XML:A
	bool				m_flag1;			// XML:A
	double				m_val1;				// XML:A
	test_t				m_testBla;			// XML:A
	std::string			m_str1;				// XML:A
	IBK::Path			m_path1;			// XML:A
	IBK::Unit			m_u1;				// XML:A

	int					m_id3;				// XML:E
	unsigned int		m_id4;				// XML:E
	bool				m_flag2;			// XML:E
	double				m_val2;				// XML:E
	test_t				m_testBlo;			// XML:E
	std::string			m_str2;				// XML:E:not-empty
	IBK::Path			m_path2;			// XML:E:not-empty
	IBK::Unit			m_u2;				// XML:E

	Interface			m_iface;			// XMLS:E

	std::vector<Interface>	m_interfaces;	// XML:E

	IBK::Parameter		m_para[NUM_test];	// XML:E
	IBK::Flag			m_flags[NUM_test];	// XML:E

	IBK::LinearSpline	m_spline;			// XMLS:E


};

}

#endif // NANDRAD_SerializationTestH
