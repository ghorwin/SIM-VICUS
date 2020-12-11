TEMPLATE = subdirs

SUBDIRS = \
    HydraulicNetworkTest \
    SuiteSparse \
    IBK

# where to find the sub projects
HydraulicNetworkTest.file = ../../HydraulicNetworkTest/projects/Qt/HydraulicNetworkTest.pro
SuiteSparse.file = ../../SuiteSparse/projects/Qt/SuiteSparse.pro
IBK.file = ../../IBK/projects/Qt/IBK.pro

# dependencies
HydraulicNetworkTest.depends = IBK

