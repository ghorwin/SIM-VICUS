#ifndef NANDRAD_SerializationTestH
#define NANDRAD_SerializationTestH

#include <string>
#include <NANDRAD_Interface.h>

#include <IBK_Flag.h>
#include <IBK_Path.h>

namespace NANDRAD {

class SerializationTest {
public:

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

	int					m_id3;				// XML:E
	unsigned int		m_id4;				// XML:E
	bool				m_flag2;			// XML:E
	double				m_val2;				// XML:E
	test_t				m_testBlo;			// XML:E
	std::string			m_str2;				// XML:E
	IBK::Path			m_path2;			// XML:A

	Interface			m_iface;			// XML:E

	std::vector<Interface>	m_interfaces;	// XML:E

	IBK::Parameter		m_para[NUM_test];	// XML:E
	IBK::Flag			m_flags[NUM_test];	// XML:E



};

}

#endif // NANDRAD_SerializationTestH
