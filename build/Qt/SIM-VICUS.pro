TEMPLATE = subdirs

SUBDIRS = \
	CCM \
	DataIO \
	IBK \
	IBKMK \
	IntegratorFramework \
	Nandrad \
	NandradSolver \
	SuiteSparse \
	sundials \
	TiCPP \
	Zeppelin

# where to find the sub projects
#SIM-VICUS.file = ../../SIM-VICUS/projects/Qt/SIM-VICUS.pro
NandradSolver.file = ../../NandradSolver/projects/Qt/NandradSolver.pro

CCM.file = ../../externals/CCM/projects/Qt/CCM.pro
DataIO.file = ../../externals/DataIO/projects/Qt/DataIO.pro
IBK.file = ../../externals/IBK/projects/Qt/IBK.pro
IBKMK.file = ../../externals/IBKMK/projects/Qt/IBKMK.pro
IntegratorFramework.file = ../../externals/IntegratorFramework/projects/Qt/IntegratorFramework.pro
Nandrad.file = ../../externals/Nandrad/projects/Qt/Nandrad.pro
SuiteSparse.file = ../../externals/SuiteSparse/projects/Qt/SuiteSparse.pro
sundials.file = ../../externals/sundials/projects/Qt/sundials.pro
TiCPP.file = ../../externals/TiCPP/projects/Qt/TiCPP.pro
Zeppelin.file = ../../externals/Zeppelin/projects/Qt/Zeppelin.pro

# dependencies
NandradSolver.depends = DataIO CCM TiCPP IBK

CCM.depends = IBK TiCPP
DataIO.depends = IBK
IBKMK.depends = IBK
TiCPP.depends = IBK
IntegratorFramework.depends = IBK IBKMK sundials SuiteSparse
sundials.depends = SuiteSparse
Nandrad.depends = IBK TiCPP
Zeppelin.depends = IBK

