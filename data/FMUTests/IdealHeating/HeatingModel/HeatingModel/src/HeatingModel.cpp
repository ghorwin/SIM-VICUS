/*

FMI Interface for FMU generated by FMICodeGenerator.

This file is part of FMICodeGenerator (https://github.com/ghorwin/FMICodeGenerator)

BSD 3-Clause License

Copyright (c) 2018, Andreas Nicolai
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of the copyright holder nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include <cstring>
#include <sstream>

#include "fmi2common/fmi2Functions.h"
#include "fmi2common/fmi2FunctionTypes.h"
#include "HeatingModel.h"

// FMI interface variables

#define FMI_INPUT_RoomTemperature 1
#define FMI_OUTPUT_HeatingPower 2


// *** Variables and functions to be implemented in user code. ***

// *** GUID that uniquely identifies this FMU code
const char * const InstanceData::GUID = "{a7f0d9ee-544d-11ec-a0e9-379d69f102d0}";

// *** Factory function, creates model specific instance of InstanceData-derived class
InstanceData * InstanceData::create() {
	return new HeatingModel; // caller takes ownership
}


HeatingModel::HeatingModel() :
	InstanceData()
{
	// initialize input variables and/or parameters
	m_realVar[FMI_INPUT_RoomTemperature] = 293.15;

	// initialize output variables
	m_realVar[FMI_OUTPUT_HeatingPower] = 0;


}


HeatingModel::~HeatingModel() {
}


// create a model instance
void HeatingModel::init() {
	logger(fmi2OK, "progress", "Starting initialization.");

	if (m_modelExchange) {
		// initialize states
		

		// TODO : implement your own initialization code here
	}
	else {
		// initialize states, these are used for our internal time integration
		

		// TODO : implement your own initialization code here

		// initialize integrator for co-simulation
		m_currentTimePoint = 0;
	}

	logger(fmi2OK, "progress", "Initialization complete.");
}


// model exchange: implementation of derivative and output update
void HeatingModel::updateIfModified() {
	if (!m_externalInputVarsModified)
		return;

	// get input variables
	double RoomTemperature = m_realVar[FMI_INPUT_RoomTemperature];


	// TODO : implement your code here

	// output variables
	m_realVar[FMI_OUTPUT_HeatingPower] = 0; // TODO : store your results here


	// reset externalInputVarsModified flag
	m_externalInputVarsModified = false;
}


// Co-simulation: time integration
void HeatingModel::integrateTo(double tCommunicationIntervalEnd) {

	// state of FMU before integration:
	//   m_currentTimePoint = t_IntervalStart;

	// get input variables
	double RoomTemperature = m_realVar[FMI_INPUT_RoomTemperature];

	double error = 296.15 - RoomTemperature;
	// normalize error 
	error /= 0.2; // 0.2 K tolerance

	double MAX_POWER = 100;

	double HeatingPower = 0.2*error*error*error;
	if (HeatingPower > 0)
		HeatingPower = std::min(HeatingPower, MAX_POWER);
	else
		HeatingPower = std::max(HeatingPower, -MAX_POWER);

	// output variables
	m_realVar[FMI_OUTPUT_HeatingPower] = HeatingPower;

	m_currentTimePoint = tCommunicationIntervalEnd;

	// state of FMU after integration:
	//   m_currentTimePoint = tCommunicationIntervalEnd;
}


void HeatingModel::computeFMUStateSize() {
	// store time, states and outputs
	m_fmuStateSize = sizeof(double)*1;
	// we store all cached variables

	// for all 4 maps, we store the size for sanity checks
	m_fmuStateSize += sizeof(int)*4;

	// serialization of the maps: first the valueRef, then the actual value

	m_fmuStateSize += (sizeof(int) + sizeof(double))*m_realVar.size();
	m_fmuStateSize += (sizeof(int) + sizeof(int))*m_integerVar.size();
	m_fmuStateSize += (sizeof(int) + sizeof(int))*m_boolVar.size(); // booleans are stored as int

	// strings are serialized in checkable format: first length, then zero-terminated string
	for (std::map<int, std::string>::const_iterator it = m_stringVar.begin();
		 it != m_stringVar.end(); ++it)
	{
		m_fmuStateSize += sizeof(int) + sizeof(int) + it->second.size() + 1; // add one char for \0
	}


	// other variables: distinguish between ModelExchange and CoSimulation
	if (m_modelExchange) {

		// TODO : store state variables and already computed derivatives

	}
	else {

		// TODO : store integrator state

	}
}


// macro for storing a POD and increasing the pointer to the linear memory array
#define SERIALIZE(type, storageDataPtr, value)\
{\
  *reinterpret_cast<type *>(storageDataPtr) = (value);\
  (storageDataPtr) = reinterpret_cast<char *>(storageDataPtr) + sizeof(type);\
}

// macro for retrieving a POD and increasing the pointer to the linear memory array
#define DESERIALIZE(type, storageDataPtr, value)\
{\
  (value) = *reinterpret_cast<type *>(storageDataPtr);\
  (storageDataPtr) = reinterpret_cast<const char *>(storageDataPtr) + sizeof(type);\
}


template <typename T>
bool deserializeMap(HeatingModel * obj, const char * & dataPtr, const char * typeID, std::map<int, T> & varMap) {
	// now de-serialize the maps: first the size (for checking), then each key-value pair
	int mapsize;
	DESERIALIZE(const int, dataPtr, mapsize);
	if (mapsize != static_cast<int>(varMap.size())) {
		std::stringstream strm;
		strm << "Bad binary data or invalid/uninitialized model data. "<< typeID << "-Map size mismatch.";
		obj->logger(fmi2Error, "deserialization", strm.str());
		return false;
	}
	for (int i=0; i<mapsize; ++i) {
		int valueRef;
		T val;
		DESERIALIZE(const int, dataPtr, valueRef);
		if (varMap.find(valueRef) == varMap.end()) {
			std::stringstream strm;
			strm << "Bad binary data or invalid/uninitialized model data. "<< typeID << "-Variable with value ref "<< valueRef
				 << " does not exist in "<< typeID << "-variable map.";
			obj->logger(fmi2Error, "deserialization", strm.str());
			return false;
		}
		DESERIALIZE(const T, dataPtr, val);
		varMap[valueRef] = val;
	}
	return true;
}



void HeatingModel::serializeFMUstate(void * FMUstate) {
	char * dataPtr = reinterpret_cast<char*>(FMUstate);
	if (m_modelExchange) {
		SERIALIZE(double, dataPtr, m_tInput);

		// TODO ModelExchange-specific serialization
	}
	else {
		SERIALIZE(double, dataPtr, m_currentTimePoint);

		// TODO CoSimulation-specific serialization
	}

	// write map size for checking
	int mapSize = static_cast<int>(m_realVar.size());
	SERIALIZE(int, dataPtr, mapSize);
	// now serialize all members of the map
	for (std::map<int,double>::const_iterator it = m_realVar.begin(); it != m_realVar.end(); ++it) {
		SERIALIZE(int, dataPtr, it->first);
		SERIALIZE(double, dataPtr, it->second);
	}
	mapSize = static_cast<int>(m_integerVar.size());
	SERIALIZE(int, dataPtr, mapSize);
	for (std::map<int,int>::const_iterator it = m_integerVar.begin(); it != m_integerVar.end(); ++it) {
		SERIALIZE(int, dataPtr, it->first);
		SERIALIZE(int, dataPtr, it->second);
	}
	mapSize = static_cast<int>(m_boolVar.size());
	SERIALIZE(int, dataPtr, mapSize);
	for (std::map<int,int>::const_iterator it = m_boolVar.begin(); it != m_boolVar.end(); ++it) {
		SERIALIZE(int, dataPtr, it->first);
		SERIALIZE(int, dataPtr, it->second);
	}
	mapSize = static_cast<int>(m_stringVar.size());
	SERIALIZE(int, dataPtr, mapSize);
	for (std::map<int, std::string>::const_iterator it = m_stringVar.begin();
		 it != m_stringVar.end(); ++it)
	{
		SERIALIZE(int, dataPtr, it->first);				// map key
		SERIALIZE(int, dataPtr, static_cast<int>(it->second.size()));		// string size
		std::memcpy(dataPtr, it->second.c_str(), it->second.size()+1); // also copy the trailing \0
		dataPtr += it->second.size()+1;
	}
}


bool HeatingModel::deserializeFMUstate(void * FMUstate) {
	const char * dataPtr = reinterpret_cast<const char*>(FMUstate);
	if (m_modelExchange) {
		DESERIALIZE(const double, dataPtr, m_tInput);

		// TODO ModelExchange-specific deserialization
		m_externalInputVarsModified = true;
	}
	else {
		DESERIALIZE(const double, dataPtr, m_currentTimePoint);

		// TODO CoSimulation-specific deserialization
	}

	if (!deserializeMap(this, dataPtr, "real", m_realVar))
		return false;
	if (!deserializeMap(this, dataPtr, "integer", m_integerVar))
		return false;
	if (!deserializeMap(this, dataPtr, "boolean", m_boolVar))
		return false;

	// special handling for deserialization of string map
	int mapsize;
	DESERIALIZE(const int, dataPtr, mapsize);
	if (mapsize != static_cast<int>(m_stringVar.size())) {
		logger(fmi2Error, "deserialization", "Bad binary data or invalid/uninitialized model data. string-variable map size mismatch.");
		return false;
	}
	for (int i=0; i<mapsize; ++i) {
		int valueRef;
		DESERIALIZE(const int, dataPtr, valueRef);
		if (m_stringVar.find(valueRef) == m_stringVar.end()) {
			std::stringstream strm;
			strm << "Bad binary data or invalid/uninitialized model data. string-variable with value ref "<< valueRef
				 << " does not exist in real variable map.";
			logger(fmi2Error, "deserialization", strm.str());
			return false;
		}
		// get length of string
		int strLen;
		DESERIALIZE(const int, dataPtr, strLen);
		// create a string of requested length
		std::string s(static_cast<size_t>(strLen), ' ');
		// copy contents of string
		std::memcpy(&s[0], dataPtr, static_cast<size_t>(strLen)); // do not copy the trailing \0
		dataPtr += strLen;
		// check that next character is a \0
		if (*dataPtr != '\0') {
			std::stringstream strm;
			strm << "Bad binary data. string-variable with value ref "<< valueRef
				 << " does not have a trailing \0.";
			logger(fmi2Error, "deserialization", strm.str());
			return false;
		}
		++dataPtr;
		// replace value in map
		m_stringVar[valueRef] = s;
	}

	return true;
}


