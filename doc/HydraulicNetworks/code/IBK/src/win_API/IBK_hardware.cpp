// IBK Library - Collection of classes, functions and algorithms
//               for development of numeric simulations
//
// Copyright (C) 2007 Andreas Nicolai, Andreas.Nicolai@gmx.net
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.

#if defined(_WIN32)
	#include <windows.h>
	#include <iphlpapi.h>
	#ifdef __BORLANDC__
	#else
		#include <intrin.h>
	#endif
#endif

#include <sstream>
#include <iomanip>

#include "IBK_hardware.h"

namespace IBK {

std::vector<std::string> getMacAddress() {
	std::vector<std::string> result;
#if defined(_WIN32)
	char data[4096];
    ZeroMemory( data, 4096 );
	unsigned long  len = 4000;
	PIP_ADAPTER_INFO pinfo = ( PIP_ADAPTER_INFO ) data;

    DWORD ret = GetAdaptersInfo( pinfo, &len );
    if( ret != ERROR_SUCCESS )
		return result;

	while ( pinfo ) {
		if( pinfo->Type == MIB_IF_TYPE_ETHERNET )  {// ignore software loopbacks and other strange things that might be present
			std::stringstream str;
			const int count = pinfo->AddressLength;
			for(int k = 0; k < count - 1; k++ ) {
				int val = static_cast<int>(pinfo->Address[k]);
				str << std::hex << std::setfill ('0') << std::setw (2) << val << "-";
			}
			int val = static_cast<int>(pinfo->Address[count - 1]);
			str << std::hex << std::setfill ('0') << std::setw (2) << val;
			result.push_back(str.str());
		}
		pinfo = pinfo->Next;
	}
#endif

	return result;
}

#ifdef __BORLANDC__
	#pragma pack(push, 1)
	struct sCpuId {
		unsigned int ulVersion;
		unsigned int ulOther;
		unsigned int ulExtendedFeatures;
		unsigned int ulFeatures;
	};
	#pragma pack(pop)

	#pragma option push -w-
	bool has_cpuid() {
		__asm {
			PUSHFD
			POP EAX
			MOV EDX, EAX
			XOR EAX, 0x00200000
			PUSH EAX
			POPFD
			PUSHFD
			POP EAX
			XOR EAX, EDX
			SHR EAX, 21
		}
	}

	void get_cpuid(sCpuId* CpuID) {
		__asm {
			PUSH EBX
			PUSH EDI
			MOV EDI, EAX
			MOV EAX,1
			DW 0xA20F
			STOSD
			MOV EAX, EBX
			STOSD
			MOV EAX, ECX
			STOSD
			MOV EAX, EDX
			STOSD
			POP EDI
			POP EBX
		}
	}
	#pragma option pop
#endif

void cpuID(unsigned i, unsigned regs[4]) {
#ifdef _WIN32
	#ifdef __BORLANDC__
		sCpuId cpu_id = {0};
		if( has_cpuid() )
			get_cpuid(&cpu_id);
		regs[0] = cpu_id.ulVersion;
		regs[1] = cpu_id.ulOther;
		regs[2] = cpu_id.ulExtendedFeatures;
		regs[3] = cpu_id.ulFeatures;
	#else
		__cpuid((int *)regs, (int)i);
	#endif
#else
	asm volatile
	("cpuid" : "=a" (regs[0]), "=b" (regs[1]), "=c" (regs[2]), "=d" (regs[3])
		: "a" (i), "c" (0));
	// ECX is set to zero for CPUID function 4
#endif
}

std::string cpuVendor() {
	unsigned regs[4];

	// Get vendor
	char vendor[12];
	cpuID(0, regs);
	((unsigned *)vendor)[0] = regs[1]; // EBX
	((unsigned *)vendor)[1] = regs[3]; // EDX
	((unsigned *)vendor)[2] = regs[2]; // ECX
	return std::string(vendor, 12);
}

} // namespace IBK
