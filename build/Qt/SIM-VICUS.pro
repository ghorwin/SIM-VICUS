TEMPLATE = subdirs

SUBDIRS = \
        SIM-VICUS \
        QuaZIP \
        qwt \
        QtExt \
        Vicus \
        CCM \
        Shading \
        DataIO \
        IBK \
        IBKMK \
        IntegratorFramework \
        Nandrad \
        NandradSolver \
        NandradModel \
        SuiteSparse \
        sundials \
        TiCPP \
        Zeppelin \
        GenericBuildings\
        NandradCodeGenerator \
        NandradFMUGenerator \
        View3D \
        NandradSolverFMI

# where to find the sub projects
SIM-VICUS.file = ../../SIM-VICUS/projects/Qt/SIM-VICUS.pro
NandradSolver.file = ../../NandradSolver/projects/Qt/NandradSolver.pro
NandradSolverFMI.file = ../../NandradSolverFMI/projects/Qt/NandradSolverFMI.pro
View3D.file = ../../View3D/projects/Qt/View3D.pro
NandradCodeGenerator.file = ../../NandradCodeGenerator/projects/Qt/NandradCodeGenerator.pro
NandradFMUGenerator.file = ../../NandradFMUGenerator/projects/Qt/NandradFMUGenerator.pro

CCM.file = ../../externals/CCM/projects/Qt/CCM.pro
Shading.file = ../../externals/Shading/projects/Qt/Shading.pro
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
QtExt.file = ../../externals/QtExt/projects/Qt/QtExt.pro
GenericBuildings.file = ../../externals/GenericBuildings/projects/Qt/GenericBuildings.pro
NandradModel.file = ../../NandradSolver/projects/Qt/NandradModel.pro

# dependencies
NandradSolver.depends = NandradModel DataIO CCM TiCPP IBK IntegratorFramework Nandrad IBKMK
NandradSolverFMI.depends = NandradModel DataIO CCM TiCPP IBK IntegratorFramework Nandrad IBKMK
NandradCodeGenerator.depends = IBK
SIM-VICUS.depends = QuaZIP qwt Vicus Nandrad IBK TiCPP CCM QtExt Zeppelin GenericBuildings

CCM.depends = IBK TiCPP
Shading.depends = IBK TiCPP CCM IBKMK DataIO
DataIO.depends = IBK
IBKMK.depends = IBK
TiCPP.depends = IBK
QtExt.depends = IBK IBKMK
GenericBuildings.depends = IBK IBKMK TiCPP
IntegratorFramework.depends = IBK IBKMK sundials SuiteSparse
sundials.depends = SuiteSparse
Nandrad.depends = IBK TiCPP IBKMK
Zeppelin.depends = IBK
Vicus.depends = IBK TiCPP Nandrad IBKMK
NandradModel.depends = DataIO CCM TiCPP IBK IntegratorFramework Nandrad IBKMK

