/*	The NANDRAD data model library.

	Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Anne Paepcke     <anne.paepcke -[at]- tu-dresden.de>

	This library is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This library is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
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
#include <IBK_point.h>

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
		m_intPara[IP_i1].set("I1", 13);
		m_intPara[IP_i2].set("I2", 15);
		m_f.set("F",true);
		m_dblVec = std::vector<double>{0, 12, 24};

		m_sched.m_type = Schedule::ST_FRIDAY;
		m_sched.m_startDayOfTheYear = static_cast<unsigned int>(IBK::Time(2007,4,1).secondsOfYear()/(3600*24));
		m_sched.m_endDayOfTheYear = static_cast<unsigned int>(IBK::Time(2007,10,1).secondsOfYear()/(3600*24));
		DailyCycle d;
		d.m_interpolation = DailyCycle::IT_Constant;
		d.m_timePoints = std::vector<double>{0,6,18};
		d.m_values.m_values["Temperatures"] = std::vector<double>{0,6,18};
		m_sched.m_dailyCycles.push_back( DailyCycle() );

		m_sched2 = m_sched;

		m_time1.set(2007,46032);

		m_table.m_values["Col1"] = std::vector<double>{1,5,3};
		m_table.m_values["Col2"] = std::vector<double>{7,2,2};

		m_singlePara.set("SinglePara", 20, IBK::Unit("C") );

		// spline tests

		// unitless spline
		m_linSpl.setValues(x,y);

		// spline with name and units
		m_splineParameter.m_name = "SplineParameter"; // name must match the tag name auto-generated from variable name
		m_splineParameter.m_values.setValues({0, 5,10}, {5,4,3});
		m_splineParameter.m_xUnit.set("m");
		m_splineParameter.m_yUnit.set("C");

		m_anotherSplineParameter.m_name = "AnotherSplineParameter"; // name must match the tag name auto-generated from variable name
		m_anotherSplineParameter.m_values.setValues({0, 5,10}, {5,4,3});
		m_anotherSplineParameter.m_xUnit.set("m");
		m_anotherSplineParameter.m_yUnit.set("C");

		m_splinePara[SP_ParameterSet1] = m_splineParameter;
		m_splinePara[SP_ParameterSet1].m_name = "ParameterSet1"; // name must match the keyword list name

		for (unsigned int & a : m_idReferences) a = NANDRAD::INVALID_ID;
		m_idReferences[SomeFurnace] = 2;
		m_idReferences[SomeHeater] = 15;
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

	enum splinePara_t {
		SP_ParameterSet1,									// Keyword: ParameterSet1
		SP_ParameterSet2,									// Keyword: ParameterSet2
		NUM_SP
	};


	enum ReferencedIDTypes {
		SomeStove,											// Keyword: SomeStove
		SomeOven,											// Keyword: SomeOven
		SomeHeater,											// Keyword: SomeHeater
		SomeFurnace,										// Keyword: SomeFurnace
		NUM_RefID
	};

	// -> id1="5"
	int					m_id1		= 5;					// XML:A:required
	// -> id2="10"
	unsigned int		m_id2		= 10;					// XML:A
	// -> flag1="0"
	bool				m_flag1		= false;				// XML:A
	// -> val1="42.42"
	double				m_val1		= 42.42;				// XML:A
	// -> testBla="X1"
	test_t				m_testBla	= t_x1;					// XML:A
	// -> str1="Blubb"
	std::string			m_str1		= "Blubb";				// XML:A
	// -> path1="/tmp"
	IBK::Path			m_path1		= IBK::Path("/tmp");	// XML:A
	// -> u1="K"
	IBK::Unit			m_u1		= IBK::Unit("K");		// XML:A

	// -> <Id3>10</Id3>
	int					m_id3		= 10;					// XML:E:required
	// -> <Id4>12</Id4>
	unsigned int		m_id4		= 12;					// XML:E
	// -> <Flag2>1</Flag2>
	bool				m_flag2		= true;					// XML:E
	// -> <Val2>41.41</Val2>
	double				m_val2		= 41.41;				// XML:E
	// -> <TestBlo>X2</TestBlo>
	test_t				m_testBlo	= t_x2;					// XML:E
	// -> <Str2>blabb</Str2>
	std::string			m_str2		= "blabb";				// XML:E

	// -> <Path2>/var</Path2>
	IBK::Path			m_path2		= IBK::Path("/var");	// XML:E
	// -> undefined/empty - not written
	IBK::Path			m_path22;							// XML:E

	// -> <U2>C</U2>
	IBK::Unit			m_u2		= IBK::Unit("C");		// XML:E
	// -> <X5>43.43</X5>
	double				m_x5		= 43.43;				// XML:E

	// -> <IBK:Flag name="F">true</IBK:Flag>  -> value of m_f.name is ignored
	IBK::Flag			m_f;								// XML:E
	// -> undefined/empty - not written
	IBK::Flag			m_f2;								// XML:E

	// -> <Time1>01.01.07 12:47:12</Time1>
	IBK::Time			m_time1;							// XML:E
	// -> undefined/empty - not written
	IBK::Time			m_time2;							// XML:E

	// -> <Table>Col1:1,5,3;Col2:7,2,2;</Table>
	DataTable			m_table;							// XML:E
	// -> undefined/empty - not written
	DataTable			m_table2;							// XML:E

	// -> 		<DblVec>0,12,24</DblVec>
	std::vector<double>		m_dblVec;						// XML:E

	// -> <Interfaces>...</Interfaces>
	std::vector<Interface>	m_interfaces;					// XML:E

	// -> <InterfaceA>....</InterfaceA>  instead of <Interface>..</Interface>
	Interface				m_interfaceA;					// XML:E:tag=InterfaceA

	// -> <IBK:Parameter name="SinglePara" unit="C">20</IBK:Parameter>
	IBK::Parameter		m_singlePara;						// XML:E

	// -> <IBK:IntPara name="SingleIntegerPara">12</IBK:IntPara>
	IBK::IntPara		m_singleIntegerPara = IBK::IntPara("blubb",12);	// XML:E

	// -> <IBK:Parameter name="X1" unit="C">12</IBK:Parameter>
	IBK::Parameter		m_para[NUM_test];						// XML:E

	// -> <IBK:IntPara name="I1">13</IBK:IntPara>
	IBK::IntPara		m_intPara[NUM_IP];						// XML:E

	// -> <IBK:Flag name="X2">true</IBK:Flag>
	IBK::Flag			m_flags[NUM_test];						// XML:E

	IDType				m_someStuffIDAsAttrib;					// XML:A
	IDType				m_someStuffIDAsElement;					// XML:E

	// -> <SomeStove>231</SomeStove> : Keywords must be unique!
	IDType				m_idReferences[NUM_RefID];				// XML:E

	// -> <IBK:LinearSpline name="LinSpl">...</IBK:LinearSpline>
	IBK::LinearSpline	m_linSpl;								// XML:E

	// -> <LinearSplineParameter name="SplineParameter">...</LinearSplineParameter>
	NANDRAD::LinearSplineParameter	m_splineParameter;			// XML:E
	// -> <LinearSplineParameter name="AnotherSplineParameter">...</LinearSplineParameter>
	LinearSplineParameter			m_anotherSplineParameter;	// XML:E

	// -> <LinearSplineParameter name="ParameterSet1">...</LinearSplineParameter>
	NANDRAD::LinearSplineParameter m_splinePara[NUM_SP];		// XML:E

	// generic class with own readXML() and writeXML() function
	// -> <Schedule...>...</Schedule>
	Schedule			m_sched;								// XML:E

	// generic class with custom tag name
	// -> <OtherSchedule...>...</OtherSchedule>
	Schedule			m_sched2;								// XML:E:tag=OtherSchedule

	// -> <Point2D>x,y</Point2D>
	IBK::point2D<double>	m_coordinate2D;						// XML:E
};

}

#endif // NANDRAD_SerializationTestH
