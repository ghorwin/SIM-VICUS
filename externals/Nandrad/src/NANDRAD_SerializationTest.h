/*	The NANDRAD data model library.

	Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Anne Paepcke     <anne.paepcke -[at]- tu-dresden.de>

	This library is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 3 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.
*/

#ifndef NANDRAD_SerializationTestH
#define NANDRAD_SerializationTestH

#include <string>

#include <IBK_Flag.h>
#include <IBK_Path.h>
#include <IBK_LinearSpline.h>
#include <IBK_Unit.h>
#include <IBK_IntPara.h>
#include <IBK_Time.h>

#include <NANDRAD_Schedule.h>
#include <NANDRAD_Interface.h>
#include <NANDRAD_CodeGenMacros.h>
#include <NANDRAD_DataTable.h>

namespace NANDRAD {

/*! Test class to check correct functionality of the NANDRAD code generator. 
	\note This class is not used in the NANDRAD data model.
*/
class SerializationTest {
public:
	SerializationTest()
	{
		m_para[t_x1].set("X1", 12, "C");
		m_flags[t_x2].set("X2", true);
		Interface iface;
		iface.m_id = 1;
		iface.m_zoneId = 0;
		m_interfaces.push_back(iface);
		std::vector<double> x = {0, 1, 1.4, 2};
		std::vector<double> y = {1, 2, 3.4, 5};
		m_spline.setValues(x,y);
		m_intPara[IP_i1].set("I1", 13);
		m_intPara[IP_i2].set("I2", 15);
		m_f.set("F",true);
		m_dblVec = std::vector<double>{0, 12, 24};

		m_sched.m_type = Schedule::ST_FRIDAY;
		m_sched.m_startDate = IBK::Time(2007,4,1);
		m_sched.m_endDate = IBK::Time(2007,8,30);
		DailyCycle d;
		d.m_interpolation = DailyCycle::IT_CONSTANT;
		d.m_timeUnit.set("h");
		d.m_timePoints = std::vector<double>{0,6,18};
		d.m_values.m_values["Temperatures"] = std::vector<double>{0,6,18};
		m_sched.m_dailyCycles.push_back( DailyCycle() );

		m_time1.set(2007,46032);

		m_table.m_values["Col1"] = std::vector<double>{1,5,3};
		m_table.m_values["Col2"] = std::vector<double>{7,2,2};
	}

	NANDRAD_READWRITE

	enum test_t {
		t_x1,												// Keyword: X1
		t_x2,												// Keyword: X2
		NUM_test
	};

	enum intPara_t {
		IP_i1,												// Keyword: I1
		IP_i2,												// Keyword: I2
		NUM_IP
	};

	int					m_id1		= 5;					// XML:A:required
	unsigned int		m_id2		= 10;					// XML:A
	bool				m_flag1		= false;				// XML:A
	double				m_val1		= 42.42;				// XML:A
	test_t				m_testBla	= t_x1;					// XML:A
	std::string			m_str1		= "Blubb";				// XML:A
	IBK::Path			m_path1		= IBK::Path("/tmp");	// XML:A
	IBK::Unit			m_u1		= IBK::Unit("K");		// XML:A

	int					m_id3		= 10;					// XML:E:required
	unsigned int		m_id4		= 12;					// XML:E
	bool				m_flag2		= true;					// XML:E
	double				m_val2		= 41.41;				// XML:E
	test_t				m_testBlo	= t_x2;					// XML:E
	std::string			m_str2		= "blabb";				// XML:E
	IBK::Path			m_path2		= IBK::Path("/var");	// XML:E
	IBK::Unit			m_u2		= IBK::Unit("C");		// XML:E
	double				m_x5		= 43.43;				// XML:E
	IBK::Flag			m_f;								// XML:E
	IBK::Time			m_time1;							// XML:E
	IBK::Time			m_time2;							// XML:E

	DataTable			m_table;							// XML:E

	std::vector<double>		m_dblVec;						// XML:E
	std::vector<Interface>	m_interfaces;					// XML:E

	IBK::Parameter		m_para[NUM_test];					// XML:E
	IBK::IntPara		m_intPara[NUM_IP];					// XML:E
	IBK::Flag			m_flags[NUM_test];					// XML:E

	IBK::LinearSpline	m_spline;							// XML:E

	// example for a generic class with own readXML() and writeXML() function
	Schedule			m_sched;							// XML:E
};

}

#endif // NANDRAD_SerializationTestH
