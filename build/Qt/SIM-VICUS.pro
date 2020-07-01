TEMPLATE = subdirs

SUBDIRS = SIM-VICUS \
		  IBK \
		  CCM \
		  DataIO \
		  TiCPP

# where to find the sub projects
SIM-VICUS.file = ../../SIM-VICUS/projects/Qt/SIM-VICUS.pro
#CCD2C6BConverter.file = ../../CCD2C6BConverter/projects/Qt/CCD2C6BConverter.pro

CCM.file = ../../externals/CCM/projects/Qt/CCM.pro
IBK.file = ../../externals/IBK/projects/Qt/IBK.pro
DataIO.file = ../../externals/DataIO/projects/Qt/DataIO.pro
TiCPP.file = ../../externals/TiCPP/projects/Qt/TiCPP.pro

# dependencies
SIM-VICUS.depends = DataIO CCM TiCPP IBK
#CCD2C6BConverter.depends = CCM IBK

CCM.depends = IBK TiCPP
DataIO.depends = IBK
TiCPP.depends = IBK

