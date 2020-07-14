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
	SerializationTest()
	{
		m_para[t_x1].set("X1", 12, "C");
		m_flags[t_x2].set("X2", true);
		m_interfaces.push_back(Interface());
		std::vector<double> x = {0, 1, 1.4, 2};
		std::vector<double> y = {1, 2, 3.4, 5};
		m_spline.setValues(x,y);
	}

	NANDRAD_READWRITE

	enum test_t {
		t_x1,									// Keyword: X1
		t_x2,									// Keyword: X2
		NUM_test
	};

	int					m_id1		= 5;					// XML:A
	unsigned int		m_id2		= 10;					// XML:A
	bool				m_flag1		= false;				// XML:A
	double				m_val1		= 42.42;				// XML:A
	test_t				m_testBla	= t_x1;					// XML:A
	std::string			m_str1		= "Blubb";				// XML:A
	IBK::Path			m_path1		= IBK::Path("/tmp");	// XML:A
	IBK::Unit			m_u1		= IBK::Unit("K");		// XML:A

	int					m_id3		= 10;					// XML:E
	unsigned int		m_id4		= 12;					// XML:E
	bool				m_flag2		= true;					// XML:E
	double				m_val2		= 41.41;				// XML:E
	test_t				m_testBlo	= NUM_test;				// XML:E
	std::string			m_str2		= "blabb";				// XML:E:not-empty
	IBK::Path			m_path2		= IBK::Path("/var");	// XML:E:not-empty
	IBK::Unit			m_u2		= IBK::Unit("C");		// XML:E

	Interface			m_iface;							// XML:E

	std::vector<Interface>	m_interfaces;					// XML:E

	IBK::Parameter		m_para[NUM_test];					// XML:E
	IBK::Flag			m_flags[NUM_test];					// XML:E

	IBK::LinearSpline	m_spline;							// XML:E

};

}

#endif // NANDRAD_SerializationTestH
