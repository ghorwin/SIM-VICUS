

# on Unix and Apple we want really detailed warnings
if (UNIX)
	ADD_DEFINITIONS( -Wall )
	# our code does not check for errno values, so we can skip this feature
	ADD_DEFINITIONS( -fno-math-errno )
endif (UNIX)


#
# combined thread checker, openmp, icc and gcc support for linux
#
IF ( UNIX )

	IF ( NOT APPLE )

		# *** Linux ***

		IF ( CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX )

			IF ( USE_OMP )

				find_package(OpenMP REQUIRED)
				SET (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
				SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")

				#ADD_DEFINITIONS( -fopenmp -fPIC )
			ELSE ( USE_OMP )

				MESSAGE( STATUS "No OpenMP support - ignoring unknown pragmas")
				ADD_DEFINITIONS( -Wunknown-pragmas )

			ENDIF ( USE_OMP )

		ELSE ( CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX )

			MESSAGE( STATUS "ICC FOUND" )

			IF ( USE_THREAD_CHECKER )

				# thread checker replaces complete cxx and c flags
				MESSAGE( STATUS "THREADCHECKER AND OPENMP SET" )
				find_package(OpenMP REQUIRED)
				SET( CMAKE_CXX_FLAGS "-g -O0" )
				SET( CMAKE_C_FLAGS "-g -O0" )
				SET (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
				SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")

			ELSEIF ( USE_THREAD_CHECKER )

				find_package(OpenMP REQUIRED)
				SET (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
				SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")

				#		  ADD_DEFINITIONS( -openmp )
			ENDIF ( USE_THREAD_CHECKER )

		ENDIF ( CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX )

	ELSE ( NOT APPLE )

		# *** Mac ***

		IF ( USE_OMP )
			set(OPENMP_LIBRARIES "/usr/local/Cellar/gcc/8.1.0/lib/gcc/8")
			set(OPENMP_INCLUDES "/usr/local/Cellar/gcc/8.1.0/include/c++/8.1.0")

			set(OpenMP_C "${CMAKE_C_COMPILER}")
			set(OpenMP_C_FLAGS "-fopenmp")
			set(OpenMP_C_LIB_NAMES "libgomp")

			set(OpenMP_CXX "${CMAKE_CXX_COMPILER}")
			set(OpenMP_CXX_FLAGS "-fopenmp")
			set(OpenMP_CXX_LIB_NAMES "libgomp")

			set(OpenMP_libgomp_LIBRARY libgomp)

			OPTION (USE_OpenMP "Use OpenMP to enamble <omp.h>" ON)

			MESSAGE( STATUS "Checking for OpenMP support on Mac")
			MESSAGE( STATUS "OpenMP_C_FLAGS = ${OpenMP_C_FLAGS}" )
			MESSAGE( STATUS "OpenMP_CXX_FLAGS = ${OpenMP_CXX_FLAGS}" )
			find_package(OpenMP REQUIRED)

			SET (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
			SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")

		ELSE ( USE_OMP )

			MESSAGE( STATUS "No OpenMP support - ignoring unknown pragmas")
			ADD_DEFINITIONS( -Wunknown-pragmas )

		ENDIF ( USE_OMP )


	ENDIF ( NOT APPLE )

ELSE ( UNIX  )

	# *** Windows ***

	IF ( CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX )
		# MinGW compiler
		MESSAGE( STATUS "GCC FOUND")

		IF ( USE_OMP )

			find_package(OpenMP REQUIRED)
			SET (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
			SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")

			#ADD_DEFINITIONS( -fopenmp )
		ELSE ( USE_OMP )

			ADD_DEFINITIONS( -Wunknown-pragmas )

		ENDIF ( USE_OMP )

	ENDIF ( CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX )

	IF (MSVC)
		# VisualStudio compiler
		IF ( USE_OMP )
			find_package(OpenMP REQUIRED)
			SET (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
			SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")

		ELSE ( USE_OMP )

			MESSAGE( STATUS "No OpenMP support - ignoring unknown pragmas")
			#ADD_DEFINITIONS( -Wunknown-pragmas )

		ENDIF ( USE_OMP )

	ENDIF (MSVC)
ENDIF( UNIX )
