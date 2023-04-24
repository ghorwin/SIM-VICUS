#include "SVImportDxfDialog.h"
#include "ui_SVImportDxfDialog.h"

#include <libdxfrw.h>

#include "SVSettings.h"
#include "SVUndoAddDrawing.h"
#include "SVStyle.h"

SVImportDxfDialog::SVImportDxfDialog(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::SVImportDxfDialog)
{
	m_ui->setupUi(this);

	m_ui->lineEditFileName->setup(m_lastFilePath, true, true, tr("DXF-Files (*.dxf)"),
								  SVSettings::instance().m_dontUseNativeDialogs);

}

SVImportDxfDialog::~SVImportDxfDialog()
{
	delete m_ui;
}

void SVImportDxfDialog::run() {

	if (exec()) {

		m_drawing = VICUS::Drawing();
		m_drawing.m_zcounter = 0;
		if (readDxfFile(m_drawing)) {

			m_drawing.updatePointer();
			switch(m_ui->comboBoxUnit->currentIndex()) {
				case 0: // 10^0
					m_drawing.m_scalingFactor = 1;
					break;
				case 1: // 10^-2
					m_drawing.m_scalingFactor = 0.01;
					break;
				case 2: // 10^-3
					m_drawing.m_scalingFactor = 0.001;
					break;
			}

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

			qDebug() << "PolyLines:";
			for(size_t i = 0; i < m_drawing.m_polylines.size(); i++){
				for(size_t j = 0; j < m_drawing.m_polylines[i].m_polyline.size(); j++) {
					qDebug() << "x: " << QString::number(m_drawing.m_polylines[i].m_polyline[j].m_x) << " y: " << QString::number(m_drawing.m_polylines[i].m_polyline[j].m_y);
				}
			}
		}

	}
	else {
		// TODO: write error to log text edit or show QMessageBox
	}
}


bool SVImportDxfDialog::readDxfFile(VICUS::Drawing &drawing) {
	DRW_InterfaceImpl *drwIntImpl = new DRW_InterfaceImpl(&drawing);
	dxfRW *dxf = new dxfRW(m_ui->lineEditFileName->filename().toUtf8().data());
	bool read = false;

	return dxf->read(drwIntImpl, read);
}


DRW_InterfaceImpl::DRW_InterfaceImpl(VICUS::Drawing *drawing){
	this->drawing = drawing;
}

void DRW_InterfaceImpl::addHeader(const DRW_Header* /*data*/){}
void DRW_InterfaceImpl::addLType(const DRW_LType& /*data*/){}
void DRW_InterfaceImpl::addLayer(const DRW_Layer& data){

	if(m_activeBlock != nullptr) return;

	// initialise struct Layer and populate the attributes
	VICUS::Drawing::Layer newLayer;

	// name of layer
	newLayer.m_name = QString::fromStdString(data.name);

	// read linewidth from dxf file, convert to double using lineWidth2dxfInt from libdxfrw
	newLayer.m_lineWeight = DRW_LW_Conv::lineWidth2dxfInt(data.lWeight);

	newLayer.m_visible = true;


	if(data.color != 256 && data.color != 7){
		newLayer.m_color = QColor(DRW::dxfColors[data.color][0], DRW::dxfColors[data.color][1], DRW::dxfColors[data.color][2]);
	}else{
		newLayer.m_color = SVStyle::instance().m_defaultDrawingColor;
	}

	// Push new layer into vector<Layer*> m_layer
	drawing->m_layer.push_back(newLayer);
}


void DRW_InterfaceImpl::addDimStyle(const DRW_Dimstyle& /*data*/){}
void DRW_InterfaceImpl::addVport(const DRW_Vport& /*data*/){}
void DRW_InterfaceImpl::addTextStyle(const DRW_Textstyle& /*data*/){}
void DRW_InterfaceImpl::addAppId(const DRW_AppId& /*data*/){}
void DRW_InterfaceImpl::addBlock(const DRW_Block& data){

	VICUS::Drawing::Block newBlock;

	newBlock.m_name = QString::fromStdString(data.name);

	newBlock.m_color = QColor();

	newBlock.m_lineWeight = 0;

	drawing->m_blocks.push_back(newBlock);

	m_activeBlock = &newBlock;

}
void DRW_InterfaceImpl::setBlock(const int /*handle*/){}
void DRW_InterfaceImpl::endBlock(){

	m_activeBlock = nullptr;

}
void DRW_InterfaceImpl::addPoint(const DRW_Point& data){

	if(m_activeBlock != nullptr) return;

	VICUS::Drawing::Point newPoint;
	newPoint.m_zposition = drawing->m_zcounter;
	drawing->m_zcounter++;


	//create new point, insert into vector m_points from drawing
	newPoint.m_point = IBKMK::Vector2D(data.basePoint.x, data.basePoint.y);
	newPoint.m_lineWeight = DRW_LW_Conv::lineWidth2dxfInt(data.lWeight);
	newPoint.m_layername = QString::fromStdString(data.layer);

	if(!(data.color == 256 || data.color == 7)) {
		newPoint.m_color = QColor(DRW::dxfColors[data.color][0], DRW::dxfColors[data.color][1], DRW::dxfColors[data.color][2]);
	}else{
		newPoint.m_color = QColor();
	}

	drawing->m_points.push_back(newPoint);

}
void DRW_InterfaceImpl::addLine(const DRW_Line& data){

	if(m_activeBlock != nullptr) return;

	VICUS::Drawing::Line newLine;
	newLine.m_zposition = drawing->m_zcounter;
	drawing->m_zcounter++;

	//create new line, insert into vector m_lines from drawing
	newLine.m_line = IBK::Line(data.basePoint.x, data.basePoint.y, data.secPoint.x, data.secPoint.y);
	newLine.m_lineWeight = DRW_LW_Conv::lineWidth2dxfInt(data.lWeight);
	newLine.m_layername = QString::fromStdString(data.layer);

	if(!(data.color == 256 || data.color == 7)) {
		newLine.m_color = QColor(DRW::dxfColors[data.color][0], DRW::dxfColors[data.color][1], DRW::dxfColors[data.color][2]);
	}else{
		newLine.m_color = QColor();
	}

	drawing->m_lines.push_back(newLine);
}

void DRW_InterfaceImpl::addRay(const DRW_Ray& /*data*/){}
void DRW_InterfaceImpl::addXline(const DRW_Xline& /*data*/){}
void DRW_InterfaceImpl::addArc(const DRW_Arc& data){

	if(m_activeBlock != nullptr) return;

	VICUS::Drawing::Arc newArc;
	newArc.m_zposition = drawing->m_zcounter;
	drawing->m_zcounter++;

	//create new arc, insert into vector m_arcs from drawing
	newArc.m_radius = data.radious;
	newArc.m_startAngle = data.staangle;
	newArc.m_endAngle = data.endangle;
	newArc.m_center = IBKMK::Vector2D(data.basePoint.x, data.basePoint.y);
	newArc.m_lineWeight = DRW_LW_Conv::lineWidth2dxfInt(data.lWeight);
	newArc.m_layername = QString::fromStdString(data.layer);

	if(!(data.color == 256 || data.color == 7)) {
		newArc.m_color = QColor(DRW::dxfColors[data.color][0], DRW::dxfColors[data.color][1], DRW::dxfColors[data.color][2]);
	}else{
		newArc.m_color = QColor();
	}

	drawing->m_arcs.push_back(newArc);
}


void DRW_InterfaceImpl::addCircle(const DRW_Circle& data){

	if(m_activeBlock != nullptr) return;

	VICUS::Drawing::Circle newCircle;
	newCircle.m_zposition = drawing->m_zcounter;
	drawing->m_zcounter++;

	newCircle.m_center = IBKMK::Vector2D(data.basePoint.x, data.basePoint.y);

	newCircle.m_radius = data.radious;
	newCircle.m_lineWeight = DRW_LW_Conv::lineWidth2dxfInt(data.lWeight);
	newCircle.m_layername = QString::fromStdString(data.layer);

	if(!(data.color == 256 || data.color == 7)) {
		newCircle.m_color = QColor(DRW::dxfColors[data.color][0], DRW::dxfColors[data.color][1], DRW::dxfColors[data.color][2]);
	}else{
		newCircle.m_color = QColor();
	}

	drawing->m_circles.push_back(newCircle);
}


void DRW_InterfaceImpl::addEllipse(const DRW_Ellipse& data){

	if(m_activeBlock != nullptr) return;

	VICUS::Drawing::Ellipse newEllipse;
	newEllipse.m_zposition = drawing->m_zcounter;
	drawing->m_zcounter++;

	newEllipse.m_center = IBKMK::Vector2D(data.basePoint.x, data.basePoint.y);
	newEllipse.m_majorAxis = IBKMK::Vector2D(data.secPoint.x, data.secPoint.y);
	newEllipse.m_ratio = data.ratio;
	newEllipse.m_startAngle = data.staparam;
	newEllipse.m_endAngle = data.endparam;
	newEllipse.m_lineWeight = DRW_LW_Conv::lineWidth2dxfInt(data.lWeight);
	newEllipse.m_layername = QString::fromStdString(data.layer);

	if(!(data.color == 256 || data.color == 7)) {
		newEllipse.m_color = QColor(DRW::dxfColors[data.color][0], DRW::dxfColors[data.color][1], DRW::dxfColors[data.color][2]);
	}else{
		newEllipse.m_color = QColor();
	}

	drawing->m_ellipses.push_back(newEllipse);
}


void DRW_InterfaceImpl::addLWPolyline(const DRW_LWPolyline& data){

	if(m_activeBlock != nullptr) return;

	VICUS::Drawing::PolyLine newpolyline;
	newpolyline.m_zposition = drawing->m_zcounter;
	drawing->m_zcounter++;

	newpolyline.m_polyline = std::vector<IBKMK::Vector2D>();
	newpolyline.m_lineWeight = DRW_LW_Conv::lineWidth2dxfInt(data.lWeight);

	// iterate over data.vertlist, insert all vertices of Polyline into vector
	for(size_t i = 0; i < data.vertlist.size(); i++){
		newpolyline.m_polyline.push_back(IBKMK::Vector2D(data.vertlist[i]->x, data.vertlist[i]->y));
	}

	newpolyline.m_layername = QString::fromStdString(data.layer);

	if(!(data.color == 256 || data.color == 7)) {
		newpolyline.m_color = QColor(DRW::dxfColors[data.color][0], DRW::dxfColors[data.color][1], DRW::dxfColors[data.color][2]);
	}else{
		newpolyline.m_color = QColor();
	}

	newpolyline.m_polyline_flag = data.flags;

	// insert vector into m_lines[data.layer] vector
	drawing->m_polylines.push_back(newpolyline);
}


void DRW_InterfaceImpl::addPolyline(const DRW_Polyline& data){

	if(m_activeBlock != nullptr) return;

	VICUS::Drawing::PolyLine newpolyline;
	newpolyline.m_zposition = drawing->m_zcounter;
	drawing->m_zcounter++;
	newpolyline.m_polyline = std::vector<IBKMK::Vector2D>();

	// iterateover data.vertlist, insert all vertices of Polyline into vector
	for(size_t i = 0; i < data.vertlist.size(); i++){
		newpolyline.m_polyline.push_back(IBKMK::Vector2D(data.vertlist[i]->basePoint.x, data.vertlist[i]->basePoint.y));
	}

	newpolyline.m_layername = QString::fromStdString(data.layer);
	newpolyline.m_lineWeight = DRW_LW_Conv::lineWidth2dxfInt(data.lWeight);

	if(!(data.color == 256 || data.color == 7)) {
		newpolyline.m_color = QColor(DRW::dxfColors[data.color][0], DRW::dxfColors[data.color][1], DRW::dxfColors[data.color][2]);
	}else{
		newpolyline.m_color = QColor();
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
void DRW_InterfaceImpl::addSolid(const DRW_Solid& data){

	if(m_activeBlock != nullptr) return;


	VICUS::Drawing::Solid newSolid;
	newSolid.m_zposition = drawing->m_zcounter;
	drawing->m_zcounter++;

	newSolid.m_point1 = IBKMK::Vector2D(data.basePoint.x, data.basePoint.y);
	newSolid.m_point2 = IBKMK::Vector2D(data.secPoint.x, data.secPoint.y);
	newSolid.m_point3 = IBKMK::Vector2D(data.thirdPoint.x, data.thirdPoint.y);
	newSolid.m_point4 = IBKMK::Vector2D(data.fourPoint.x, data.fourPoint.y);
	newSolid.m_lineWeight = DRW_LW_Conv::lineWidth2dxfInt(data.lWeight);
	newSolid.m_layername = QString::fromStdString(data.layer);

	if(!(data.color == 256 || data.color == 7)) {
		newSolid.m_color = QColor(DRW::dxfColors[data.color][0], DRW::dxfColors[data.color][1], DRW::dxfColors[data.color][2]);
	}else{
		newSolid.m_color = QColor();
	}

	drawing->m_solids.push_back(newSolid);

}
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

void SVImportDxfDialog::on_comboBoxUnit_activated(int index)
{
	m_ui->comboBoxUnit->setCurrentIndex(index);
}

