This directory shall be checked out below the src directory of an FMU project. Example:

TheraklesFMI_v2
├── projects
│   ├── Qt
│   │   ├── TheraklesFMIModelExchange_v2.pro
│   └── VC10
│       ├── TheraklesFMU_v2.vcxproj
│       └── TheraklesFMU_v2.vcxproj.filters
└── src
	├── fmi2common                      --> svn:externals "^/../FMU/trunk/fmi2common fmi2common"
	│   ├── fmi2Functions_complete.h
	│   ├── fmi2Functions.cpp
	│   ├── fmi2Functions.h
	│   ├── fmi2FunctionTypes.h
	│   ├── fmi2TypesPlatform.h
	│   ├── InstanceDataCommon.cpp
	│   └── InstanceDataCommon.h
	├── InstanceData.cpp
	└── InstanceData.h

