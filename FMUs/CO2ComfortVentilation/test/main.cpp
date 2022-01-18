#include <iostream>
#include <cstdlib>
#include <iomanip>
#include <algorithm>

#include "../src/LinearSpline.h"
#include "../src/CO2ComfortVentilation.h"

#include "../src/fmi2common/fmi2Functions.h"

// Helper functions for FMU Master
void logHandler(fmi2ComponentEnvironment env, fmi2String context, fmi2Status status, fmi2String category, fmi2String msg, ...) {
	std::cout << msg << std::endl;
}

void stepFinishedHandler(fmi2ComponentEnvironment, fmi2Status) {
	// nothing to do
}

struct CallbackFunctions {
	CallbackFunctions() :
		logger(logHandler),
		allocateMemory(NULL),
		freeMemory(NULL),
		stepFinished(stepFinishedHandler),
		componentEnvironment(NULL)
	{}

	const fmi2CallbackLogger         logger;
	const fmi2CallbackAllocateMemory allocateMemory;
	const fmi2CallbackFreeMemory     freeMemory;
	const fmi2StepFinished           stepFinished;
	const fmi2ComponentEnvironment   componentEnvironment;
};

int main(int argc, char * argv[]) {


	try {
		CallbackFunctions cbf;

		// We simulate a FMU CoSim Master
		void * c = fmi2Instantiate("CO2Balance",
			fmi2CoSimulation,
			"{17d077d8-59b4-11ec-8093-3ce1a14c97e0}",
			"d:/Programmierung/SIM-VICUS/FMUs/CO2ComfortVentilation/data",
			(fmi2CallbackFunctions*)&cbf,
			fmi2True,
			fmi2True);

		if (c == NULL) {
			std::cout << "Error instantiating model." << std::endl;
			return EXIT_FAILURE;
		}

		// pre-initialization, set results root dir
		fmi2ValueReference svr[1];
		svr[0] = 42;
		const char * const svalues1[1] = {
			"d:/Programmierung/SIM-VICUS/FMUs/CO2ComfortVentilation/data"
		};
		fmi2SetString(c, svr, 1, svalues1);


		//// *** Model Initialization ***

		// Setup simulation frame
		fmi2SetupExperiment(c,
			fmi2True,	// use reltol
			1e-5,		// reltol
			0,			// starttime
			fmi2True,	// use stoptime
			2 * 24 * 3600		// stop time
		);

		// Enter initialization mode
		if (fmi2EnterInitializationMode(c) != fmi2OK) {
			return EXIT_FAILURE;
		}

		// Set input variables
		// Output variables
		const unsigned int NUM_REAL_INPUT_VARIABLES = 4;
		const unsigned int NUM_REAL_OUTPUT_VARIABLES = 6;
		fmi2ValueReference invr[NUM_REAL_INPUT_VARIABLES];
		invr[0] = 1;	// AmbientTemperature	= 1
		invr[1] = 1001;	// RoomAirTemperature 1	= 1001
		invr[2] = 1003;	// RoomAirTemperature 3	= 1003
		invr[3] = 1007;	// RoomAirTemperature 7	= 1007
		fmi2ValueReference outvr[NUM_REAL_OUTPUT_VARIABLES];
		outvr[0] = 2001;// AirChangeRate 1		= 2001
		outvr[1] = 2003;// AirChangeRate 3		= 2003
		outvr[2] = 2007;// AirChangeRate 7		= 2007
		outvr[3] = 3001;// CO2Density 1			= 3001
		outvr[4] = 3003;// CO2Density 3			= 3003
		outvr[5] = 3007;// CO2Density 7			= 3007

		double realOutputValues[NUM_REAL_OUTPUT_VARIABLES];
		double realInputValues[NUM_REAL_INPUT_VARIABLES];

		// create input values
		realInputValues[0] = 273.15;
		realInputValues[1] = 295.15;
		realInputValues[2] = 295.15;
		realInputValues[3] = 295.15;

		// set inputs
		fmi2Status res = fmi2SetReal(c, invr, NUM_REAL_INPUT_VARIABLES, realInputValues);
		if (res != fmi2OK) {
			return EXIT_FAILURE;
		}

		// get outputs of FMU
		res = fmi2GetReal(c, outvr, NUM_REAL_OUTPUT_VARIABLES, realOutputValues);
		if (res != fmi2OK) {
			return EXIT_FAILURE;
		}

		// Leave initialization mode
		fmi2ExitInitializationMode(c);

		// reserve memory for serializing FMU state
		fmi2FMUstate state = NULL;

		//std::ofstream resDump("../../../data/FMU/Reihenhaus_Interface3/unpack/Project/log/OutputValues_CS.txt");
		//resDump << std::fixed;
		//std::cout << std::fixed;

		// Start integration loop
		double dt = 60; // 1 minute steps
		unsigned int i = 0;

		while (i*dt < 24 * 3600) {
			double t = i*dt;

			// serialize FMU state
			res = fmi2GetFMUstate(c, &state);
			// integrate and iterate over FMU

			// integrate step
			fmi2DoStep(c, t, dt, fmi2True);

			// get outputs of FMU
			fmi2Status res = fmi2GetReal(c, outvr, NUM_REAL_OUTPUT_VARIABLES, realOutputValues);
			if (res != fmi2OK) {
				return EXIT_FAILURE;
			}

			std::cout << "t = " << std::right << std::setprecision(3) << t / 3600.0 << " h,"
				"\tTAmbient = " << std::right << std::setprecision(2) << realInputValues[0] - 273.15 <<
				"\tTRoom = " << std::right << std::setprecision(2) << realInputValues[1] - 273.15 <<
				"C,\tnAir=" << std::right << std::setprecision(4) << realOutputValues[0] <<
				" 1/s\trhoCO2=" << std::right << std::setprecision(4) << realOutputValues[3] * 1e+06 << " ppm" << std::endl;

			double TAmbient = realInputValues[0];
			// set convective heating load
			for(unsigned int i = 0; i < 3; ++i) {
				double &TRoom = realInputValues[i + 1];
				double nRoom = realOutputValues[i];
				// recalculate
				TRoom += dt * nRoom * (TAmbient - TRoom);
			}

			// set new values
			res = fmi2SetReal(c, invr, NUM_REAL_INPUT_VARIABLES, realInputValues);
			if (res != fmi2OK) {
				return EXIT_FAILURE;
			}

			++i;
		}


		// free FMU state
		fmi2FreeFMUstate(c, &state);

		// free FMU instance
		fmi2FreeInstance(c);

		return EXIT_SUCCESS;
	}
	catch(std::exception & ex) {
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

