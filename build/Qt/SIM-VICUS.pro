TEMPLATE = subdirs

SUBDIRS = \
	../../externals/Vicus/projects/Qt/Vicus.pro \
	SIM-VICUS \
	QuaZIP \
	qwt \
	Vicus \
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
	Zeppelin \
	NandradCodeGenerator

# where to find the sub projects
SIM-VICUS.file = ../../SIM-VICUS/projects/Qt/SIM-VICUS.pro
NandradSolver.file = ../../NandradSolver/projects/Qt/NandradSolver.pro
NandradCodeGenerator.file = ../../NandradCodeGenerator/projects/Qt/NandradCodeGenerator.pro

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
QuaZIP.file = ../../externals/QuaZIP/projects/Qt/QuaZIP.pro
qwt.file = ../../externals/qwt/projects/Qt/qwt.pro
Vicus.file = ../../externals/Vicus/projects/Qt/Vicus.pro

# dependencies
NandradSolver.depends = DataIO CCM TiCPP IBK IntegratorFramework
NandradCodeCreator.depends = IBK
SIM-VICUS.depends = QuaZIP qwt Vicus Nandrad IBK TiCPP CCM

CCM.depends = IBK TiCPP
DataIO.depends = IBK
IBKMK.depends = IBK
TiCPP.depends = IBK
IntegratorFramework.depends = IBK IBKMK sundials SuiteSparse
sundials.depends = SuiteSparse
Nandrad.depends = IBK TiCPP
Zeppelin.depends = IBK
Vicus.depends = IBK TiCPP Nandrad

