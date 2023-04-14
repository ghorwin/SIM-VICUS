#include "SVImportDxfDialog.h"
#include "ui_SVImportDxfDialog.h"
#include <libdxfrw.h>
#include "SVSettings.h"
#include "SVUndoAddDrawing.h"

SVImportDxfDialog::SVImportDxfDialog(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::SVImportDxfDialog)
{
	m_ui->setupUi(this);

	m_ui->lineEditFileName->setup(m_lastFilePath, true, true, tr("DXF-Files (*.dxf)"),
								  SVSettings::instance().m_dontUseNativeDialogs);

	// TODO Maik: populate combobox unit

}

SVImportDxfDialog::~SVImportDxfDialog()
{
	delete m_ui;
}

void SVImportDxfDialog::run() {

	if (exec()) {

		m_drawing = VICUS::Drawing();
		readDxfFile(&m_drawing);
		m_drawing.updatePointer();

		SVUndoAddDrawing *undo = new SVUndoAddDrawing("", m_drawing);
		undo->push();

		qDebug() << "Layer:";
		for(size_t i = 0; i < m_drawing.m_layer.size(); i++){
			qDebug() << m_drawing.m_layer[i].m_name;
		}

		qDebug() << "Lines:";
		for(size_t i = 0; i < m_drawing.m_lines.size(); i++){
			qDebug() << "x1: " << QString::number(m_drawing.m_lines[i].m_line.m_p1.m_x) << " y1: " << QString::number(m_drawing.m_lines[i].m_line.m_p1.m_y);
			qDebug() << "x2: " << QString::number(m_drawing.m_lines[i].m_line.m_p2.m_x) << " y2: " << QString::number(m_drawing.m_lines[i].m_line.m_p2.m_y);
		}

		qDebug() << "Points:";
		for(size_t i = 0; i < m_drawing.m_points.size(); i++){
			qDebug() << "x: " << QString::number(m_drawing.m_points[i].m_point.m_x) << " y: " << QString::number(m_drawing.m_points[i].m_point.m_y);
		}

		qDebug() << "LWPolyLines:";
		for(size_t i = 0; i < m_drawing.m_lwpolylines.size(); i++){
			for(size_t j = 0; j < m_drawing.m_lwpolylines[i].m_lwpolyline.size(); j++) {
				qDebug() << "x: " << QString::number(m_drawing.m_lwpolylines[i].m_lwpolyline[j].m_x) << " y: " << QString::number(m_drawing.m_lwpolylines[i].m_lwpolyline[j].m_y);
			}
		}

		qDebug() << "PolyLines:";
		for(size_t i = 0; i < m_drawing.m_polylines.size(); i++){
			for(size_t j = 0; j < m_drawing.m_polylines[i].m_polyline.size(); j++) {
				qDebug() << "x: " << QString::number(m_drawing.m_polylines[i].m_polyline[j].m_x) << " y: " << QString::number(m_drawing.m_polylines[i].m_polyline[j].m_y);
			}
		}
	}
}


void SVImportDxfDialog::readDxfFile(VICUS::Drawing *drawing) {
	DRW_InterfaceImpl *drwIntImpl = new DRW_InterfaceImpl(drawing);
	dxfRW *dxf = new dxfRW(m_ui->lineEditFileName->filename().toUtf8().data());
	bool read = false;

	if(!dxf->read(drwIntImpl, read))
	{
		qDebug() << "error while reading";
		exit(0);
	}
}

DRW_InterfaceImpl::DRW_InterfaceImpl(VICUS::Drawing *drawing){
	this->drawing = drawing;
}

void DRW_InterfaceImpl::addHeader(const DRW_Header* /*data*/){}
void DRW_InterfaceImpl::addLType(const DRW_LType& /*data*/){}
void DRW_InterfaceImpl::addLayer(const DRW_Layer& data){

	// initialise struct Layer and populate the attributes
	VICUS::Drawing::Layer newLayer;

	// name of layer
	newLayer.m_name = QString::fromStdString(data.name);

	// read linewidth from dxf file, convert to double using lineWidth2dxfInt from libdxfrw
	newLayer.m_lineWidth = (double)DRW_LW_Conv::lineWidth2dxfInt(data.lWeight);


	// create QColor and fill with color value of layer
	// there are two ways to store color in a dxf layer:
	// color: integer value from 0-255, predefined color values from dxfColors in drw_objects.h
	// color24: 24bit RGB coding

	newLayer.m_visible = true;


	if(data.color != -1){

		newLayer.m_color = QColor(DRW::dxfColors[data.color][0], DRW::dxfColors[data.color][1], DRW::dxfColors[data.color][2]);
	}

	if(data.color24 != -1){
		int red = (data.color24 >> 16) & 0xFF;
		int green = (data.color24 >> 8) & 0xFF;
		int blue = data.color24 & 0xFF;
		newLayer.m_color = QColor(red, green, blue);
	}

	// Push new layer into vector<Layer*> m_layer
	drawing->m_layer.push_back(newLayer);
}
void DRW_InterfaceImpl::addDimStyle(const DRW_Dimstyle& /*data*/){}
void DRW_InterfaceImpl::addVport(const DRW_Vport& /*data*/){}
void DRW_InterfaceImpl::addTextStyle(const DRW_Textstyle& /*data*/){}
void DRW_InterfaceImpl::addAppId(const DRW_AppId& /*data*/){}
void DRW_InterfaceImpl::addBlock(const DRW_Block& /*data*/){}
void DRW_InterfaceImpl::setBlock(const int /*handle*/){}
void DRW_InterfaceImpl::endBlock(){}
void DRW_InterfaceImpl::addPoint(const DRW_Point& data){

	struct VICUS::Drawing::Point newPoint;

	//create new point, insert into vector m_points from drawing
	newPoint.m_point = IBKMK::Vector2D(data.basePoint.x, data.basePoint.y);
	newPoint.m_lineWidth = data.thickness;
	newPoint.m_layername = QString::fromStdString(data.layer);

	if(data.color != -1){
		newPoint.m_color = QColor(DRW::dxfColors[data.color][0], DRW::dxfColors[data.color][1], DRW::dxfColors[data.color][2]);
	}

	drawing->m_points.push_back(newPoint);

}
void DRW_InterfaceImpl::addLine(const DRW_Line& data){

	VICUS::Drawing::Line newLine;

	//create new line, insert into vector m_lines from drawing
	newLine.m_line = IBK::Line(data.basePoint.x, data.basePoint.y, data.secPoint.x, data.secPoint.y);
	newLine.m_lineWidth = data.thickness;
	newLine.m_lineWidth = data.lWeight;
	newLine.m_layername = QString::fromStdString(data.layer);

	if(data.color > 0){
		if(data.color == 256){
			newLine.m_color = 256;
		}
		else{
			newLine.m_color = QColor(DRW::dxfColors[data.color][0], DRW::dxfColors[data.color][1], DRW::dxfColors[data.color][2]);
		}

	}

	drawing->m_lines.push_back(newLine);
}

void DRW_InterfaceImpl::addRay(const DRW_Ray& /*data*/){}
void DRW_InterfaceImpl::addXline(const DRW_Xline& /*data*/){}
void DRW_InterfaceImpl::addArc(const DRW_Arc& /*data*/){}
void DRW_InterfaceImpl::addCircle(const DRW_Circle& /*data*/){}
void DRW_InterfaceImpl::addEllipse(const DRW_Ellipse& /*data*/){}
void DRW_InterfaceImpl::addLWPolyline(const DRW_LWPolyline& data){

	struct VICUS::Drawing::LWPolyLine newlwpolyline;
	newlwpolyline.m_lwpolyline = std::vector<IBKMK::Vector2D>();

	// iterate over data.vertlist, insert all vertices of Polyline into vector
	for(size_t i = 0; i < data.vertlist.size(); i++){
		newlwpolyline.m_lwpolyline.push_back(IBKMK::Vector2D(data.vertlist[i]->x, data.vertlist[i]->y));
	}

	newlwpolyline.m_layername = QString::fromStdString(data.layer);

	if(data.color != -1){
		newlwpolyline.m_color = QColor(DRW::dxfColors[data.color][0], DRW::dxfColors[data.color][1], DRW::dxfColors[data.color][2]);
	}

	newlwpolyline.m_polyline_flag = data.flags;

	// insert vector into m_lines[data.layer] vector
	drawing->m_lwpolylines.push_back(newlwpolyline);
}
void DRW_InterfaceImpl::addPolyline(const DRW_Polyline& data){

	struct VICUS::Drawing::PolyLine newpolyline;
	newpolyline.m_polyline = std::vector<IBKMK::Vector2D>();

	// iterateover data.vertlist, insert all vertices of Polyline into vector
	for(size_t i = 0; i < data.vertlist.size(); i++){
		newpolyline.m_polyline.push_back(IBKMK::Vector2D(data.vertlist[i]->basePoint.x, data.vertlist[i]->basePoint.y));
	}

	newpolyline.m_layername = QString::fromStdString(data.layer);

	if(data.color != -1){
		newpolyline.m_color = QColor(DRW::dxfColors[data.color][0], DRW::dxfColors[data.color][1], DRW::dxfColors[data.color][2]);
	}


	newpolyline.m_polyline_flag = data.flags;

	// insert vector into m_lines[data.layer] vector
	drawing->m_polylines.push_back(newpolyline);
}
void DRW_InterfaceImpl::addSpline(const DRW_Spline* /*data*/){}
void DRW_InterfaceImpl::addKnot(const DRW_Entity & /*data*/){}
void DRW_InterfaceImpl::addInsert(const DRW_Insert& /*data*/){}
void DRW_InterfaceImpl::addTrace(const DRW_Trace& /*data*/){}
void DRW_InterfaceImpl::add3dFace(const DRW_3Dface& /*data*/){}
void DRW_InterfaceImpl::addSolid(const DRW_Solid& /*data*/){}
void DRW_InterfaceImpl::addMText(const DRW_MText& /*data*/){}
void DRW_InterfaceImpl::addText(const DRW_Text& /*data*/){}
void DRW_InterfaceImpl::addDimAlign(const DRW_DimAligned */*data*/){}
void DRW_InterfaceImpl::addDimLinear(const DRW_DimLinear */*data*/){}
void DRW_InterfaceImpl::addDimRadial(const DRW_DimRadial */*data*/){}
void DRW_InterfaceImpl::addDimDiametric(const DRW_DimDiametric */*data*/){}
void DRW_InterfaceImpl::addDimAngular(const DRW_DimAngular */*data*/){}
void DRW_InterfaceImpl::addDimAngular3P(const DRW_DimAngular3p */*data*/){}
void DRW_InterfaceImpl::addDimOrdinate(const DRW_DimOrdinate */*data*/){}
void DRW_InterfaceImpl::addLeader(const DRW_Leader */*data*/){}
void DRW_InterfaceImpl::addHatch(const DRW_Hatch */*data*/){}
void DRW_InterfaceImpl::addViewport(const DRW_Viewport& /*data*/){}
void DRW_InterfaceImpl::addImage(const DRW_Image */*data*/){}
void DRW_InterfaceImpl::linkImage(const DRW_ImageDef */*data*/){}
void DRW_InterfaceImpl::addComment(const char* /*comment*/){}

// no need to implement
void DRW_InterfaceImpl::writeHeader(DRW_Header& /*data*/){}
void DRW_InterfaceImpl::writeBlocks(){}
void DRW_InterfaceImpl::writeBlockRecords(){}
void DRW_InterfaceImpl::writeEntities(){}
void DRW_InterfaceImpl::writeLTypes(){}
void DRW_InterfaceImpl::writeLayers(){}
void DRW_InterfaceImpl::writeTextstyles(){}
void DRW_InterfaceImpl::writeVports(){}
void DRW_InterfaceImpl::writeDimstyles(){}
void DRW_InterfaceImpl::writeAppId(){}
