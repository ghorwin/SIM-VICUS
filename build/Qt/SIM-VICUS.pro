TEMPLATE = subdirs

SUBDIRS = CCMEditor \
#		  CCD2C6BConverter \
		  IBK \
		  CCM \
		  qwt \
		  QtExt \
		  SciChart \
		  DataIO \
		  TiCPP

# where to find the sub projects
CCMEditor.file = ../../CCMEditor/projects/Qt/CCMEditor.pro
#CCD2C6BConverter.file = ../../CCD2C6BConverter/projects/Qt/CCD2C6BConverter.pro

CCM.file = ../../externals/CCM/projects/Qt/CCM.pro
IBK.file = ../../externals/IBK/projects/Qt/IBK.pro
qwt.file = ../../externals/qwt/projects/Qt/qwt.pro
SciChart.file = ../../externals/SciChart/projects/Qt/SciChart.pro
DataIO.file = ../../externals/DataIO/projects/Qt/DataIO.pro
TiCPP.file = ../../externals/TiCPP/projects/Qt/TiCPP.pro
QtExt.file = ../../externals/QtExt/projects/Qt/QtExt.pro

# dependencies
CCMEditor.depends = QtExt DataIO CCM TiCPP IBK qwt SciChart
#CCD2C6BConverter.depends = CCM IBK

CCM.depends = IBK TiCPP
DataIO.depends = IBK
TiCPP.depends = IBK
SciChart.depends = IBK DataIO TiCPP QtExt qwt
QtExt.depends = IBK

win32 {
	SUBDIRS += EmfEngine
	# where to find the sub projects
	EmfEngine.file = ../../externals/EmfEngine/projects/Qt/EmfEngine.pro
	CCMEditor.depends += EmfEngine
}
