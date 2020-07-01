/*	Copyright (c) 2001-2017, Institut für Bauklimatik, TU Dresden, Germany

	Written by A. Nicolai, H. Fechner, St. Vogelsang, A. Paepcke, J. Grunewald
	All rights reserved.

	This file is part of the IBK Library.

	Redistribution and use in source and binary forms, with or without modification,
	are permitted provided that the following conditions are met:

	1. Redistributions of source code must retain the above copyright notice, this
	   list of conditions and the following disclaimer.

	2. Redistributions in binary form must reproduce the above copyright notice,
	   this list of conditions and the following disclaimer in the documentation
	   and/or other materials provided with the distribution.

	3. Neither the name of the copyright holder nor the names of its contributors
	   may be used to endorse or promote products derived from this software without
	   specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
	ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
	DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
	ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
	(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
	LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
	ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


	This library contains derivative work based on other open-source libraries.
	See OTHER_LICENCES and source code headers for details.

*/

#ifndef IBK_memory_usageH
#define IBK_memory_usageH


#if defined(MPI_VERSION)
#	include <mpi.h>
#endif

//#if defined(__MINGW32__)
//#define WINVER 0x0500
//#endif

#if (defined(_MSC_VER) || defined(__BORLANDC__) || defined(__MINGW32__))
#	include <windows.h>
#	include <psapi.h>
#endif

#if defined(__linux__)
#	include <cstdlib>
#	include <cstdio>
#	include <cstring>
#endif

#if defined(__APPLE__)
#	include <mach/vm_statistics.h>
#	include <mach/mach_types.h>
#	include <mach/mach_init.h>
#	include <mach/mach_host.h>
#	include<mach/mach.h>
#endif



namespace IBK {


	/*! This static function tries to return the current RAM usage of a utilizing process
	*	its compiled into in kibibytes. Returns 0 if value couldn't be enquired.
	*
	* Look for lines in the procfile contents like:
	* VmRSS:		 5560 kB
	* VmSize:		 5560 kB
	*
	* Grab the number between the whitespace and the "kB"
	* If 1 is returned in the end, there was a serious problem
	* (we could not find one of the memory usages)
	*
	* serial version.
	*/
	static int memoryUsageKb(unsigned long long* vmrss_kb, unsigned long long* vmsize_kb){

#if defined(__GNUC__)

#	if defined(__linux__)

		/* Get the the current process' status file from the proc filesystem */
		FILE* procfile = fopen("/proc/self/status", "r");

		long to_read = 8192;
		char buffer[to_read];
		/*int read = */fread(buffer, sizeof(char), to_read, procfile);
		fclose(procfile);

		short found_vmrss = 0;
		short found_vmsize = 0;
		char* search_result;

		/* Look through proc status contents line by line */
		char delims[] = "\n";
		char* line = strtok(buffer, delims);

		while (line != NULL && (found_vmrss == 0 || found_vmsize == 0) )
		{
			search_result = strstr(line, "VmRSS:");
			if (search_result != NULL)
			{
				sscanf(line, "%*s %llu", vmrss_kb);
				found_vmrss = 1;
			}

			search_result = strstr(line, "VmSize:");
			if (search_result != NULL)
			{
				sscanf(line, "%*s %llu", vmsize_kb);
				found_vmsize = 1;
			}

			line = strtok(NULL, delims);
		}

		return (found_vmrss == 1 && found_vmsize == 1) ? 0 : 1;


#	elif defined(__MINGW32__) || defined(__MINGW64__)

		DWORD processID = GetCurrentProcessId();
		HANDLE hprocess = OpenProcess(  PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, false, processID );
		if( hprocess == 0)
			return 1;
		PROCESS_MEMORY_COUNTERS pmc;
		if ( !GetProcessMemoryInfo( hprocess, &pmc, sizeof(pmc)) )
			return 1;

		(*vmsize_kb) = pmc.WorkingSetSize / 1024;
		(*vmrss_kb) = pmc.PeakWorkingSetSize / 1024;
		return 0;

#	elif defined(__APPLE__)

		// APPLE
		/// \todo find solution for apple, and test this code

		struct task_basic_info t_info;
		mach_msg_type_number_t t_info_count = TASK_BASIC_INFO_COUNT;

		if (KERN_SUCCESS != task_info(mach_task_self(),
									  TASK_BASIC_INFO, (task_info_t)&t_info,
									  &t_info_count))
		{
			return -1;
		}
		// resident size is
		*(vmrss_kb)=t_info.resident_size;
		// virtual size is in t_info.virtual_size;

		vm_size_t page_size;
		mach_port_t mach_port;
		mach_msg_type_number_t count;
		vm_statistics_data_t vm_stats;

		mach_port = mach_host_self();
		count = sizeof(vm_stats) / sizeof(natural_t);
		if (KERN_SUCCESS == host_page_size(mach_port, &page_size) &&
			KERN_SUCCESS == host_statistics(mach_port, HOST_VM_INFO,
											(host_info_t)&vm_stats, &count))
		{
			myFreeMemory = (int64_t)vm_stats.free_count * (int64_t)page_size;

			used_memory = ((int64_t)vm_stats.active_count +
						   (int64_t)vm_stats.inactive_count +
						   (int64_t)vm_stats.wire_count) *  (int64_t)page_size;
		}

		*(vmsize_kb) = used_memory;

		return 0;

#	endif

#else

#	if defined(_MSC_VER)

		DWORD processID = GetCurrentProcessId();
		HANDLE hprocess = OpenProcess(  PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, false, processID );
		if( hprocess == 0)
			return 1;
		PROCESS_MEMORY_COUNTERS pmc;
		if ( !GetProcessMemoryInfo( hprocess, &pmc, sizeof(pmc)) )
			return 1;

		(*vmsize_kb) = pmc.WorkingSetSize / 1024;
		(*vmrss_kb) = pmc.PeakWorkingSetSize / 1024;
		return 0;

#	elif defined(__BORLANDC__)
		DWORD processID = GetCurrentProcessId();
		HANDLE hprocess = OpenProcess(  PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, false, processID );
		if( hprocess == 0)
			return 1;
		PROCESS_MEMORY_COUNTERS pmc;
		if ( !GetProcessMemoryInfo( hprocess, &pmc, sizeof(pmc)) )
			return 1;

		(*vmsize_kb) = pmc.WorkingSetSize / 1024;
		(*vmrss_kb) = pmc.PeakWorkingSetSize / 1024;
		return 0;
#	else
		/// \todo find PRECOMPILER defines for others
		return 1;
#	endif

#endif

		return 1;
	}


#if defined(MPI_VERSION)

	int clusterMemoryUsageKb(long* vmrss_per_process, long* vmsize_per_process, int root, int np)
	{
		long vmrss_kb;
		long vmsize_kb;
		int ret_code = memoryUsageKb(&vmrss_kb, &vmsize_kb);

		if (ret_code != 0)
		{
			printf("Could not gather memory usage!\n");
			return ret_code;
		}

		MPI_Gather(&vmrss_kb, 1, MPI_UNSIGNED_LONG,
			vmrss_per_process, 1, MPI_UNSIGNED_LONG,
			root, MPI_COMM_WORLD);

		MPI_Gather(&vmsize_kb, 1, MPI_UNSIGNED_LONG,
			vmsize_per_process, 1, MPI_UNSIGNED_LONG,
			root, MPI_COMM_WORLD);

		return 0;
	}

	int globalMemoryUsageKb(long* global_vmrss, long* global_vmsize, int np)
	{
		long vmrss_per_process[np];
		long vmsize_per_process[np];
		int ret_code = clusterMemoryUsageKb(vmrss_per_process, vmsize_per_process, 0, np);

		if (ret_code != 0)
		{
			return ret_code;
		}

		*global_vmrss = 0;
		*global_vmsize = 0;
		for (int i = 0; i < np; i++)
		{
			*global_vmrss += vmrss_per_process[i];
			*global_vmsize += vmsize_per_process[i];
		}

		return 0;
	}

#endif



} // namespace IBK

/*! \file IBK_memory_usage.h
	\brief Contains utility functions that report memory currently used by the application.
*/

#endif // IBK_memory_usageH
