#include "SVImportDXFDialog.h"
#include "IBK_physics.h"
#include "SVConversions.h"
#include "ui_SVImportDXFDialog.h"

#include <libdxfrw.h>

#include "SVSettings.h"
#include "SVUndoAddDrawing.h"
#include "SVStyle.h"

#include "SVProjectHandler.h"
#include <VICUS_utilities.h>


SVImportDXFDialog::SVImportDXFDialog(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::SVImportDXFDialog)
{
	m_ui->setupUi(this);

	std::set<QString> existingNames;
	for (const VICUS::Drawing & b : project().m_drawings)
		existingNames.insert(b.m_displayName);
	QString defaultName = VICUS::uniqueName(tr("Drawing"), existingNames);

	m_ui->lineEditDrawingName->setText(defaultName);

	m_ui->comboBoxUnit->addItem("Meter", SU_Meter);
	m_ui->comboBoxUnit->addItem("Decimeter", SU_Decimeter);
	m_ui->comboBoxUnit->addItem("Centimeter", SU_Centimeter);
	m_ui->comboBoxUnit->addItem("Millimeter", SU_Millimeter);
}

SVImportDXFDialog::ImportResults SVImportDXFDialog::import(const QString &fname) {

	if (m_ui->lineEditDrawingName->text().trimmed().isEmpty()) {
		QMessageBox::critical(this, QString(), tr("Please enter a descriptive name!"));
		m_ui->lineEditDrawingName->selectAll();
		m_ui->lineEditDrawingName->setFocus();
	}

	m_ui->pushButtonImport->setEnabled(false);
	m_filePath = fname;

	m_ui->lineEditDrawingName->setText(fname);

	int res = exec();
	if (res == QDialog::Rejected)
		return ImportCancelled;

	return m_returnCode;
}

SVImportDXFDialog::~SVImportDXFDialog() {
	delete m_ui;
}


bool SVImportDXFDialog::readDxfFile(VICUS::Drawing &drawing, const QString &fname) {
	DRW_InterfaceImpl drwIntImpl(&drawing, m_nextId);
	dxfRW dxf(fname.toStdString().c_str());

	bool success = dxf.read(&drwIntImpl, false);
	return success;
}

void addPoints(IBKMK::Vector2D &center, const VICUS::Drawing::AbstractDrawingObject &object) {
	for (const IBKMK::Vector2D &v2D : object.m_points) {
		center += v2D;
	}
	center /= 1 + object.m_points.size();
}

void movePoints(const IBKMK::Vector2D &center, VICUS::Drawing::AbstractDrawingObject &object) {
	for (IBKMK::Vector2D &v2D : object.m_points) {
		v2D -= center;
	}
}

void SVImportDXFDialog::moveDrawings() {
	IBKMK::Vector2D center;

	for (const VICUS::Drawing::Point &d : m_drawing.m_points)
		addPoints(center, d);

	for (const VICUS::Drawing::Arc &d : m_drawing.m_arcs)
		addPoints(center, d);

	for (const VICUS::Drawing::Circle &d : m_drawing.m_circles)
		addPoints(center, d);

	for (const VICUS::Drawing::Ellipse &d : m_drawing.m_ellipses)
		addPoints(center, d);

	for (const VICUS::Drawing::Line &d : m_drawing.m_lines)
		addPoints(center, d);

	for (const VICUS::Drawing::PolyLine &d : m_drawing.m_polylines)
		addPoints(center, d);

	for (const VICUS::Drawing::Solid &d : m_drawing.m_solids)
		addPoints(center, d);

	IBKMK::Vector3D p(center.m_x,
					  center.m_y,
					  0.0); // we assume always in x-y axis

	// scale Vector with selected unit
	p *= m_drawing.m_scalingFactor;
	// rotation
	QVector3D vec = m_drawing.m_rotationMatrix.toQuaternion() * IBKVector2QVector(p);
	m_drawing.m_origin = - 1.0 * QVector2IBKVector(vec);
}

const VICUS::Drawing& SVImportDXFDialog::drawing() const {
	return m_drawing;
}

void SVImportDXFDialog::on_comboBoxUnit_activated(int index) {
	m_ui->comboBoxUnit->setCurrentIndex(index);
}

void SVImportDXFDialog::on_pushButtonConvert_clicked() {
	QString log;
	QFile fileName(m_filePath);
	if (!fileName.exists()) {
		throw QMessageBox::warning(this, tr("DXF Conversion"), tr("File %1 does not exist.").arg(fileName.fileName()));
		log += "File " + fileName.fileName() + " does not exist! Aborting Conversion.\n";
	}

	bool success;
	try {
		m_nextId = project().nextUnusedID(); // separate ID space
		m_drawing = VICUS::Drawing();
		m_drawing.m_id = m_nextId++;

		success = readDxfFile(m_drawing, fileName.fileName());
	} catch (IBK::Exception &ex) {
		log += "Error in converting DXF-File. See Error below\n";
		log += ex.what();
		return;
	}

	m_ui->pushButtonImport->setEnabled(success);
	if (success) {

		// set name for drawing from lineEdit
		m_drawing.m_displayName = m_ui->lineEditDrawingName->text();

		log += "Import successful!\nThe following objects were imported:\n";
		log += QString("Layers:\t%1\n").arg(m_drawing.m_layers.size());
		log += QString("Lines:\t%1\n").arg(m_drawing.m_lines.size());
		log += QString("Polylines:\t%1\n").arg(m_drawing.m_polylines.size());
		log += QString("Lines:\t%1\n").arg(m_drawing.m_lines.size());
		log += QString("Arcs:\t%1\n").arg(m_drawing.m_arcs.size());
		log += QString("Circles:\t%1\n").arg(m_drawing.m_circles.size());
		log += QString("Ellipses:\t%1\n").arg(m_drawing.m_ellipses.size());
		log += QString("Points:\t%1\n").arg(m_drawing.m_points.size());

		// m_drawing.updatePointer();
		m_drawing.updateParents();

		ScaleUnit su = (ScaleUnit)m_ui->comboBoxUnit->currentData().toInt();

		switch (su) {

		case SU_Meter:		m_drawing.m_scalingFactor = 1;		break;
		case SU_Decimeter:	m_drawing.m_scalingFactor = 0.1;	break;
		case SU_Centimeter: m_drawing.m_scalingFactor = 0.01;	break;
		case SU_Millimeter: m_drawing.m_scalingFactor = 0.001;	break;

		case NUM_SU: break; // make compiler happy

		}

		if (m_ui->checkBoxMove)
			moveDrawings();
	}
	else
		log += "Import of DXF-File was not successful!";

	m_ui->plainTextEditLogWindow->setPlainText(log);
}


void SVImportDXFDialog::on_pushButtonImport_clicked() {
	m_returnCode = AddDrawings;
	accept();
}


DRW_InterfaceImpl::DRW_InterfaceImpl(VICUS::Drawing *drawing, unsigned int &nextId) :
	m_drawing(drawing),
	m_nextId(&nextId)
{}


void DRW_InterfaceImpl::addHeader(const DRW_Header* /*data*/){}
void DRW_InterfaceImpl::addLType(const DRW_LType& /*data*/){}
void DRW_InterfaceImpl::addLayer(const DRW_Layer& data){

	// If nullptr return
	if(m_activeBlock != nullptr)
		return;

	// initialise struct Layer and populate the attributes
	VICUS::Drawing::DrawingLayer newLayer;

	// Set ID
	newLayer.m_id = (*m_nextId)++;

	// name of layer
	newLayer.m_displayName = QString::fromStdString(data.name);

	// read linewidth from dxf file, convert to double using lineWidth2dxfInt from libdxfrw
	newLayer.m_lineWeight = DRW_LW_Conv::lineWidth2dxfInt(data.lWeight);

	/* value 256 means use defaultColor, value 7 is black */
	if (data.color != 256 && data.color != 7)
		newLayer.m_color = QColor(DRW::dxfColors[data.color][0], DRW::dxfColors[data.color][1], DRW::dxfColors[data.color][2]);
	else
		newLayer.m_color = SVStyle::instance().m_defaultDrawingColor;

	// Push new layer into vector<Layer*> m_layer
	m_drawing->m_layers.push_back(newLayer);
}


void DRW_InterfaceImpl::addDimStyle(const DRW_Dimstyle& /*data*/){}
void DRW_InterfaceImpl::addVport(const DRW_Vport& /*data*/){}
void DRW_InterfaceImpl::addTextStyle(const DRW_Textstyle& /*data*/){}
void DRW_InterfaceImpl::addAppId(const DRW_AppId& /*data*/){}
void DRW_InterfaceImpl::addBlock(const DRW_Block& data){

	// New Block
	VICUS::Drawing::Block newBlock;

	// Set name
	newBlock.m_name = QString::fromStdString(data.name);

	// Set color
	newBlock.m_color = QColor();

	// Set line weight
	newBlock.m_lineWeight = 0;

	// Add blocks
	m_drawing->m_blocks.push_back(newBlock);

	// Set actove block
	m_activeBlock = &newBlock;

}
void DRW_InterfaceImpl::setBlock(const int /*handle*/){}
void DRW_InterfaceImpl::endBlock(){
	// Active block not existing
	m_activeBlock = nullptr;

}
void DRW_InterfaceImpl::addPoint(const DRW_Point& data){

	// Activeblock
	if(m_activeBlock != nullptr)
		return;

	VICUS::Drawing::Point newPoint;
	newPoint.m_zPosition = m_drawing->m_zCounter;
	m_drawing->m_zCounter++;

	//create new point, insert into vector m_points from drawing
	newPoint.m_point = IBKMK::Vector2D(data.basePoint.x, data.basePoint.y);
	newPoint.m_lineWeight = DRW_LW_Conv::lineWidth2dxfInt(data.lWeight);
	newPoint.m_layerName = QString::fromStdString(data.layer);

	newPoint.m_id = (*m_nextId)++;

	/* value 256 means use defaultColor, value 7 is black */
	if(!(data.color == 256 || data.color == 7))
		newPoint.m_color = QColor(DRW::dxfColors[data.color][0], DRW::dxfColors[data.color][1], DRW::dxfColors[data.color][2]);
	else
		newPoint.m_color = QColor();

	newPoint.m_points.push_back(newPoint.m_point);

	m_drawing->m_points.push_back(newPoint);

}
void DRW_InterfaceImpl::addLine(const DRW_Line& data){

	if(m_activeBlock != nullptr) return;

	VICUS::Drawing::Line newLine;
	newLine.m_zPosition = m_drawing->m_zCounter;
	m_drawing->m_zCounter++;

	//create new line, insert into vector m_lines from drawing
	newLine.m_line = IBK::Line(data.basePoint.x, data.basePoint.y, data.secPoint.x, data.secPoint.y);
	newLine.m_lineWeight = DRW_LW_Conv::lineWidth2dxfInt(data.lWeight);
	newLine.m_layerName = QString::fromStdString(data.layer);

	newLine.m_id = (*m_nextId)++;

	/* value 256 means use defaultColor, value 7 is black */
	/* value 256 means use defaultColor, value 7 is black */
	if(!(data.color == 256 || data.color == 7))
		newLine.m_color = QColor(DRW::dxfColors[data.color][0], DRW::dxfColors[data.color][1], DRW::dxfColors[data.color][2]);
	else
		newLine.m_color = QColor();

	newLine.m_points.push_back(newLine.m_line.m_p1);
	newLine.m_points.push_back(newLine.m_line.m_p2);

	m_drawing->m_lines.push_back(newLine);
}

void DRW_InterfaceImpl::addRay(const DRW_Ray& /*data*/){}
void DRW_InterfaceImpl::addXline(const DRW_Xline& /*data*/){}
void DRW_InterfaceImpl::addArc(const DRW_Arc& data){

	if(m_activeBlock != nullptr) return;

	VICUS::Drawing::Arc newArc;
	newArc.m_zPosition = m_drawing->m_zCounter;
	m_drawing->m_zCounter++;

	//create new arc, insert into vector m_arcs from drawing
	newArc.m_radius = data.radious;
	newArc.m_startAngle = data.staangle;
	newArc.m_endAngle = data.endangle;
	newArc.m_center = IBKMK::Vector2D(data.basePoint.x, data.basePoint.y);
	newArc.m_lineWeight = DRW_LW_Conv::lineWidth2dxfInt(data.lWeight);
	newArc.m_layerName = QString::fromStdString(data.layer);

	newArc.m_id = (*m_nextId)++;

	/* value 256 means use defaultColor, value 7 is black */
	if(!(data.color == 256 || data.color == 7))
		newArc.m_color = QColor(DRW::dxfColors[data.color][0], DRW::dxfColors[data.color][1], DRW::dxfColors[data.color][2]);
	else
		newArc.m_color = QColor();

	newArc.m_points.resize(VICUS::SEGMENT_COUNT_ARC);

	double startAngle = newArc.m_startAngle;
	double endAngle = newArc.m_endAngle;

	double angleDifference;

	if (startAngle > endAngle)
		angleDifference = 2 * IBK::PI - startAngle + endAngle;
	else
		angleDifference = endAngle - startAngle;

	unsigned int toCalcN = (int)(VICUS::SEGMENT_COUNT_ARC * (2 * IBK::PI / angleDifference));
	double stepAngle = angleDifference / toCalcN;

	for (unsigned int i = 0; i < toCalcN; i++){
		newArc.m_points[i] = IBKMK::Vector2D(newArc.m_center.m_x + newArc.m_radius * cos(startAngle + i * stepAngle),
											 newArc.m_center.m_y + newArc.m_radius * sin(startAngle + i * stepAngle));
	}

	m_drawing->m_arcs.push_back(newArc);
}


void DRW_InterfaceImpl::addCircle(const DRW_Circle& data){

	if(m_activeBlock != nullptr) return;

	VICUS::Drawing::Circle newCircle;
	newCircle.m_zPosition = m_drawing->m_zCounter;
	m_drawing->m_zCounter++;

	newCircle.m_center = IBKMK::Vector2D(data.basePoint.x, data.basePoint.y);

	newCircle.m_radius = data.radious;
	newCircle.m_lineWeight = DRW_LW_Conv::lineWidth2dxfInt(data.lWeight);
	newCircle.m_layerName = QString::fromStdString(data.layer);

	newCircle.m_id = (*m_nextId)++;

	/* value 256 means use defaultColor, value 7 is black */
	if(!(data.color == 256 || data.color == 7))
		newCircle.m_color = QColor(DRW::dxfColors[data.color][0], DRW::dxfColors[data.color][1], DRW::dxfColors[data.color][2]);
	else
		newCircle.m_color = QColor();

	newCircle.m_points.resize(VICUS::SEGMENT_COUNT_CIRCLE);

	for(unsigned int i = 0; i < VICUS::SEGMENT_COUNT_CIRCLE; i++){
		newCircle.m_points[i] =IBKMK::Vector2D(newCircle.m_center.m_x + newCircle.m_radius * cos(2 * IBK::PI * i / VICUS::SEGMENT_COUNT_CIRCLE),
											   newCircle.m_center.m_y + newCircle.m_radius * sin(2 * IBK::PI * i / VICUS::SEGMENT_COUNT_CIRCLE));
	}

	m_drawing->m_circles.push_back(newCircle);
}


void DRW_InterfaceImpl::addEllipse(const DRW_Ellipse& data){

	if(m_activeBlock != nullptr) return;

	VICUS::Drawing::Ellipse newEllipse;
	newEllipse.m_zPosition = m_drawing->m_zCounter;
	m_drawing->m_zCounter++;

	newEllipse.m_center = IBKMK::Vector2D(data.basePoint.x, data.basePoint.y);
	newEllipse.m_majorAxis = IBKMK::Vector2D(data.secPoint.x, data.secPoint.y);
	newEllipse.m_ratio = data.ratio;
	newEllipse.m_startAngle = data.staparam;
	newEllipse.m_endAngle = data.endparam;
	newEllipse.m_lineWeight = DRW_LW_Conv::lineWidth2dxfInt(data.lWeight);
	newEllipse.m_layerName = QString::fromStdString(data.layer);

	newEllipse.m_id = (*m_nextId)++;

	double startAngle = newEllipse.m_startAngle;
	double endAngle = newEllipse.m_endAngle;

	double angleStep = (endAngle - startAngle) / VICUS::SEGMENT_COUNT_ELLIPSE;

	double majorRadius = sqrt(pow(newEllipse.m_majorAxis.m_x, 2) + pow(newEllipse.m_majorAxis.m_y, 2));
	double minorRadius = majorRadius * newEllipse.m_ratio;

	double rotationAngle = atan2(newEllipse.m_majorAxis.m_y, newEllipse.m_majorAxis.m_x);

	double x, y, rotated_x, rotated_y;

	newEllipse.m_points.resize(VICUS::SEGMENT_COUNT_ELLIPSE);

	for (unsigned int i = 0; i <= VICUS::SEGMENT_COUNT_ELLIPSE; i++) {

		double currentAngle = startAngle + i * angleStep;

		x = majorRadius * cos(currentAngle);
		y = minorRadius * sin(currentAngle);

		rotated_x = x * cos(rotationAngle) - y * sin(rotationAngle);
		rotated_y = x * sin(rotationAngle) + y * cos(rotationAngle);

		newEllipse.m_points[i] = IBKMK::Vector2D(rotated_x + newEllipse.m_center.m_x,
												 rotated_y + newEllipse.m_center.m_y);

	}

	/* value 256 means use defaultColor, value 7 is black */
	if(!(data.color == 256 || data.color == 7))
		newEllipse.m_color = QColor(DRW::dxfColors[data.color][0], DRW::dxfColors[data.color][1], DRW::dxfColors[data.color][2]);
	else
		newEllipse.m_color = QColor();

	m_drawing->m_ellipses.push_back(newEllipse);
}


void DRW_InterfaceImpl::addLWPolyline(const DRW_LWPolyline& data){

	if(m_activeBlock != nullptr) return;

	VICUS::Drawing::PolyLine newPolyline;
	newPolyline.m_zPosition = m_drawing->m_zCounter;
	m_drawing->m_zCounter++;

	newPolyline.m_polyline = std::vector<IBKMK::Vector2D>();
	newPolyline.m_lineWeight = DRW_LW_Conv::lineWidth2dxfInt(data.lWeight);

	newPolyline.m_id = (*m_nextId)++;

	// iterate over data.vertlist, insert all vertices of Polyline into vector
	for(size_t i = 0; i < data.vertlist.size(); i++){
		IBKMK::Vector2D point(data.vertlist[i]->x, data.vertlist[i]->y);
		newPolyline.m_points.push_back(point);
		newPolyline.m_polyline.push_back(point);
	}

	newPolyline.m_layerName = QString::fromStdString(data.layer);

	/* value 256 means use defaultColor, value 7 is black */
	if(!(data.color == 256 || data.color == 7))
		newPolyline.m_color = QColor(DRW::dxfColors[data.color][0], DRW::dxfColors[data.color][1], DRW::dxfColors[data.color][2]);
	else
		newPolyline.m_color = QColor();


	newPolyline.m_endConnected = data.flags == 1;

	// insert vector into m_lines[data.layer] vector
	m_drawing->m_polylines.push_back(newPolyline);
}


void DRW_InterfaceImpl::addPolyline(const DRW_Polyline& data){

	if(m_activeBlock != nullptr) return;

	VICUS::Drawing::PolyLine newPolyline;
	newPolyline.m_zPosition = m_drawing->m_zCounter;
	m_drawing->m_zCounter++;
	newPolyline.m_polyline = std::vector<IBKMK::Vector2D>();

	// iterateover data.vertlist, insert all vertices of Polyline into vector
	for(size_t i = 0; i < data.vertlist.size(); i++){
		IBKMK::Vector2D point(data.vertlist[i]->basePoint.x, data.vertlist[i]->basePoint.y);
		newPolyline.m_points.push_back(point);
		newPolyline.m_polyline.push_back(point);
	}

	newPolyline.m_id = (*m_nextId)++;
	newPolyline.m_layerName = QString::fromStdString(data.layer);
	newPolyline.m_lineWeight = DRW_LW_Conv::lineWidth2dxfInt(data.lWeight);

	/* value 256 means use defaultColor, value 7 is black */
	if(!(data.color == 256 || data.color == 7))
		newPolyline.m_color = QColor(DRW::dxfColors[data.color][0], DRW::dxfColors[data.color][1], DRW::dxfColors[data.color][2]);
	else
		newPolyline.m_color = QColor();


	newPolyline.m_endConnected = data.flags == 1;

	// insert vector into m_lines[data.layer] vector
	m_drawing->m_polylines.push_back(newPolyline);
}
void DRW_InterfaceImpl::addSpline(const DRW_Spline* /*data*/){}
void DRW_InterfaceImpl::addKnot(const DRW_Entity & /*data*/){}
void DRW_InterfaceImpl::addInsert(const DRW_Insert& /*data*/){}
void DRW_InterfaceImpl::addTrace(const DRW_Trace& /*data*/){}
void DRW_InterfaceImpl::add3dFace(const DRW_3Dface& /*data*/){}
void DRW_InterfaceImpl::addSolid(const DRW_Solid& data){

	if(m_activeBlock != nullptr) return;


	VICUS::Drawing::Solid newSolid;
	newSolid.m_zPosition = m_drawing->m_zCounter;
	m_drawing->m_zCounter++;

	newSolid.m_point1 = IBKMK::Vector2D(data.basePoint.x, data.basePoint.y);
	newSolid.m_point2 = IBKMK::Vector2D(data.secPoint.x, data.secPoint.y);
	newSolid.m_point3 = IBKMK::Vector2D(data.thirdPoint.x, data.thirdPoint.y);
	newSolid.m_point4 = IBKMK::Vector2D(data.fourPoint.x, data.fourPoint.y);
	newSolid.m_lineWeight = DRW_LW_Conv::lineWidth2dxfInt(data.lWeight);
	newSolid.m_layerName = QString::fromStdString(data.layer);
	newSolid.m_id = (*m_nextId)++;

	/* value 256 means use defaultColor, value 7 is black */
	if(!(data.color == 256 || data.color == 7))
		newSolid.m_color = QColor(DRW::dxfColors[data.color][0], DRW::dxfColors[data.color][1], DRW::dxfColors[data.color][2]);
	else
		newSolid.m_color = QColor();

	newSolid.m_points.resize(4);
	newSolid.m_points.push_back(newSolid.m_point1);
	newSolid.m_points.push_back(newSolid.m_point2);
	newSolid.m_points.push_back(newSolid.m_point3);
	newSolid.m_points.push_back(newSolid.m_point4);

	m_drawing->m_solids.push_back(newSolid);

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

