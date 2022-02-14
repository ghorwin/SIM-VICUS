/*	SIM-VICUS - Building and District Energy Simulation Tool.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Dirk Weiss  <dirk.weiss -[at]- tu-dresden.de>
	  Stephan Hirth  <stephan.hirth -[at]- tu-dresden.de>
	  Hauke Hirsch  <hauke.hirsch -[at]- tu-dresden.de>

	  ... all the others from the SIM-VICUS team ... :-)

	This program is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

#include "SVPropEditGeometry.h"
#include "ui_SVPropEditGeometry.h"

#include <IBK_physics.h>

#include <IBKMK_3DCalculations.h>
#include <IBKMK_Quaternion.h>

#include <QtExt_ValidatingLineEdit.h>
#include <QtExt_ValidatingInputBase.h>

#include <VICUS_Project.h>
#include <VICUS_Object.h>
#include <VICUS_BuildingLevel.h>
#include <VICUS_Polygon3D.h>
#include <VICUS_utilities.h>

#include <QMessageBox>

#include <SV_Conversions.h>

#include "SVViewStateHandler.h"
#include "SVProjectHandler.h"
#include "SVUndoModifySurfaceGeometry.h"
#include "SVUndoAddSurface.h"
#include "SVUndoAddZone.h"
#include "SVUndoCopyZones.h"
#include "SVUndoCopySurfaces.h"
#include "SVUndoCopySubSurfaces.h"
#include "SVPropVertexListWidget.h"
#include "SVGeometryView.h"
#include "SVPropAddWindowWidget.h"
#include "SVLocalCoordinateView.h"

#include "Vic3DNewGeometryObject.h"
#include "Vic3DCoordinateSystemObject.h"
#include "Vic3DWireFrameObject.h"
#include "Vic3DTransform3D.h"
#include "Vic3DSceneView.h"

#include <QLocale>
#include <QWheelEvent>

/*! helper function to compare two IBKMK vectors */
template <int digits>
bool checkVectors(const IBKMK::Vector3D &v1, const IBKMK::Vector3D &v2 ) {
	return ( IBK::nearly_equal<digits>(v1.m_x, v2.m_x) &&
			 IBK::nearly_equal<digits>(v1.m_y, v2.m_y) &&
			 IBK::nearly_equal<digits>(v1.m_z, v2.m_z) );
}



class LineEditFormater : public QtExt::FormatterBase {
public:
	~LineEditFormater() override;
	QString formatted(double value) override {
		return QString("%L1").arg(value, 0, 'f', 3);
	}
};

// dummy destructor needed to tell compiler to place virtual function table in this object file
LineEditFormater::~LineEditFormater() {}


// *** Widget implementation ***

SVPropEditGeometry::SVPropEditGeometry(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVPropEditGeometry)
{
	m_ui->setupUi(this);
	m_ui->verticalLayoutMaster->setMargin(0);
//	m_ui->verticalLayoutPage1->setMargin(0);
//	m_ui->verticalLayoutPage2->setMargin(0);

	SVViewStateHandler::instance().m_propEditGeometryWidget = this;

	connect(&SVProjectHandler::instance(), &SVProjectHandler::modified,
			this, &SVPropEditGeometry::onModified);

	connect(&SVViewStateHandler::instance(), &SVViewStateHandler::viewStateChanged,
			this, &SVPropEditGeometry::onViewStateChanged);

	m_ui->lineEditX->setup(-1E5,1E5,tr("X Value"),true, true);
	m_ui->lineEditY->setup(-1E5,1E5,tr("Y Value"),true, true);
	m_ui->lineEditZ->setup(-1E5,1E5,tr("Z Value"),true, true);

	m_ui->lineEditX->setFormatter(new LineEditFormater);
	m_ui->lineEditY->setFormatter(new LineEditFormater);
	m_ui->lineEditZ->setFormatter(new LineEditFormater);
	m_ui->lineEditInclination->setFormatter(new LineEditFormater);
	m_ui->lineEditOrientation->setFormatter(new LineEditFormater);

	m_ui->lineEditXCopy->setFormatter(new LineEditFormater);
	m_ui->lineEditYCopy->setFormatter(new LineEditFormater);
	m_ui->lineEditZCopy->setFormatter(new LineEditFormater);

	m_ui->lineEditX->installEventFilter(this);
	m_ui->lineEditY->installEventFilter(this);
	m_ui->lineEditZ->installEventFilter(this);
	m_ui->lineEditInclination->installEventFilter(this);
	m_ui->lineEditOrientation->installEventFilter(this);

	m_ui->lineEditXCopy->installEventFilter(this);
	m_ui->lineEditYCopy->installEventFilter(this);
	m_ui->lineEditZCopy->installEventFilter(this);

	m_modificationState[MT_Translate] = MS_Absolute;
	m_modificationState[MT_Rotate] = MS_Absolute;
	m_modificationState[MT_Scale] = MS_Absolute;

	// set initial states
	setState(MT_Translate, MS_Absolute);

	initializeCopy();

	m_ui->widgetXYZ->layout()->setMargin(0);
	m_ui->widgetRota->layout()->setMargin(0);

	setCurrentPage(O_AddGeometry);
	// we set the local coordinate system object
	m_cso = SVViewStateHandler::instance().m_coordinateSystemObject;

}


SVPropEditGeometry::~SVPropEditGeometry() {
	delete m_ui;
}


void SVPropEditGeometry::setCurrentPage(const SVPropEditGeometry::Operation & op) {
	switch (op) {
		case O_AddGeometry :
			m_ui->pushButtonAdd->setChecked(true);
			m_ui->pushButtonEdit->setChecked(false);
			m_ui->stackedWidget->setCurrentIndex(0);
		break;
		case O_EditGeometry :
			m_ui->pushButtonEdit->setChecked(true);
			m_ui->pushButtonAdd->setChecked(false);
			m_ui->stackedWidget->setCurrentIndex(1);
		break;
	}
}

void SVPropEditGeometry::setCoordinates(const Vic3D::Transform3D &t) {
	// is being called from local coordinate system object, whenever this has changed location (regardless of
	// its own visibility)
	m_localCoordinatePosition =  t;

	// compute dimensions of bounding box (dx, dy, dz) and center point of all selected surfaces
	m_bbDim[OM_Local] = project().boundingBox(m_selSurfaces, m_selSubSurfaces, m_bbCenter[OM_Local],
												  QVector2IBKVector(m_cso->translation() ),
												  QVector2IBKVector(m_cso->localXAxis() ),
												  QVector2IBKVector(m_cso->localYAxis() ),
												  QVector2IBKVector(m_cso->localZAxis() ) );
	m_bbDim[OM_Global] = project().boundingBox(m_selSurfaces, m_selSubSurfaces, m_bbCenter[OM_Global]);

	updateInputs();
}

void SVPropEditGeometry::setRotation(const IBKMK::Vector3D &normal) {
	// TODO nochmal nachdenken
	m_normal = normal.normalized();

	m_ui->lineEditInclination->setText( QString("%L1").arg(std::acos(normal.m_z)/IBK::DEG2RAD, 0, 'f', 3) );
	// positive y Richtung = Norden = Orientation 0°
	// positive x Richtung = Osten = Orientation 90°

	double orientation = std::atan2(normal.m_x, ( normal.m_y == 0. ? 1E-8 : normal.m_y ) ) /IBK::DEG2RAD ;
	m_ui->lineEditOrientation->setText( QString("%L1").arg(orientation < 0 ? ( orientation + 360 ) : orientation, 0, 'f', 3 ) );
}

void SVPropEditGeometry::onModified(int modificationType, ModificationInfo * ) {
	SVProjectHandler::ModificationTypes modType((SVProjectHandler::ModificationTypes)modificationType);
	switch (modType) {
		case SVProjectHandler::AllModified:
		case SVProjectHandler::BuildingGeometryChanged:
		case SVProjectHandler::NodeStateModified:
			// When the building geometry has changed, we need to update the geometrical info
			// in the widget based on the current selection.

			// Also, if the selection has changed, we need to distinguish between no selection and selection
			// and update the buttons accordingly.
			updateUi();
		break;

		default: ; // just to make compiler happy
	}
}


void SVPropEditGeometry::onViewStateChanged() {
	const SVViewState & vs = SVViewStateHandler::instance().viewState();
	if (vs.m_sceneOperationMode == SVViewState::NUM_OM) {
		// setCurrentPage(O_AddGeometry);
		// m_ui->pushButtonEdit->setEnabled(false);
		// clear current selection transformation matrix
		m_ui->widgetEdit->setEnabled(false);
		SVViewStateHandler::instance().m_selectedGeometryObject->m_transform = Vic3D::Transform3D();
	}
	else {
		m_ui->widgetEdit->setEnabled(true);
		//m_ui->pushButtonEdit->setEnabled(true);
		updateCoordinateSystemLook();
	}
}

// *** slots ***

void SVPropEditGeometry::on_pushButtonAddPolygon_clicked() {
	// reset new polygon object and set it into polygon mode
	SVViewStateHandler::instance().m_newGeometryObject->startNewGeometry(Vic3D::NewGeometryObject::NGM_Polygon);
	// signal, that we want to start adding a new polygon
	SVViewState vs = SVViewStateHandler::instance().viewState();
	vs.m_sceneOperationMode = SVViewState::OM_PlaceVertex;
	vs.m_propertyWidgetMode = SVViewState::PM_VertexList;
	// now tell all UI components to toggle their view state
	SVViewStateHandler::instance().setViewState(vs);
	// clear vertex list in property widget
	SVViewStateHandler::instance().m_propVertexListWidget->setup(Vic3D::NewGeometryObject::NGM_Polygon);
	SVViewStateHandler::instance().m_geometryView->focusSceneView();
}

void SVPropEditGeometry::on_pushButtonAddRect_clicked() {
	// reset new polygon object and set it into rect mode
	SVViewStateHandler::instance().m_newGeometryObject->startNewGeometry(Vic3D::NewGeometryObject::NGM_Rect);
	// signal, that we want to start adding a new polygon
	SVViewState vs = SVViewStateHandler::instance().viewState();
	vs.m_sceneOperationMode = SVViewState::OM_PlaceVertex;
	vs.m_propertyWidgetMode = SVViewState::PM_VertexList;
	// now tell all UI components to toggle their view state
	SVViewStateHandler::instance().setViewState(vs);
	// clear vertex list in property widget
	SVViewStateHandler::instance().m_propVertexListWidget->setup(Vic3D::NewGeometryObject::NGM_Rect);
	SVViewStateHandler::instance().m_geometryView->focusSceneView();
}

void SVPropEditGeometry::on_pushButtonAddZone_clicked() {
	// signal, that we want to start adding a new polygon
	SVViewState vs = SVViewStateHandler::instance().viewState();
	vs.m_sceneOperationMode = SVViewState::OM_PlaceVertex;
	vs.m_propertyWidgetMode = SVViewState::PM_VertexList;
	// now tell all UI components to toggle their view state
	SVViewStateHandler::instance().setViewState(vs);
	// clear vertex list in property widget
	SVViewStateHandler::instance().m_propVertexListWidget->setup(Vic3D::NewGeometryObject::NGM_Zone);
	SVViewStateHandler::instance().m_geometryView->focusSceneView();
}

void SVPropEditGeometry::on_pushButtonAddRoof_clicked() {
	// signal, that we want to start adding a new polygon
	SVViewState vs = SVViewStateHandler::instance().viewState();
	vs.m_sceneOperationMode = SVViewState::OM_PlaceVertex;
	vs.m_propertyWidgetMode = SVViewState::PM_VertexList;
	// now tell all UI components to toggle their view state
	SVViewStateHandler::instance().setViewState(vs);
	// clear vertex list in property widget
	SVViewStateHandler::instance().m_propVertexListWidget->setup(Vic3D::NewGeometryObject::NGM_Roof);
	SVViewStateHandler::instance().m_geometryView->focusSceneView();
}

void SVPropEditGeometry::on_pushButtonAddWindow_clicked() {
	// set property widget into "add window/door" mode
	SVViewState vs = SVViewStateHandler::instance().viewState();
	vs.m_propertyWidgetMode = SVViewState::PM_AddSubSurfaceGeometry;
	SVViewStateHandler::instance().setViewState(vs);
	// clear vertex list in property widget
	SVViewStateHandler::instance().m_propAddWindowWidget->setup();
}


void SVPropEditGeometry::on_lineEditX_editingFinished() {
	QLineEdit * lineEdit = qobject_cast<QLineEdit *>( QObject::sender() );

	// Ignore undesirable signals.
	if ( lineEdit->isModified() )
		return;
	lineEdit->setModified( false );

	on_lineEditX_returnPressed();
}

void SVPropEditGeometry::on_lineEditY_editingFinished() {
	QLineEdit * lineEdit = qobject_cast<QLineEdit *>( QObject::sender() );

	// Ignore undesirable signals.
	if ( lineEdit->isModified() )
		return;
	lineEdit->setModified( false );

	on_lineEditY_returnPressed();
}

void SVPropEditGeometry::on_lineEditZ_editingFinished() {
	QLineEdit * lineEdit = qobject_cast<QLineEdit *>( QObject::sender() );

	// Ignore undesirable signals.
	if ( lineEdit->isModified() )
		return;
	lineEdit->setModified( false );

	on_lineEditZ_returnPressed();
}


void SVPropEditGeometry::translate() {
	// get translation vector from selected geometry object
	IBKMK::Vector3D translation = QVector2IBKVector(SVViewStateHandler::instance().m_selectedGeometryObject->m_transform.translation());

	if (translation == IBKMK::Vector3D())
		return;

	// compose vector of modified surface geometries
	std::vector<VICUS::Surface>			modifiedSurfaces;
	std::set<VICUS::Surface*>			handledSurfaces;

	// process all selected objects
	for (const VICUS::Object * o : SVViewStateHandler::instance().m_selectedGeometryObject->m_selectedObjects) {
		// handle surfaces
		const VICUS::Surface * s = dynamic_cast<const VICUS::Surface *>(o);
		if (s != nullptr) {
			std::vector<IBKMK::Vector3D> vertexes = s->polygon3D().vertexes();
			for ( IBKMK::Vector3D & v : vertexes ) {
				// use just this instead of making a QVetor3D
				v += translation;
			}
			VICUS::Surface modS(*s);
			modS.setPolygon3D( VICUS::Polygon3D(vertexes) );

			if(!modS.geometry().isValid())
				modS.healGeometry(vertexes);

			modifiedSurfaces.push_back(modS);
		}
		// TODO : Netzwerk zeugs

		// handle only selected sub surfaces where parent is not selected
		const VICUS::SubSurface * ss = dynamic_cast<const VICUS::SubSurface *>(o);
		if (ss != nullptr) {

			// we keep our original surface
			VICUS::Surface *parentSurf = dynamic_cast<VICUS::Surface*>(ss->m_parent);
			if (parentSurf != nullptr && parentSurf->m_selected && parentSurf->m_visible)
				continue; //

			if(handledSurfaces.find(parentSurf) != handledSurfaces.end())
				continue; // surface already handled

			// parentSurf->m_selected = true; // !!!! only for now till we adjust the function !!!
			VICUS::Surface modS(*parentSurf);

			// we cache our poldon data
			IBKMK::Vector3D offset3d = modS.geometry().offset();
			const IBKMK::Vector3D &localX = modS.geometry().localX();
			const IBKMK::Vector3D &localY = modS.geometry().localY();

			std::vector<VICUS::SubSurface> newSubSurfs(modS.subSurfaces().size() );
			// now we also have to scale the sub surfaces
			for ( unsigned int i = 0; i<modS.subSurfaces().size(); ++i ) {

				Q_ASSERT(modS.subSurfaces().size() == modS.geometry().holes().size());
				newSubSurfs[i] = modS.subSurfaces()[i];

				if (!modS.subSurfaces()[i].m_selected || !modS.subSurfaces()[i].m_visible)
					continue;

				// we only modify our selected sub surface
				VICUS::SubSurface &subS = const_cast<VICUS::SubSurface &>(modS.subSurfaces()[i]);
				std::vector<IBKMK::Vector2D> newSubSurfVertexes(subS.m_polygon2D.vertexes().size());


				for ( unsigned int j=0; j<subS.m_polygon2D.vertexes().size(); ++j ) {
						IBKMK::Vector2D v2D = subS.m_polygon2D.vertexes()[j];

						// we now calculate the 3D points of the sub surface
						// afterwards we scale up the surface
						IBKMK::Vector3D v3D = offset3d + localX * v2D.m_x + localY * v2D.m_y;

						Vic3D::Transform3D t;
						t.translate(IBKVector2QVector(translation) );
						t.setTranslation(t.toMatrix()*IBKVector2QVector(v3D) );
						v3D = QVector2IBKVector(t.translation() );

						// and we calculate back the projection on the plane
						// we have to take the offset of our new scaled polygon
						IBKMK::planeCoordinates(modS.geometry().offset(), localX, localY, v3D, newSubSurfVertexes[j].m_x, newSubSurfVertexes[j].m_y);

				}

				newSubSurfs[i].m_polygon2D = newSubSurfVertexes;
			}
			// we cache that we already handled all selected sub surfaces of the surface
			handledSurfaces.insert(parentSurf);

			// we update the 2D polyline
			modS.setSubSurfaces(newSubSurfs);


			modifiedSurfaces.push_back(modS);
		}
	}

	// in case operation was executed without any selected objects - should be prevented
	if (modifiedSurfaces.empty())
		return;

	SVUndoModifySurfaceGeometry * undoSurf = new SVUndoModifySurfaceGeometry(tr("Translated surface geometry"), modifiedSurfaces );
	undoSurf->push();

	// reset local transformation matrix
	SVViewStateHandler::instance().m_selectedGeometryObject->m_transform = Vic3D::Transform3D();
}

void SVPropEditGeometry::scale() {
	// we now apply the already specified transformation
	// get translation and scale vector from selected geometry object
	IBKMK::Vector3D scale = QVector2IBKVector(SVViewStateHandler::instance().m_selectedGeometryObject->m_transform.scale());
	IBKMK::Vector3D trans = QVector2IBKVector(SVViewStateHandler::instance().m_selectedGeometryObject->m_transform.translation());

	if (scale == IBKMK::Vector3D() || scale == IBKMK::Vector3D(1,1,1) )
		return;
	QVector3D transLCSO = m_cso->translation();

	// compose vector of modified surface geometries
	std::vector<VICUS::Surface>			modifiedSurfaces;
	std::set<const VICUS::Surface*>		handledSurfaces;

	// process all selected objects
	for (const VICUS::Object * o : SVViewStateHandler::instance().m_selectedGeometryObject->m_selectedObjects) {
		// handle surfaces
		const VICUS::Surface * s = dynamic_cast<const VICUS::Surface *>(o);
		if (s != nullptr) {
			std::vector<IBKMK::Vector3D> vertexes = s->polygon3D().vertexes();
			for ( IBKMK::Vector3D & v : vertexes ) {
				// use just this instead of making a QVetor3D
				Vic3D::Transform3D t;
				t.scale(IBKVector2QVector(scale) );
				t.translate(IBKVector2QVector(trans) );
				t.setTranslation(t.toMatrix()*IBKVector2QVector(v) );
				v = QVector2IBKVector(t.translation());
			}
			VICUS::Surface modS(*s);

			modS.setPolygon3D( VICUS::Polygon3D(vertexes) );

			if(!modS.geometry().isValid())
				modS.healGeometry(vertexes);

			// we cache our poldon data
			IBKMK::Vector3D offset3dOld = s->geometry().offset();
			IBKMK::Vector3D offset3dNew = modS.geometry().offset();

			const IBKMK::Vector3D &localXOld = s->geometry().localX();
			const IBKMK::Vector3D &localYOld = s->geometry().localY();

			const IBKMK::Vector3D &localXNew = modS.geometry().localX();
			const IBKMK::Vector3D &localYNew = modS.geometry().localY();

			std::vector<VICUS::SubSurface> newSubSurfs(modS.subSurfaces().size() );
			// now we also have to scale the sub surfaces
			for ( unsigned int i = 0; i<modS.subSurfaces().size(); ++i ) {

				Q_ASSERT(modS.subSurfaces().size() == modS.geometry().holes().size());

				newSubSurfs[i] = modS.subSurfaces()[i];
				VICUS::SubSurface &subS = const_cast<VICUS::SubSurface &>(s->subSurfaces()[i]);
				std::vector<IBKMK::Vector2D> newSubSurfVertexes(subS.m_polygon2D.vertexes().size());

				for ( unsigned int j=0; j<subS.m_polygon2D.vertexes().size(); ++j ) {
						IBKMK::Vector2D v2D = subS.m_polygon2D.vertexes()[j];

						// we now calculate the 3D points of the sub surface
						// afterwards we scale up the surface
						IBKMK::Vector3D v3D = offset3dOld + localXOld * v2D.m_x + localYOld * v2D.m_y;

						Vic3D::Transform3D t;
						if (s->subSurfaces()[i].m_selected && s->subSurfaces()[i].m_visible) {
							t.scale(IBKVector2QVector(scale) );
							t.translate(IBKVector2QVector(trans) );
							t.setTranslation(t.toMatrix()*IBKVector2QVector(v3D) );
							v3D = QVector2IBKVector(t.translation() );
						}
						// and we calculate back the projection on the plane
						// we have to take the offset of our new scaled polygon
						IBKMK::planeCoordinates(offset3dNew, localXNew, localYNew, v3D, newSubSurfVertexes[j].m_x, newSubSurfVertexes[j].m_y);

				}

				newSubSurfs[i].m_polygon2D = newSubSurfVertexes;
			}
			// we update the 2D polyline
			modS.setSubSurfaces(newSubSurfs);

			// We update the floor area
			if (modS.m_parent != nullptr && modS.geometry().normal().m_z < -0.707)
				VICUS::KeywordList::setParameter(dynamic_cast<VICUS::Room*>(modS.m_parent)->m_para, "Room::para_t", VICUS::Room::P_Area, modS.geometry().area() );

			modifiedSurfaces.push_back(modS);
		}

		// handle only selected sub surfaces where parent is not selected
		const VICUS::SubSurface * ss = dynamic_cast<const VICUS::SubSurface *>(o);
		if (ss != nullptr) {

			VICUS::Surface *parentSurf = dynamic_cast<VICUS::Surface*>(ss->m_parent);
			if (parentSurf != nullptr && ss->m_parent->m_selected && ss->m_parent->m_visible)
				continue; // already handled by surface scaling

			if(handledSurfaces.find(parentSurf) != handledSurfaces.end())
				continue; // surface already handled

			// parentSurf->m_selected = true; // !!!! only for now till we adjust the function !!!
			VICUS::Surface modS(*parentSurf);

			// we cache our poldon data
			IBKMK::Vector3D offset3d = modS.geometry().offset();
			const IBKMK::Vector3D &localX = modS.geometry().localX();
			const IBKMK::Vector3D &localY = modS.geometry().localY();

			std::vector<VICUS::SubSurface> newSubSurfs(modS.subSurfaces().size() );
			// now we also have to scale the sub surfaces
			for ( unsigned int i = 0; i<modS.subSurfaces().size(); ++i ) {

				Q_ASSERT(modS.subSurfaces().size() == modS.geometry().holes().size());
				newSubSurfs[i] = modS.subSurfaces()[i];

				if (!modS.subSurfaces()[i].m_selected || !modS.subSurfaces()[i].m_visible)
					continue;

				// we only modify our selected sub surface
				VICUS::SubSurface &subS = const_cast<VICUS::SubSurface &>(modS.subSurfaces()[i]);
				std::vector<IBKMK::Vector2D> newSubSurfVertexes(subS.m_polygon2D.vertexes().size());


				for ( unsigned int j=0; j<subS.m_polygon2D.vertexes().size(); ++j ) {
						IBKMK::Vector2D v2D = subS.m_polygon2D.vertexes()[j];

						// we now calculate the 3D points of the sub surface
						// afterwards we scale up the surface
						IBKMK::Vector3D v3D = offset3d + localX * v2D.m_x + localY * v2D.m_y;

						Vic3D::Transform3D t;
						t.scale(IBKVector2QVector(scale) );
						t.translate(IBKVector2QVector(trans) );
						t.setTranslation(t.toMatrix()*IBKVector2QVector(v3D) );
						v3D = QVector2IBKVector(t.translation() );

						// and we calculate back the projection on the plane
						// we have to take the offset of our new scaled polygon
						IBKMK::planeCoordinates(modS.geometry().offset(), localX, localY, v3D, newSubSurfVertexes[j].m_x, newSubSurfVertexes[j].m_y);
				}

				newSubSurfs[i].m_polygon2D = newSubSurfVertexes;
			}
			handledSurfaces.insert(parentSurf);

			// we update the 2D polyline
			modS.setSubSurfaces(newSubSurfs);
			modifiedSurfaces.push_back(modS);
		}

		// TODO : Netzwerk zeugs
	}

	// ToDO Update Volumen

	// in case operation was executed without any selected objects - should be prevented
	if (modifiedSurfaces.empty())
		return;

	SVUndoModifySurfaceGeometry * undoSurf = new SVUndoModifySurfaceGeometry(tr("Scaled surface geometry."), modifiedSurfaces );
	undoSurf->push();

	// reset local transformation matrix
	SVViewStateHandler::instance().m_selectedGeometryObject->m_transform = Vic3D::Transform3D();
	m_cso->setTranslation(transLCSO);
}

void SVPropEditGeometry::rotate() {
	FUNCID("SVPropEditGeometry::rotate");

	// we now apply the already specified transformation
	// get rotation and scale vector from selected geometry object
	QVector4D qrotate = SVViewStateHandler::instance().m_selectedGeometryObject->m_transform.rotation().toVector4D();
	// use IBKMK class to permorm rotation with IBKMK Vector3D
	IBKMK::Quaternion rotate((double) qrotate.w(),(double) qrotate.x(),(double) qrotate.y(),(double) qrotate.z());
	IBKMK::Vector3D trans = QVector2IBKVector( SVViewStateHandler::instance().m_selectedGeometryObject->m_transform.translation() );

	if (rotate == IBKMK::Quaternion())
		return;

	// compose vector of modified surface geometries
	std::vector<VICUS::Surface> modifiedSurfaces;
	QVector3D transLCSO = m_cso->translation();

	// process all selected objects
	for (const VICUS::Object * o : SVViewStateHandler::instance().m_selectedGeometryObject->m_selectedObjects) {
		// handle surfaces
		const VICUS::Surface * s = dynamic_cast<const VICUS::Surface *>(o);
		if (s == nullptr) continue; // skip all others

		const VICUS::Polygon3D &poly = s->polygon3D();
		// TODO : Stephan, can we fix already broken polygons?
		if (!poly.isValid())
			continue; // skip invalid polygons

		// original 3D vertexes
		const std::vector<IBKMK::Vector3D> &vertexes = poly.vertexes();
		// vertexes of the 2D polygon
		const std::vector<IBKMK::Vector2D> &polylineVertexes = poly.polyline().vertexes();

		// create vector for modified vertexes, already resized correctly
		std::vector<IBKMK::Vector3D> newVertexes(vertexes.size());

		// rotate o copy of polyline vector
		IBKMK::Vector3D rotaLocalX = poly.localX();
		IBKMK::Vector3D rotaLocalY = poly.localY();
		rotate.rotateVector(rotaLocalX);
		rotate.rotateVector(rotaLocalY);

		// rotate vertexes and store in new vector
		newVertexes[0] = vertexes[0];
		rotate.rotateVector(newVertexes[0]);

		// transform the other vertexes
		// we take our polyline and rotated local axes to construct our rotated polygon3D
		// this operation is faster than rotating the individual vertexes and also less prone to rounding errors
		for (unsigned int i=1; i<vertexes.size(); ++i) {
			newVertexes[i] = newVertexes[0] + rotaLocalX * polylineVertexes[i].m_x + rotaLocalY * polylineVertexes[i].m_y;
		}

		// we also want to translate all points back to its original center
		for ( IBKMK::Vector3D & v : newVertexes )
			v += trans;

		// create a copy of our modified surface
		VICUS::Surface modS = *s;
		modS.setPolygon3D( newVertexes );
		// TODO : Stephan, if the surface didn't "survive" the rotation, this will rip the building geometry apart...
		//        can't we fix this somehow? Or at least check, why it is broken?
		if (modS.polygon3D().isValid() )
			modifiedSurfaces.push_back(modS);
		else
			IBK::IBK_Message(IBK::FormatString("Geometry of surface %1 is broken after rotation.")
							 .arg(modS .m_displayName.toStdString()),
							 IBK::MSG_ERROR, FUNC_ID, IBK::VL_STANDARD);

		// TODO : Netzwerk zeugs
	}

	// in case operation was executed without any selected objects - should be prevented
	if (modifiedSurfaces.empty())
		return;

	blockSignals(true);
	SVUndoModifySurfaceGeometry * undo = new SVUndoModifySurfaceGeometry(tr("Rotated geometry"), modifiedSurfaces );
	undo->push();
	// reset local transformation matrix
	SVViewStateHandler::instance().m_selectedGeometryObject->m_transform = Vic3D::Transform3D();
	m_cso->setTranslation(transLCSO);
	blockSignals(false);
}


int SVPropEditGeometry::requestCopyOperation(QWidget * parent, const QString & title, const QString & text,
											 const QString & button1, const QString & button2)
{
	QMessageBox dlg(QMessageBox::Question, title, text, QMessageBox::Cancel, parent);

	QPushButton * btn = new QPushButton(button1);
	dlg.addButton(btn, (QMessageBox::ButtonRole)(QMessageBox::YesRole));
	dlg.setDefaultButton(btn);

	btn = new QPushButton(button2);
	dlg.addButton(btn, (QMessageBox::ButtonRole)(QMessageBox::NoRole));

	int res = dlg.exec();
	if (res == QMessageBox::Cancel)
		return -1;
	if (res == QMessageBox::YesRole)
		return 1;
	else
		return 2;
}


// *** private functions ***

void SVPropEditGeometry::updateUi() {

	// update our selection lists
	std::set<const VICUS::Object*> sel;

	// first we get how many surfaces are selected
	project().selectObjects(sel, VICUS::Project::SG_All, false, false);

	// we also have to cache all existing names, so we take alle existing objects
	m_selSurfaces.clear();
	m_selRooms.clear();
	m_selSubSurfaces.clear();
	m_selBuildings.clear();
	m_selBuildingLevels.clear();

	m_subSurfNames.clear();
	m_surfNames.clear();
	m_buildingNames.clear();
	m_buildingLevelNames.clear();


	// process all selected objects and sort them into vectors
	for (const VICUS::Object * o : sel) {
		const VICUS::Surface * s = dynamic_cast<const VICUS::Surface *>(o);
		if (s != nullptr ) {
			m_surfNames.insert(s->m_displayName );
			if (s->m_selected && s->m_visible)
				m_selSurfaces.push_back(s);
		}
		const VICUS::Room * r = dynamic_cast<const VICUS::Room *>(o);
		if (r != nullptr ) {
			m_roomNames.insert(r->m_displayName );
			if (r->m_selected && r->m_visible)
				m_selRooms.push_back(r);
		}
		const VICUS::SubSurface * sub = dynamic_cast<const VICUS::SubSurface *>(o);
		if (sub != nullptr ) {
			m_subSurfNames.insert(sub->m_displayName );
			if (sub->m_selected && sub->m_visible)
				m_selSubSurfaces.push_back(sub);
		}
		const VICUS::BuildingLevel * bl = dynamic_cast<const VICUS::BuildingLevel *>(o);
		if (bl != nullptr ) {
			m_buildingLevelNames.insert(bl->m_displayName );
			if (bl->m_selected && bl->m_visible)
				m_selBuildingLevels.push_back(bl);
		}
		const VICUS::Building * b = dynamic_cast<const VICUS::Building *>(o);
		if (b != nullptr ) {
			m_subSurfNames.insert(b->m_displayName );
			if (b->m_selected && b->m_visible)
				m_selBuildings.push_back(b);
		}
	}

	// enable copy functions only if respective objects are selected
	m_ui->pushButtonCopySurfaces->setEnabled(!m_selSurfaces.empty());
	m_ui->pushButtonCopyRooms->setEnabled(!m_selRooms.empty());
	// TODO Stephan
	m_ui->pushButtonCopyBuildingLevels->setEnabled(!m_selBuildingLevels.empty());
	m_ui->pushButtonCopyBuilding->setEnabled(!m_selBuildings.empty());

	// handling if surfaces are selected
	if (!m_selSurfaces.empty()) {

		if ( m_selSurfaces.size() == 1 ) {
			const VICUS::Surface *s = m_selSurfaces[0];
			m_rotationState = RS_Normal;
			m_ui->toolButtonAbs->setEnabled(true);
			setRotation(s->geometry().normal() );
		}
		else {

			switch (m_rotationState) {
				case SVPropEditGeometry::RS_XAxis:
					setRotation( QVector2IBKVector(m_cso->localXAxis() ) );
				break;
				case SVPropEditGeometry::RS_YAxis:
					setRotation( QVector2IBKVector(m_cso->localYAxis() ) );
				break;
				case SVPropEditGeometry::RS_Normal:
				case SVPropEditGeometry::RS_ZAxis:
					m_rotationState = RS_ZAxis;
					setRotation( QVector2IBKVector(m_cso->localZAxis() ) );
				break;
				case SVPropEditGeometry::NUM_RS:
				break;

			}
			if(m_modificationType == ModificationType::MT_Rotate) {
				setToolButtonsRotationState(false);
			}
		}

		// enable "add subsurface" button
		m_ui->pushButtonAddWindow->setEnabled(true);
	}
	else {
		m_ui->pushButtonAddWindow->setEnabled(false);

		// handling if only sub-surfaces are selected
		if (!m_selSubSurfaces.empty()) {
			if ( m_selSubSurfaces.size() == 1 ) {
				const VICUS::SubSurface *sub = m_selSubSurfaces[0];
				const VICUS::Surface *s = dynamic_cast<const VICUS::Surface*>(sub->m_parent);
				setRotation(s->geometry().normal() );
				m_ui->toolButtonAbs->setEnabled(true);
			}
			else {
				if(m_modificationType == ModificationType::MT_Rotate) {
					setToolButtonsRotationState(false);
				}
				setRotation( QVector2IBKVector(m_cso->localZAxis() ) );
			}
		}
	}


	// compute dimensions of bounding box (dx, dy, dz) and center point of all selected surfaces
	m_bbDim[OM_Local] = project().boundingBox(m_selSurfaces, m_selSubSurfaces, m_bbCenter[OM_Local],
												  QVector2IBKVector(m_cso->translation() ),
												  QVector2IBKVector(m_cso->localXAxis() ),
												  QVector2IBKVector(m_cso->localYAxis() ),
												  QVector2IBKVector(m_cso->localZAxis() ) );
	m_bbDim[OM_Global] = project().boundingBox(m_selSurfaces, m_selSubSurfaces, m_bbCenter[OM_Global]);

	SVViewStateHandler::instance().m_localCoordinateViewWidget->setBoundingBoxDimension(m_bbDim[m_orientationMode]);
	// position local coordinate system, but only if we are showing the edit page
	m_cso->setTranslation(IBKVector2QVector(m_bbCenter[m_orientationMode]) );

	// update local coordinates
	Vic3D::Transform3D t;
	t.setTranslation(IBKVector2QVector(m_bbCenter[m_orientationMode]) );
	setCoordinates( t ); // calls updateInputs() internally

}

void SVPropEditGeometry::updateOrientationMode() {
	// we update the button state
	m_ui->toolButtonLocalCoordinateOrientation->setChecked(m_orientationMode == OM_Local);

	// we have to update our bounding box dimensions in our specific coordinate system
	// compute dimensions of bounding box (dx, dy, dz) and center point of all selected surfaces
	m_bbDim[OM_Local] = project().boundingBox( m_selSurfaces, m_selSubSurfaces, m_bbCenter[OM_Local],
											   QVector2IBKVector(m_cso->translation() ),
											   QVector2IBKVector(m_cso->localXAxis() ),
											   QVector2IBKVector(m_cso->localYAxis() ),
											   QVector2IBKVector(m_cso->localZAxis() ) );
	m_bbDim[OM_Global] = project().boundingBox(m_selSurfaces, m_selSubSurfaces, m_bbCenter[OM_Global]);

	SVViewStateHandler::instance().m_localCoordinateViewWidget->setBoundingBoxDimension(m_bbDim[m_orientationMode]);
	// we also update all the line edits and boxes
	updateInputs();
}

void SVPropEditGeometry::onWheelTurned(double offset, QtExt::ValidatingLineEdit * lineEdit) {
	if (!lineEdit->isValid())
		return; // invalid input, do nothing
	double val = lineEdit->value();
	val += offset;
	lineEdit->setValue(val); // this does not trigger any signals, so we need to send change info manually
	onLineEditTextChanged(lineEdit);
}

void SVPropEditGeometry::initializeCopy() {
	// initialize the translation vector
	m_translation = IBKMK::Vector3D( 0, 0, 0 );

	// we set up the lineEdit fields
	m_ui->lineEditXCopy->setValue( m_translation.m_x);
	m_ui->lineEditYCopy->setValue( m_translation.m_y);
	m_ui->lineEditZCopy->setValue( m_translation.m_z);
}

void SVPropEditGeometry::updateCoordinateSystemLook() {
	if (SVViewStateHandler::instance().m_geometryView == nullptr)
		return; // do nothing while initializing
	// adjust appearance of local coordinate system
	if (m_ui->stackedWidget->currentIndex() == 0) {
		// put local coordinate system back into "plain" mode
		if (m_cso->m_geometryTransformMode != 0) {
			m_cso->m_geometryTransformMode = 0;
			SVViewStateHandler::instance().m_geometryView->refreshSceneView();
		}
	}
	else {
		// put local coordinate system back into correct transform mode
		switch (m_modificationType) {
			case SVPropEditGeometry::MT_Translate:
				if (m_cso->m_geometryTransformMode != Vic3D::CoordinateSystemObject::TM_Translate) {
					m_cso->m_geometryTransformMode = Vic3D::CoordinateSystemObject::TM_Translate;
					SVViewStateHandler::instance().m_geometryView->refreshSceneView();
				}
			break;

			case SVPropEditGeometry::MT_Rotate:
				if (m_cso->m_geometryTransformMode != Vic3D::CoordinateSystemObject::TM_RotateMask) {
					m_cso->m_geometryTransformMode = Vic3D::CoordinateSystemObject::TM_RotateMask;
					SVViewStateHandler::instance().m_geometryView->refreshSceneView();
				}
			break;

			case SVPropEditGeometry::MT_Scale:
				if (m_cso->m_geometryTransformMode != Vic3D::CoordinateSystemObject::TM_ScaleMask) {
					m_cso->m_geometryTransformMode = Vic3D::CoordinateSystemObject::TM_ScaleMask;
					SVViewStateHandler::instance().m_geometryView->refreshSceneView();
				}
			break;
		}
	}
}

bool SVPropEditGeometry::eventFilter(QObject * target, QEvent * event) {

	if ( event->type() == QEvent::Wheel ) {
		// we listen to scroll wheel turns only for some line edits
		if (target == m_ui->lineEditX ||
				target == m_ui->lineEditY ||
				target == m_ui->lineEditZ ||
				target == m_ui->lineEditInclination ||
				target == m_ui->lineEditOrientation ||
				target == m_ui->lineEditXCopy ||
				target == m_ui->lineEditYCopy||
				target == m_ui->lineEditZCopy )
		{	double delta = 0.1; // for copy operation

			switch (m_modificationType) {
				case MT_Translate				:
				case MT_Scale					: delta = 0.01; break;
				case MT_Rotate					: delta = 1; break;
			}

			QWheelEvent *wheelEvent = static_cast<QWheelEvent*>(event);
			// offset are changed in 0.01 steps
			double offset = (wheelEvent->delta()>0) ? delta : -delta;
			onWheelTurned(offset, qobject_cast<QtExt::ValidatingLineEdit*>(target)); // we know that target points to a ValidatingLineEdit
		}
	}
	else if ( event->type() == QEvent::KeyPress ) {
		QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
		if (keyEvent->matches(QKeySequence::Copy)) {
			// we enter translation mode
			m_modificationType = MT_Translate;
			m_copyMode = true;
		}
	}
	else if ( event->type() == QEvent::FocusOut ) {
		QLineEdit *edit = dynamic_cast<QLineEdit*>(target);
		qDebug() << "Focus out -> return press event on " << edit;
		if(edit != nullptr)
			emit edit->returnPressed();
	}

	return false;
}


void SVPropEditGeometry::on_lineEditX_returnPressed() {
	// check if entered value is valid, if not reset it to its default
	if ( !m_ui->lineEditX->isValid() ) {
		m_ui->lineEditX->setValue( m_originalValues.m_x );
		return;
	}
	if ( std::fabs( m_originalValues.m_x - m_ui->lineEditX->value() ) < 1E-3 )
		return;


	//	double tempXValue = m_ui->lineEditX->value();
	switch ( m_modificationType ) {
		case MT_Translate: translate(); break;
		case MT_Scale: scale(); break;
		case MT_Rotate: rotate(); break;
	}
}

void SVPropEditGeometry::on_lineEditY_returnPressed() {
	// check if entered value is valid, if not reset it to its default
	if ( !m_ui->lineEditY->isValid() ) {
		m_ui->lineEditX->setValue( m_originalValues.m_y );
		return;
	}
	if ( std::fabs( m_originalValues.m_y - m_ui->lineEditY->value() ) < 1E-3 )
		return;

	//	double tempXValue = m_ui->lineEditX->value();
	switch ( m_modificationType ) {
		case MT_Translate: translate(); break;
		case MT_Scale: scale(); break;
		case MT_Rotate: rotate(); break;
	}
}

void SVPropEditGeometry::on_lineEditZ_returnPressed(){
	// check if entered value is valid, if not reset it to its default
	if ( !m_ui->lineEditZ->isValid() ) {
		m_ui->lineEditZ->setValue( m_originalValues.m_z );
		return;
	}
	if ( std::fabs( m_originalValues.m_z - m_ui->lineEditZ->value() ) < 1E-3 )
		return;

	switch ( m_modificationType ) {
		case MT_Translate: translate(); break;
		case MT_Scale: scale(); break;
		case MT_Rotate: rotate(); break;
	}
}


void SVPropEditGeometry::setState(const SVPropEditGeometry::ModificationType & type,
								  const SVPropEditGeometry::ModificationState & state)
{
	// we save our editing type (scale, translate, rotate)

	m_modificationType = type;
	m_modificationState[type] = state;

	// check/uncheck operation buttons
	setToolButton();

	// we also set the state of the tool buttons
	setToolButtonAbsMode();

	updateInputs();
	updateCoordinateSystemLook();
}

void SVPropEditGeometry::updateInputs() {

	ModificationState state = m_modificationState[m_modificationType];

	switch (m_modificationType) {
		case MT_Translate : {
			showDeg(false);
			showRotation(false);

			switch (state) {
				case MS_Absolute: {

					m_ui->labelX->setText("X");
					m_ui->labelY->setText("Y");
					m_ui->labelZ->setText("Z");

					// cache current local coordinate systems position as fall-back values
					m_originalValues = QVector2IBKVector(m_localCoordinatePosition.translation());

					QVector3D translation(m_localCoordinatePosition.translation());

					if (m_orientationMode == OM_Local) {
						QVector3D newTrans( translation.x()*m_cso->localXAxis().x() + translation.y()*m_cso->localXAxis().y() + translation.z()*m_cso->localXAxis().z(),
											translation.x()*m_cso->localYAxis().x() + translation.y()*m_cso->localYAxis().y() + translation.z()*m_cso->localYAxis().z(),
											translation.x()*m_cso->localZAxis().x() + translation.y()*m_cso->localZAxis().y() + translation.z()*m_cso->localZAxis().z() );
						translation = newTrans;
					}

					m_ui->lineEditX->setValue(translation.x());
					m_ui->lineEditY->setValue(translation.y());
					m_ui->lineEditZ->setValue(translation.z());
				} break;

				default:
					m_ui->labelX->setText("ΔX");
					m_ui->labelY->setText("ΔY");
					m_ui->labelZ->setText("ΔZ");

					m_originalValues = IBKMK::Vector3D();

					m_ui->lineEditX->setValue(0);
					m_ui->lineEditY->setValue(0);
					m_ui->lineEditZ->setValue(0);
			} // switch

		} break;


		case MT_Rotate: {
			showDeg();
			showRotation(false);

			m_ui->labelX->setText("X");
			m_ui->labelY->setText("Y");
			m_ui->labelZ->setText("Z");

			switch (state) {
				case MS_Absolute: {
					showRotation();
					showDeg(false);

					m_ui->lineEditX->setValue(0);
					m_ui->lineEditY->setValue(0);
					m_ui->lineEditZ->setValue(0);
				}
					break;

				default:
					m_ui->lineEditX->setText( QString("%L1").arg( 0.0,0, 'f', 3 ) );
					m_ui->lineEditY->setText( QString("%L1").arg( 0.0,0, 'f', 3 ) );
					m_ui->lineEditZ->setText( QString("%L1").arg( 0.0,0, 'f', 3 ) );
			}

		} break;


		case MT_Scale: {

			showDeg(false);
			showRotation(false);

			switch (state) {
				case MS_Absolute: {

					m_ui->labelX->setText("L<sub>X</sub>");
					m_ui->labelY->setText("W<sub>Y</sub>");
					m_ui->labelZ->setText("H<sub>Z</sub>");

					m_originalValues = m_bbCenter[m_orientationMode];

					m_ui->lineEditX->setValue(m_bbDim[m_orientationMode].m_x);
					m_ui->lineEditY->setValue(m_bbDim[m_orientationMode].m_y);
					m_ui->lineEditZ->setValue(m_bbDim[m_orientationMode].m_z);

					break;
				}
				default:
					m_ui->labelX->setText("X");
					m_ui->labelY->setText("Y");
					m_ui->labelZ->setText("Z");

					m_originalValues = IBKMK::Vector3D( 1,1,1 );

					m_ui->lineEditX->setValue(m_originalValues.m_x );
					m_ui->lineEditY->setValue(m_originalValues.m_y );
					m_ui->lineEditZ->setValue(m_originalValues.m_z );
			}
		} break;
	} // switch modification type
}

void SVPropEditGeometry::setToolButton() {


	m_ui->toolButtonTrans->setChecked(false);
	m_ui->toolButtonRotate->setChecked(false);
	m_ui->toolButtonScale->setChecked(false);

	switch (m_modificationType) {
		case MT_Translate:		m_ui->toolButtonTrans->setChecked(true);			break;
		case MT_Rotate:			m_ui->toolButtonRotate->setChecked(true);			break;
		case MT_Scale:			m_ui->toolButtonScale->setChecked(true);			break;
	}
}

void SVPropEditGeometry::setToolButtonAbsMode() {
	m_ui->toolButtonRel->blockSignals(true);
	m_ui->toolButtonAbs->blockSignals(true);

	switch (m_modificationType) {
		case MT_Translate:
			m_ui->toolButtonAbs->setText( tr("Move to world coordinates") );
			m_ui->toolButtonRel->setText( tr("Relative translation") );
			break;
		case MT_Rotate:
			m_ui->toolButtonAbs->setText( tr("Align surface to angles") );
			m_ui->toolButtonRel->setText( tr("Relative rotation") );
			break;
		case MT_Scale:
			m_ui->toolButtonAbs->setText( tr("Resize surfaces") );
			m_ui->toolButtonRel->setText( tr("Relative scaling") );
			break;
	}

	bool checkAbsState = m_modificationState[m_modificationType] == ModificationState::MS_Absolute;

	m_ui->toolButtonAbs->setChecked(checkAbsState);
	m_ui->toolButtonRel->setChecked(!checkAbsState);

	m_ui->toolButtonAbs->blockSignals(false);
	m_ui->toolButtonRel->blockSignals(false);

}

void SVPropEditGeometry::setToolButtonsRotationState(bool absOn) {
	m_ui->toolButtonAbs->setEnabled(absOn);
	m_ui->toolButtonRel->setChecked(!absOn);
}

void SVPropEditGeometry::showDeg(const bool & show) {
	if ( show ) {
		m_ui->labelXDeg->show();
		m_ui->labelYDeg->show();
		m_ui->labelZDeg->show();
	}
	else {
		m_ui->labelXDeg->hide();
		m_ui->labelYDeg->hide();
		m_ui->labelZDeg->hide();
	}
}

void SVPropEditGeometry::showRotation(const bool & abs) {
	// we show all that is necessary for absolute Rotation Mode
	m_ui->lineEditOrientation->blockSignals(true);
	m_ui->lineEditInclination->blockSignals(true);
	if ( abs ) {
		m_ui->widgetXYZ->hide();
		m_ui->widgetRota->show();
	}
	else {
		m_ui->widgetXYZ->show();
		m_ui->widgetRota->hide();
	}
	m_ui->lineEditOrientation->blockSignals(false);
	m_ui->lineEditInclination->blockSignals(false);
}


void SVPropEditGeometry::on_toolButtonTrans_clicked() {
	setState(MT_Translate, m_modificationState[MT_Translate]);
}

void SVPropEditGeometry::on_toolButtonRotate_clicked() {
	setState(MT_Rotate, m_modificationState[MT_Rotate]);
}

void SVPropEditGeometry::on_toolButtonScale_clicked() {
	setState(MT_Scale, m_modificationState[MT_Scale]);
}


void SVPropEditGeometry::on_lineEditOrientation_returnPressed() {
	// check if entered value is valid, if not reset it to its default
	double orientation = std::atan2(m_normal.m_x, ( m_normal.m_y == 0. ? 1E-8 : m_normal.m_y ) ) /IBK::DEG2RAD ;
	if ( !m_ui->lineEditOrientation->isValid() ) {
		m_ui->lineEditOrientation->setValue( orientation < 0 ? ( orientation + 360 ) : orientation  );
		return;
	}
	if ( std::fabs( orientation < 0 ? ( orientation + 360 ) : orientation - m_ui->lineEditOrientation->value() ) < 1E-3 )
		return;


	rotate();
}

void SVPropEditGeometry::on_lineEditInclination_returnPressed() {

	if ( !m_ui->lineEditInclination->isValid() ) {
		m_ui->lineEditInclination->setValue( std::acos(m_normal.m_z)/IBK::DEG2RAD );
		return;
	}

	rotate();
}


void SVPropEditGeometry::on_lineEditOrientation_editingFinished() {
	QLineEdit * lineEdit = qobject_cast<QLineEdit *>( QObject::sender() );

	// Ignore undesirable signals.
	if ( lineEdit->isModified() )
		return;
	lineEdit->setModified( false );

	on_lineEditOrientation_returnPressed();
}

void SVPropEditGeometry::on_lineEditInclination_editingFinished() {
	QLineEdit * lineEdit = qobject_cast<QLineEdit *>( QObject::sender() );

	// Ignore undesirable signals.
	if ( lineEdit->isModified() )
		return;
	lineEdit->setModified( false );

	on_lineEditInclination_returnPressed();
}


void SVPropEditGeometry::on_lineEditXCopy_editingFinished() {
	if ( m_ui->lineEditXCopy->isValid() )
		m_translation.m_x = m_ui->lineEditXCopy->value();

	m_ui->lineEditXCopy->setValue(m_translation.m_x );
}

void SVPropEditGeometry::on_lineEditYCopy_editingFinished() {
	if ( m_ui->lineEditYCopy->isValid() )
		m_translation.m_y = m_ui->lineEditYCopy->value();

	m_ui->lineEditYCopy->setValue(m_translation.m_y );
}

void SVPropEditGeometry::on_lineEditZCopy_editingFinished() {
	if ( m_ui->lineEditZCopy->isValid() )
		m_translation.m_z = m_ui->lineEditZCopy->value();

	m_ui->lineEditZCopy->setValue(m_translation.m_z );
}


void SVPropEditGeometry::onLineEditTextChanged(QtExt::ValidatingLineEdit * lineEdit) {
	// take transformation value, and if valid, modify transform in wire frame object

	if (!lineEdit->isValid())
		return;

	ModificationState state = m_modificationState[m_modificationType];
	ModificationType type = m_modificationType;

	// compose all transformation values
	switch (type) {

		// Translation operation
		case MT_Translate: {

			// compose translation vector depending on translation mode
			switch (state) {
				case MS_Absolute : {
					// for this operation, we need all three coordinates
					QVector3D targetPos((float)m_ui->lineEditX->value(), (float)m_ui->lineEditY->value(), (float)m_ui->lineEditZ->value());
					// now compose a transform object and set it in the wireframe object
					Vic3D::Transform3D trans;
					// compute offset from current local coordinate system position
					QVector3D translation;
					if (m_orientationMode == OM_Local) {
						QVector3D localTrans (m_cso->translation() );
						QVector3D newTrans( localTrans.x()*m_cso->localXAxis().x() + localTrans.y()*m_cso->localXAxis().y() + localTrans.z()*m_cso->localXAxis().z(),
											localTrans.x()*m_cso->localYAxis().x() + localTrans.y()*m_cso->localYAxis().y() + localTrans.z()*m_cso->localYAxis().z(),
											localTrans.x()*m_cso->localZAxis().x() + localTrans.y()*m_cso->localZAxis().y() + localTrans.z()*m_cso->localZAxis().z() );


						if (lineEdit == m_ui->lineEditX) {
							translation = (targetPos.x() - newTrans.x()) * m_cso->localXAxis();
						}
						else if (lineEdit == m_ui->lineEditX) {
							translation = (targetPos.y() - newTrans.y()) * m_cso->localYAxis();
						}
						else {
							translation = (targetPos.z() - newTrans.z()) * m_cso->localZAxis();
						}
					}
					else
						translation = targetPos - m_localCoordinatePosition.translation();

					trans.setTranslation(translation);
					SVViewStateHandler::instance().m_selectedGeometryObject->m_transform = trans;
					const_cast<Vic3D::SceneView*>(SVViewStateHandler::instance().m_geometryView->sceneView())->renderNow();
					const_cast<Vic3D::SceneView*>(SVViewStateHandler::instance().m_geometryView->sceneView())->renderLater();
				} break;

					// inputs are global coordinate offsets
				case MS_Relative : {
					// for this operation, we need all three coordinates
					QVector3D translation((float)m_ui->lineEditX->value(), (float)m_ui->lineEditY->value(), (float)m_ui->lineEditZ->value());
					// now compose a transform object and set it in the wireframe object
					Vic3D::Transform3D trans;

					if (m_orientationMode == OM_Local) {
						QVector3D newTrans( translation.x()*m_cso->localXAxis().x() + translation.y()*m_cso->localYAxis().x() + translation.z()*m_cso->localZAxis().x(),
											translation.x()*m_cso->localXAxis().y() + translation.y()*m_cso->localYAxis().y() + translation.z()*m_cso->localZAxis().y(),
											translation.x()*m_cso->localXAxis().z() + translation.y()*m_cso->localYAxis().z() + translation.z()*m_cso->localZAxis().z() );
						translation = newTrans;
					}

					trans.setTranslation(translation);
					SVViewStateHandler::instance().m_selectedGeometryObject->m_transform = trans;
					const_cast<Vic3D::SceneView*>(SVViewStateHandler::instance().m_geometryView->sceneView())->renderNow();
					const_cast<Vic3D::SceneView*>(SVViewStateHandler::instance().m_geometryView->sceneView())->renderLater();
				} break;
			}
		} break;

		// Scale operation
		case MT_Scale: {
			switch (state) {
				case MS_Absolute: {
					// for this operation, we need at first the dimensions of the bounding box from
					IBKMK::Vector3D targetScale(m_ui->lineEditX->value(), m_ui->lineEditY->value(), m_ui->lineEditZ->value());
					// compute offset from current local coordinate system position
					QVector3D scale;

					if (m_orientationMode == OM_Local) {
						// we know the local bounding box
						// we can scale up the local bounding box by the factor
						// finally we calc back our dimensions of the global bounding box

						IBKMK::Vector3D newBBDimLocal = m_bbDim[OM_Local];
						IBKMK::Vector3D newBBDimGlobal = m_bbDim[OM_Global];

						QVector3D local;
						double diff;

						if (lineEdit == m_ui->lineEditX) { // we scale with the x axis
							newBBDimLocal.m_x = targetScale.m_x;
							diff = newBBDimLocal.m_x - m_bbDim[OM_Local].m_x;
							local = m_cso->localXAxis();
						}
						else if (lineEdit == m_ui->lineEditY) {  // we scale with the y axis
							newBBDimLocal.m_y = targetScale.m_y;
							diff = newBBDimLocal.m_y - m_bbDim[OM_Local].m_y;
							local = m_cso->localYAxis();
						}
						else { // we scale with the z axis
							newBBDimLocal.m_z = targetScale.m_z;
							diff = newBBDimLocal.m_z - m_bbDim[OM_Local].m_z;
							local = m_cso->localZAxis();
						}

						newBBDimGlobal.m_x = m_bbDim[OM_Global].m_x + diff * std::fabs(local.x());
						newBBDimGlobal.m_y = m_bbDim[OM_Global].m_y + diff * std::fabs(local.y());
						newBBDimGlobal.m_z = m_bbDim[OM_Global].m_z + diff * std::fabs(local.z());

						scale.setX(newBBDimGlobal.m_x / (std::fabs(m_bbDim[OM_Global].m_x) < 1e-4 ? 1e-4 : m_bbDim[OM_Global].m_x));
						scale.setY(newBBDimGlobal.m_y / (std::fabs(m_bbDim[OM_Global].m_y) < 1e-4 ? 1e-4 : m_bbDim[OM_Global].m_y));
						scale.setZ(newBBDimGlobal.m_z / (std::fabs(m_bbDim[OM_Global].m_z) < 1e-4 ? 1e-4 : m_bbDim[OM_Global].m_z));
					}
					else {


						scale.setX( m_bbDim[m_orientationMode].m_x < 1E-4 ? 1.0 : ( std::fabs(targetScale.m_x ) < 1E-4 ? 1.0 : targetScale.m_x / ( m_bbDim[m_orientationMode].m_x < 1E-4 ? 1.0 : m_bbDim[m_orientationMode].m_x ) ) );
						scale.setY( m_bbDim[m_orientationMode].m_y < 1E-4 ? 1.0 : ( std::fabs(targetScale.m_y ) < 1E-4 ? 1.0 : targetScale.m_y / ( m_bbDim[m_orientationMode].m_y < 1E-4 ? 1.0 : m_bbDim[m_orientationMode].m_y ) ) );
						scale.setZ( m_bbDim[m_orientationMode].m_z < 1E-4 ? 1.0 : ( std::fabs(targetScale.m_z ) < 1E-4 ? 1.0 : targetScale.m_z / ( m_bbDim[m_orientationMode].m_z < 1E-4 ? 1.0 : m_bbDim[m_orientationMode].m_z ) ) );
						// now compose a transform object and set it in the wireframe object
						// first we scale our selected objects

					}

					Vic3D::Transform3D scaling;
					scaling.setScale(scale);


					// and then we also have to guarantee that the center point of the bounding box stays at the same position
					// this can be achieved since the center points are also just scaled by the specified scaling factors
					// so we know how big the absolute translation has to be
					IBKMK::Vector3D trans;

					const QVector3D &transObj = m_cso->transform().translation();

					trans.m_x = std::fabs(transObj.x()) < 1E-4 ? 0.0 : transObj.x() * ( 1 - scale.x() );
					trans.m_y = std::fabs(transObj.y()) < 1E-4 ? 0.0 : transObj.y() * ( 1 - scale.y() );
					trans.m_z = std::fabs(transObj.z()) < 1E-4 ? 0.0 : transObj.z() * ( 1 - scale.z() );
					scaling.setTranslation( IBKVector2QVector(trans) );

					// we give our transformation to the wire frame object
					SVViewStateHandler::instance().m_selectedGeometryObject->m_transform = scaling;
					const_cast<Vic3D::SceneView*>(SVViewStateHandler::instance().m_geometryView->sceneView())->renderNow(); // needed right now since two surfaces are shown
					const_cast<Vic3D::SceneView*>(SVViewStateHandler::instance().m_geometryView->sceneView())->renderLater();
				} break;

				case MS_Relative: {
					// for this operation, we get directly the surface scaling factors from the line edits
					// so it is basically like the absolute scaling, but we do not have to calculate the scaling factors
					QVector3D  scale(m_ui->lineEditX->value(), m_ui->lineEditY->value(), m_ui->lineEditZ->value());
					// compute offset from current local coordinate system position
					const Vic3D::CoordinateSystemObject *cso = m_cso;

					QVector3D lcsTrans = cso->translation();

					QVector3D newScale;
					if (m_orientationMode == OM_Local) {
						// we know the local bounding box
						// we can scale up the local bounding box by the factor
						// finally we calc back our dimensions of the global bounding box

						IBKMK::Vector3D newBBDimLocal = m_bbDim[OM_Local];
						IBKMK::Vector3D newBBDimGlobal = m_bbDim[OM_Global];

						QVector3D local;
						double diff;

						if (lineEdit == m_ui->lineEditX) { // we scale with the x axis
							newBBDimLocal.m_x = scale.x() * m_bbDim[OM_Local].m_x;
							diff = newBBDimLocal.m_x - m_bbDim[OM_Local].m_x;
							local = m_cso->localXAxis();
						}
						else if (lineEdit == m_ui->lineEditY) {  // we scale with the y axis
							newBBDimLocal.m_y = scale.y() * m_bbDim[OM_Local].m_y;
							diff = newBBDimLocal.m_y - m_bbDim[OM_Local].m_y;
							local = m_cso->localYAxis();
						}
						else { // we scale with the z axis
							newBBDimLocal.m_z = scale.z() * m_bbDim[OM_Local].m_z;
							diff = newBBDimLocal.m_z - m_bbDim[OM_Local].m_z;
							local = m_cso->localZAxis();
						}

						newBBDimGlobal.m_x = m_bbDim[OM_Global].m_x + diff * local.x();
						newBBDimGlobal.m_y = m_bbDim[OM_Global].m_y + diff * local.y();
						newBBDimGlobal.m_z = m_bbDim[OM_Global].m_z + diff * local.z();

						scale.setX(newBBDimGlobal.m_x / (std::fabs(m_bbDim[OM_Global].m_x) < 1e-4 ? 1e-4 : m_bbDim[OM_Global].m_x));
						scale.setY(newBBDimGlobal.m_y / (std::fabs(m_bbDim[OM_Global].m_y) < 1e-4 ? 1e-4 : m_bbDim[OM_Global].m_y));
						scale.setZ(newBBDimGlobal.m_z / (std::fabs(m_bbDim[OM_Global].m_z) < 1e-4 ? 1e-4 : m_bbDim[OM_Global].m_z));
					}
					else {



					}


					Vic3D::Transform3D scaling;
					scaling.setScale(scale);

					// and the we also hav to guarantee that the center point of the bounding box stays at the same position
					// this can be achieved since the center points are also just scaled by the specified scaling factors
					// so we know how big the absolute translation has to be
					IBKMK::Vector3D trans;
					const QVector3D &transObj = m_cso->transform().translation();

					trans.m_x = std::fabs(transObj.x()) < 1E-4 ? 0.0 : transObj.x() * ( 1.0 - scale.x() );
					trans.m_y = std::fabs(transObj.y()) < 1E-4 ? 0.0 : transObj.y() * ( 1.0 - scale.y() );
					trans.m_z = std::fabs(transObj.z()) < 1E-4 ? 0.0 : transObj.z() * ( 1.0 - scale.z() );

					scaling.setTranslation( IBKVector2QVector(trans) );

					// we give our transformation to the wire frame object
					SVViewStateHandler::instance().m_selectedGeometryObject->m_transform = scaling;
					const_cast<Vic3D::SceneView*>(SVViewStateHandler::instance().m_geometryView->sceneView())->renderNow();
					const_cast<Vic3D::SceneView*>(SVViewStateHandler::instance().m_geometryView->sceneView())->renderLater();

				} break;

			}
		} break;

		// Rotation operations
		case MT_Rotate: {
			switch (state) {

				case MS_Absolute: {
					// get the rotation angles
					double oriRad = m_ui->lineEditOrientation->value() * IBK::DEG2RAD;
					double incliRad = (90-m_ui->lineEditInclination->value()) * IBK::DEG2RAD;


					Vic3D::Transform3D rota;

					if (m_orientationMode == OM_Global) {
						switch (m_rotationState) {
						case SVPropEditGeometry::RS_Normal: {
							IBKMK::Vector3D newNormal(	std::sin( oriRad ) * std::cos( incliRad ),
														std::cos( oriRad ) * std::cos( incliRad ),
														std::sin( incliRad ) );

							// we only want to rotate if the normal vectors are not the same
							if ( checkVectors<4>( m_normal, newNormal ) )
								return; // do nothing

							// we find the rotation axis by taking the cross product of the normal vector and the normal vector we want to
							// rotate to
							IBKMK::Vector3D rotationAxis ( m_normal.crossProduct(newNormal).normalized() );
							qDebug() << "Rotation axis: " << rotationAxis.m_x << "\t" << rotationAxis.m_y << "\t" << rotationAxis.m_z;

							// we now also have to find the angle between both normals

							double angle = (float)angleBetweenVectorsDeg(m_normal, newNormal);

							rota.rotate(angle, IBKVector2QVector(rotationAxis) );
							qDebug() << "Roation angle: " << angle << " °";
						}
						break;
						case SVPropEditGeometry::RS_XAxis:
						break;
						case SVPropEditGeometry::RS_YAxis:
						break;
						case SVPropEditGeometry::RS_ZAxis:
						break;


						}
					}
					else {


//						if ( m_ui->toolButtonX->isChecked() )
//							rota.setRotation((float)m_ui->lineEditX->value(), m_cso->localXAxis());
//						else if ( m_ui->toolButtonY->isChecked() )
//							rota.setRotation((float)m_ui->lineEditY->value(), m_cso->localYAxis());
//						else if ( m_ui->toolButtonZ->isChecked() )
//							rota.setRotation((float)m_ui->lineEditZ->value(), m_cso->localZAxis());
					}

					// we take the QQuarternion to rotate
					QVector4D rotVec = rota.rotation().toVector4D();
					IBKMK::Vector3D center = QVector2IBKVector(m_cso->translation() );
					IBKMK::Vector3D newCenter = center;
					IBKMK::Quaternion centerRota((double) rotVec.w(), (double) rotVec.x(), (double) rotVec.y(), (double) rotVec.z());

					centerRota.rotateVector(newCenter);

					// we also have to find the center point after rotation and translate our center back to its origin
					rota.setTranslation(IBKVector2QVector(center - newCenter) );

					// we give our tranfsformation to the wire frame object
					SVViewStateHandler::instance().m_selectedGeometryObject->m_transform = rota;
					const_cast<Vic3D::SceneView*>(SVViewStateHandler::instance().m_geometryView->sceneView())->renderNow();
					const_cast<Vic3D::SceneView*>(SVViewStateHandler::instance().m_geometryView->sceneView())->renderLater();
				} break;
				case MS_Relative: {
					// now compose a transform object and set it in the wireframe object
					Vic3D::Transform3D rota;
					if (m_orientationMode == OM_Global) {
						if ( lineEdit == m_ui->lineEditX )
							rota.setRotation((float)m_ui->lineEditX->value(), 1, 0, 0);
						else if ( lineEdit == m_ui->lineEditY )
							rota.setRotation((float)m_ui->lineEditY->value(), 0, 1, 0);
						else if ( lineEdit == m_ui->lineEditZ )
							rota.setRotation((float)m_ui->lineEditZ->value(), 0, 0, 1);
					}
					else {
						if ( lineEdit == m_ui->lineEditX )
							rota.setRotation((float)m_ui->lineEditX->value(), m_cso->localXAxis());
						else if ( lineEdit == m_ui->lineEditY )
							rota.setRotation((float)m_ui->lineEditY->value(), m_cso->localYAxis());
						else if ( lineEdit == m_ui->lineEditZ )
							rota.setRotation((float)m_ui->lineEditZ->value(), m_cso->localZAxis());
					}


					// and then we also have to guarantee that the center point of the bounding box stays at the same position
					// this can be achieved since the center points are also just rotated by the specified rotation
					// so we know how big the absolute translation has to be
					QVector4D rotVec = rota.rotation().toVector4D();
					IBKMK::Vector3D newCenter = QVector2IBKVector(m_cso->translation() );
					IBKMK::Quaternion centerRota((double) rotVec.w(), (double) rotVec.x(), (double) rotVec.y(), (double) rotVec.z());
					centerRota.rotateVector(newCenter);

					// we also have to find the center point after rotation and translate our center back to its origin
					rota.setTranslation(IBKVector2QVector(m_bbCenter[m_orientationMode] - newCenter) );

					SVViewStateHandler::instance().m_selectedGeometryObject->m_transform = rota;
					const_cast<Vic3D::SceneView*>(SVViewStateHandler::instance().m_geometryView->sceneView())->renderNow();
					const_cast<Vic3D::SceneView*>(SVViewStateHandler::instance().m_geometryView->sceneView())->renderLater();
				} break;
			}
		} break;
	}
}


void SVPropEditGeometry::on_lineEditX_textChanged(const QString &) {
	onLineEditTextChanged(m_ui->lineEditX);
}

void SVPropEditGeometry::on_lineEditY_textChanged(const QString &) {
	onLineEditTextChanged(m_ui->lineEditY);
}

void SVPropEditGeometry::on_lineEditZ_textChanged(const QString &) {
	onLineEditTextChanged(m_ui->lineEditZ);
}


void SVPropEditGeometry::on_lineEditOrientation_textChanged(const QString &) {
	onLineEditTextChanged(m_ui->lineEditOrientation);
}

void SVPropEditGeometry::on_lineEditInclination_textChanged(const QString &) {
	onLineEditTextChanged(m_ui->lineEditInclination);
}

void SVPropEditGeometry::on_pushButtonCopySurfaces_clicked() {

	bool allSubSurfacesSelected = true;
	// check if all subsurfaces (if existing) of selected surfaces are selected as well
	for (const VICUS::Surface * s : m_selSurfaces) {
		for (const VICUS::SubSurface & sub : s->subSurfaces()) {
			if (!sub.m_selected) {
				allSubSurfacesSelected = false;
				break;
			}
		}
		if (!allSubSurfacesSelected)
			break;
	}

	// we get our copy method
	int copyAllSubSurfaces = 1;

	// if we have some invisible ones, ask user what to do
	if (!allSubSurfacesSelected)
		copyAllSubSurfaces = requestCopyOperation(dynamic_cast<QWidget *>(this), tr("Method for copying surfaces"),
			tr("Some of the surfaces have un-selected sub-surfaces. Copy them anyway?"),
			tr("Copy all"),								// returns 1
			tr("Copy only selected and visible") );		// returns 2

	if (copyAllSubSurfaces == -1)
		return;

	// now create a vector for the new surfaces
	std::vector<VICUS::Surface> newSurfaces;
	std::set<unsigned int> deselectedSurfaceUniqueIDs;

	// this map stores the old vs. new ID association, needed for copying component instance
	std::map<unsigned int, unsigned int> oldNewIDMap;

	unsigned int newID = project().nextUnusedID();

	// we go through all objects and find the hierarchy
	for (const VICUS::Surface * s : m_selSurfaces) {
		// remember ID of surface to be deselected
		deselectedSurfaceUniqueIDs.insert(s->m_id);

		// we make a copy of the surface but with a new unique ID
		VICUS::Surface newSurf(*s);
		newSurf.m_id = newID++;
		newSurf.m_selected = true; // select copied surface

		// NOTE: newSurf has a new unique ID, yet the copied subsurfaces still have the original IDs

		std::vector<VICUS::SubSurface> subs;
		for (std::vector<VICUS::SubSurface>::const_iterator it = newSurf.subSurfaces().begin();
			 it != newSurf.subSurfaces().end(); ++it)
		{
			// if we copy only selected, only add sub-surfaces that are selected
			if (copyAllSubSurfaces == 2 || it->m_selected) {
				const_cast<VICUS::SubSurface&>(*it).m_id = newID++; // give new ID
				const_cast<VICUS::SubSurface&>(*it).m_displayName = VICUS::uniqueName(it->m_displayName, m_subSurfNames);
				// remember old vs. new surface ID map
				oldNewIDMap[it->m_id] = it->m_id;

				subs.push_back(*it);							// keeps unqiue ID
			}
		}
		newSurf.setSubSurfaces(subs);

		// TODO Stephan, lookup "name generation" function
		// name generation
		// selected objects

		// we take our name set for this purpose and use the unique Name Function to take care of our new names
		newSurf.m_displayName = VICUS::uniqueName(s->m_displayName, m_surfNames );

		// now translate surface; no need to translate sub-surfaces, since they are embedded anyway
		std::vector<IBKMK::Vector3D> vertexes = newSurf.polygon3D().vertexes();
		for ( IBKMK::Vector3D &v : vertexes ) {
			v += m_translation;
		}
		newSurf.setPolygon3D( VICUS::Polygon3D(vertexes) );

		newSurfaces.push_back(newSurf);

		// remember old vs. new surface ID map
		oldNewIDMap[s->m_id] = newSurf.m_id;
	}

	// TODO Stephan, diskutieren mit Dirk -> Vermeiden von ComponentInstance-Kopien von Zwischenwänden

	// new component instances to be created
	std::vector<VICUS::ComponentInstance> newComponentInstances;
	std::vector<VICUS::SubSurfaceComponentInstance> newSubSurfaceComponentInstances;

	std::vector<unsigned int> compInstanceIDs;
	for (const VICUS::ComponentInstance & ci : project().m_componentInstances)
		compInstanceIDs.push_back(ci.m_id);

	// process all existing component instances
	for (const VICUS::ComponentInstance & ci : project().m_componentInstances) {
		// we create a copy of the component instance
		bool leftSideUsed =
				(ci.m_idSideASurface != VICUS::INVALID_ID &&
					 VICUS::contains(m_selSurfaces, ci.m_idSideASurface) );
		bool rightSideUsed =
				(ci.m_idSideBSurface != VICUS::INVALID_ID &&
					 VICUS::contains(m_selSurfaces, ci.m_idSideBSurface) );

		// skip unrelated component instances
		if (!leftSideUsed && !rightSideUsed)
			continue;

		// create copy of CI
		VICUS::ComponentInstance newCi;
		newCi.m_id = VICUS::uniqueId(compInstanceIDs);
		compInstanceIDs.push_back(newCi.m_id);
		newCi.m_idComponent = ci.m_idComponent;
		if (leftSideUsed)
			newCi.m_idSideASurface = oldNewIDMap[ci.m_idSideASurface];
		if (rightSideUsed)
			newCi.m_idSideBSurface = oldNewIDMap[ci.m_idSideBSurface];
		newComponentInstances.push_back(newCi);
	}

	// TODO Stephan, also copy sub-surface component instances
	// process all existing component instances
	for (const VICUS::SubSurfaceComponentInstance & ci : project().m_subSurfaceComponentInstances) {
		// we create a copy of the component instance
		bool leftSideUsed =
				(ci.m_idSideASurface != VICUS::INVALID_ID &&
					( VICUS::contains(m_selSubSurfaces, ci.m_idSideASurface) ) );
		bool rightSideUsed =
				(ci.m_idSideBSurface != VICUS::INVALID_ID &&
					( VICUS::contains(m_selSubSurfaces, ci.m_idSideBSurface) ) );

		// skip unrelated component instances
		if (!leftSideUsed && !rightSideUsed)
			continue;

		// create copy of CI
		VICUS::SubSurfaceComponentInstance newCi;
		newCi.m_id = VICUS::uniqueId(compInstanceIDs);
		compInstanceIDs.push_back(newCi.m_id);
		newCi.m_idSubSurfaceComponent = ci.m_idSubSurfaceComponent;
		if (leftSideUsed)
			newCi.m_idSideASurface = oldNewIDMap[ci.m_idSideASurface];
		if (rightSideUsed)
			newCi.m_idSideBSurface = oldNewIDMap[ci.m_idSideBSurface];
		newSubSurfaceComponentInstances.push_back(newCi);
	}

	SVUndoCopySurfaces *undo = new SVUndoCopySurfaces("Copied Surfaces.", newSurfaces, deselectedSurfaceUniqueIDs,
													  newComponentInstances, newSubSurfaceComponentInstances);
	undo->push();
}


void SVPropEditGeometry::on_pushButtonCopyRooms_clicked() {
	/// We now also want to copy rooms
	///	Therefore we first check which rooms are checked
	///	Then we also have to check if all surfaces are checked
	/// same for the subsurfaces

	bool allSurfacesSelected = true;
	// check if all subsurfaces (if existing) of selected surfaces are selected as well
	for (const VICUS::Room * r : m_selRooms) {
		for (const VICUS::Surface & s : r->m_surfaces ) {
			if (!s.m_selected) {
				allSurfacesSelected = false;
				break;
			}
			for (const VICUS::SubSurface &sub : s.subSurfaces() ) {
				if (!sub.m_selected) {
					allSurfacesSelected = false;
					break;
				}
			}
		}
		if (!allSurfacesSelected)
			break;
	}

	// we get our copy method
	int copyAllSurfaces = 1;

	// if we have some invisible ones, ask user what to do
	if (!allSurfacesSelected)
		copyAllSurfaces = requestCopyOperation(dynamic_cast<QWidget *>(this), tr("Method for copying rooms"),
			tr("Some of the rooms have un-selected surfaces or sub-surfaces. Copy them anyway?"),
			tr("Copy all"),								// returns 1
			tr("Copy only selected and visible") );		// returns 2

	if (copyAllSurfaces == -1)
		return;

	// now create a vector for the new surfaces
	std::vector<VICUS::Room> newRooms;
	std::set<unsigned int> deselectedUniqueIDs;

	// this map stores the old vs. new ID association, needed for copying component instances
	std::map<unsigned int, unsigned int> oldNewIDMap;

	// get next free unqiue ID
	unsigned int newID = project().nextUnusedID();

	for ( const VICUS::Room *r : m_selRooms ) {

		// remeber IDs so that we can deselct the rooms
		deselectedUniqueIDs.insert(r->m_id);

		// we make a copy of the room but with a new unique ID
		VICUS::Room newRoom(*r);
		newRoom.m_surfaces.clear(); // we add surfaces back below
		newRoom.m_displayName = VICUS::uniqueName(r->m_displayName, m_roomNames);

		newRoom.m_id = newID++;
		newRoom.m_selected = true; // select copied surface

		// if we copy only selected, remove all sub-surfaces that are not selected
		std::vector<VICUS::Surface> surfs;
		for (const VICUS::Surface & surf : newRoom.m_surfaces) {
			// skip if neither selected and not "Copy All" mode
			if (copyAllSurfaces != 2 && !surf.m_selected) continue;

			// remember ID so that we can deselect it
			deselectedUniqueIDs.insert(surf.m_id);

			// create a copy of the surface and give it a new unique ID
			VICUS::Surface newSurf(surf);
			newSurf.m_id = newID++;
			// TODO: Stephan, select suitable unique name for subsurface

			// NOTE: we replace the subsurfaces below

			// remember old vs. new surface ID map
			oldNewIDMap[surf.m_id] = newSurf.m_id;

			std::vector<VICUS::SubSurface> subs;
			// process all copied subsurfaces and only keep selected (unless "Copy all" was used)
			for (const VICUS::SubSurface & sub : surf.subSurfaces()) {
				// skip if neither selected and not "Copy All" mode
				if (copyAllSurfaces != 2 && !sub.m_selected) continue;

				// remember ID so that we can deselect it
				deselectedUniqueIDs.insert(sub.m_id);

				// create a copy of the subsurface
				VICUS::SubSurface newSubSurf(sub);
				newSubSurf.m_id = newID++;
				// TODO : Stephan, select suitable unique name for subsurface

				// remember old vs. new surface ID map
				oldNewIDMap[sub.m_id] = newSubSurf.m_id;

				subs.push_back(newSubSurf);
			}
			// replace sub-surfaces in copied surface
			newSurf.setSubSurfaces(subs);

			// now translate surface vertexes,
			// no need to translate sub-surfaces, since they are embedded anyway
			std::vector<IBKMK::Vector3D> vertexes = newSurf.polygon3D().vertexes();
			for ( IBKMK::Vector3D &v : vertexes ) {
				v += m_translation;
			}
			newSurf.setPolygon3D( VICUS::Polygon3D(vertexes) );

			surfs.push_back(newSurf);
		}
		newRoom.m_surfaces = surfs;

		// TODO Stephan, lookup "name generation" function
		// name generation
		// selected objects

		// we take our name set for this purpose and use the unique Name Function to take care of our new names
		newRoom.m_displayName = VICUS::uniqueName(r->m_displayName, m_roomNames );

		newRooms.push_back(newRoom);
	}

	std::vector<VICUS::ComponentInstance> newComponentInstances;
	std::vector<VICUS::SubSurfaceComponentInstance> newSubSurfaceComponentInstances;

	std::vector<unsigned int> compInstanceIDs;
	for (const VICUS::ComponentInstance & ci : project().m_componentInstances)
		compInstanceIDs.push_back(ci.m_id);

	// process all existing component instances
	for (const VICUS::ComponentInstance & ci : project().m_componentInstances) {
		// we create a copy of the component instance
		bool leftSideUsed =
				(ci.m_idSideASurface != VICUS::INVALID_ID &&
					 VICUS::contains(m_selSurfaces, ci.m_idSideASurface) );
		bool rightSideUsed =
				(ci.m_idSideBSurface != VICUS::INVALID_ID &&
					 VICUS::contains(m_selSurfaces, ci.m_idSideBSurface) );

		// skip unrelated component instances
		if (!leftSideUsed && !rightSideUsed)
			continue;

		// create copy of CI
		VICUS::ComponentInstance newCi;
		newCi.m_id = VICUS::uniqueId(compInstanceIDs);
		compInstanceIDs.push_back(newCi.m_id);
		newCi.m_idComponent = ci.m_idComponent;
		if (leftSideUsed)
			newCi.m_idSideASurface = oldNewIDMap[ci.m_idSideASurface];
		if (rightSideUsed)
			newCi.m_idSideBSurface = oldNewIDMap[ci.m_idSideBSurface];
		newComponentInstances.push_back(newCi);
	}

	// TODO Stephan, also copy sub-surface component instances
	// process all existing component instances
	for (const VICUS::SubSurfaceComponentInstance & ci : project().m_subSurfaceComponentInstances) {
		// we create a copy of the component instance
		bool leftSideUsed =
				(ci.m_idSideASurface != VICUS::INVALID_ID &&
					( VICUS::contains(m_selSubSurfaces, ci.m_idSideASurface) ) );
		bool rightSideUsed =
				(ci.m_idSideBSurface != VICUS::INVALID_ID &&
					( VICUS::contains(m_selSubSurfaces, ci.m_idSideBSurface) ) );

		// skip unrelated component instances
		if (!leftSideUsed && !rightSideUsed)
			continue;

		// create copy of CI
		VICUS::SubSurfaceComponentInstance newCi;
		newCi.m_id = VICUS::uniqueId(compInstanceIDs);
		compInstanceIDs.push_back(newCi.m_id);
		newCi.m_idSubSurfaceComponent = ci.m_idSubSurfaceComponent;
		if (leftSideUsed)
			newCi.m_idSideASurface = oldNewIDMap[ci.m_idSideASurface];
		if (rightSideUsed)
			newCi.m_idSideBSurface = oldNewIDMap[ci.m_idSideBSurface];
		newSubSurfaceComponentInstances.push_back(newCi);
	}

	SVUndoCopyZones *undo = new SVUndoCopyZones("Copied Zones.", newRooms, deselectedUniqueIDs,
														newComponentInstances, newSubSurfaceComponentInstances);
	undo->push();

}

void SVPropEditGeometry::on_pushButtonAdd_clicked() {
	setCurrentPage(O_AddGeometry);
	updateCoordinateSystemLook();
}

void SVPropEditGeometry::on_pushButtonEdit_clicked() {
	setCurrentPage(O_EditGeometry);
	updateCoordinateSystemLook();
}


void SVPropEditGeometry::on_pushButtonThreePointRotation_clicked() {
	// when clicked, we set the scene into three-point-rotation mode
	// TODO Stephan
}

void SVPropEditGeometry::on_pushButtonFlipNormals_clicked() {
	// compose vector of modified surface geometries
	std::vector<VICUS::Surface> modifiedSurfaces;

	// flip all plane geometries
	for (const VICUS::Object * o : SVViewStateHandler::instance().m_selectedGeometryObject->m_selectedObjects) {
		// handle surfaces
		const VICUS::Surface * s = dynamic_cast<const VICUS::Surface *>(o);
		if (s != nullptr) {
			VICUS::Surface modS(*s);
			modS.flip();
			if (modS.geometry().isValid() )
				modifiedSurfaces.push_back(modS);

			else{
				// heal by stephan
				modS.healGeometry(modS.geometry().polygon().vertexes());
				if (!modS.geometry().isValid() )
					IBK::IBK_Message(IBK::FormatString("Surface %1 could not be flipped.").arg(modS.m_displayName.toStdString()),
								 IBK::MSG_WARNING, "Surface::flip", IBK::VL_STANDARD);
				else
					modifiedSurfaces.push_back(modS);
			}
		}
	}


	// in case operation was executed without any selected objects - should be prevented
	if (modifiedSurfaces.empty())
		return;

	// create undo-action
	SVUndoModifySurfaceGeometry * undo = new SVUndoModifySurfaceGeometry(tr("Flipped normal vectors"), modifiedSurfaces );
	undo->push();
	// reset local transformation matrix
	SVViewStateHandler::instance().m_selectedGeometryObject->m_transform = Vic3D::Transform3D();
}

void SVPropEditGeometry::on_toolButtonLocalCoordinateOrientation_clicked(bool checked) {
	// we set the state of our local coordinate system
	m_orientationMode = checked ? OM_Local : OM_Global;

	// we update all the specific parameters
	updateOrientationMode();
}

void SVPropEditGeometry::on_pushButtonCopySubSurfaces_clicked() {

	// now create a vector for the new surfaces
	std::vector<VICUS::SubSurface> newSubSurfaces;
	std::set<unsigned int> deselectedSubSurfaceUniqueIDs;

	// this map stores the old vs. new ID association, needed for copying component instance
	std::map<unsigned int, unsigned int> oldNewIDMap;


	for ( unsigned int i = 0; i<m_selSubSurfaces.size(); ++i ) {

		// we now copy our sub surface --> means we copy the object itself
		// and we update the 2D Geometry of it
		// also we have to update the sub surface component instance
		// There you go then

		unsigned int newID = project().nextUnusedID();

		const VICUS::SubSurface &subSurf = *m_selSubSurfaces[i];

		deselectedSubSurfaceUniqueIDs.insert(subSurf.m_id);

		VICUS::SubSurface clonedSubSurf = subSurf; // clone the object with an unique ID
		clonedSubSurf.m_id = newID++;

		clonedSubSurf.m_displayName = VICUS::uniqueName(subSurf.m_displayName, m_subSurfNames); // get a unique name back
		// we also have to take care that we have to change the id (not unique id) of the surface
		// for now I assume that I can take the unique id as the id
		// needed for component instances
		clonedSubSurf.m_selected = true; // select copied surface

		// update 2D Coordinates
		// How are we going to do this?
		Q_ASSERT(clonedSubSurf.m_parent != nullptr);

		oldNewIDMap[subSurf.m_id] = clonedSubSurf.m_id;

		const VICUS::Surface *surf = dynamic_cast<VICUS::Surface*>(clonedSubSurf.m_parent);

		IBKMK::Vector3D offset3d = dynamic_cast<VICUS::Surface*>(clonedSubSurf.m_parent)->geometry().polygon().vertexes()[0];
		IBKMK::Vector2D offset2d = clonedSubSurf.offset();

		std::vector<IBKMK::Vector2D> newVertexes (subSurf.m_polygon2D.vertexes().size() );

		for ( unsigned int j = 0; j<clonedSubSurf.m_polygon2D.vertexes().size(); ++j ) {
			IBKMK::Vector2D v = clonedSubSurf.m_polygon2D.vertexes()[j];

			// first we calc the 3D Point
			// then we transform it
			// and finally we project it onto our plane
			// so that our window is always in our plane

			// 1) Calc 3D Point
			IBKMK::Vector3D p = offset3d	+ v.m_x * surf->geometry().localX()
											+ v.m_y * surf->geometry().localY()	+ m_translation;
			IBKMK::planeCoordinates(offset3d, surf->geometry().localX(), surf->geometry().localY(), p, newVertexes[j].m_x, newVertexes[j].m_y);

		}

		clonedSubSurf.m_polygon2D.setVertexes(newVertexes);


		newSubSurfaces.push_back(clonedSubSurf);

	}

	std::vector<VICUS::SubSurfaceComponentInstance> newSubSurfaceComponentInstances;
	std::vector<unsigned int> compInstanceIDs;
	// TODO Stephan, also copy sub-surface component instances
	// process all existing component instances
	for (const VICUS::SubSurfaceComponentInstance & ci : project().m_subSurfaceComponentInstances) {
		// we create a copy of the component instance
		bool leftSideUsed =
				(ci.m_idSideASurface != VICUS::INVALID_ID &&
					( VICUS::contains(m_selSubSurfaces, ci.m_idSideASurface) ) );
		bool rightSideUsed =
				(ci.m_idSideBSurface != VICUS::INVALID_ID &&
					( VICUS::contains(m_selSubSurfaces, ci.m_idSideBSurface) ) );

		// skip unrelated component instances
		if (!leftSideUsed && !rightSideUsed)
			continue;

		// create copy of CI
		VICUS::SubSurfaceComponentInstance newCi;
		newCi.m_id = VICUS::uniqueId(compInstanceIDs);
		compInstanceIDs.push_back(newCi.m_id);
		newCi.m_idSubSurfaceComponent = ci.m_idSubSurfaceComponent;
		if (leftSideUsed)
			newCi.m_idSideASurface = oldNewIDMap[ci.m_idSideASurface];
		if (rightSideUsed)
			newCi.m_idSideBSurface = oldNewIDMap[ci.m_idSideBSurface];
		newSubSurfaceComponentInstances.push_back(newCi);
	}

	SVUndoCopySubSurfaces *undo = new SVUndoCopySubSurfaces("Copied Sub Surfaces.", newSubSurfaces, deselectedSubSurfaceUniqueIDs, newSubSurfaceComponentInstances);
	undo->push();
}


void SVPropEditGeometry::on_toolButtonAbs_clicked(bool /*checked*/) {

	// set new state
	m_modificationState[m_modificationType] = ModificationState::MS_Absolute;
	// update tool buttons
	setToolButtonAbsMode();
	// now update inputs
	updateInputs();
}

void SVPropEditGeometry::on_toolButtonRel_clicked(bool /*checked*/) {

	m_ui->toolButtonAbs->setChecked(false);
	// set new state
	m_modificationState[m_modificationType] = ModificationState::MS_Relative;
	// update tool buttons
	setToolButtonAbsMode();
	// now update inputs
	updateInputs();
}

void SVPropEditGeometry::on_toolButtonNormal_clicked() {
	m_rotationState = RS_Normal;
	setToolButtonsRotationState(m_modificationState[m_modificationType] == MS_Absolute);
}


void SVPropEditGeometry::on_toolButtonX_clicked() {
	m_rotationState = RS_XAxis;
	setToolButtonsRotationState(m_modificationState[m_modificationType] == MS_Absolute);
	if (m_orientationMode == OM_Local)
		setRotation( QVector2IBKVector(m_cso->localXAxis() ) );
	else
		setRotation( IBKMK::Vector3D(1,0,0));
}

void SVPropEditGeometry::on_toolButtonY_clicked() {
	m_rotationState = RS_YAxis;
	setToolButtonsRotationState(m_modificationState[m_modificationType] == MS_Absolute);
	if (m_orientationMode == OM_Local)
		setRotation( QVector2IBKVector(m_cso->localYAxis() ) );
	else
		setRotation( IBKMK::Vector3D(0,1,0));
}

void SVPropEditGeometry::on_toolButtonZ_clicked() {
	m_rotationState = RS_ZAxis;
	setToolButtonsRotationState(m_modificationState[m_modificationType] == MS_Absolute);

	if (m_orientationMode == OM_Local)
		setRotation( QVector2IBKVector(m_cso->localZAxis() ) );
	else
		setRotation( IBKMK::Vector3D(0,0,1));
}

void SVPropEditGeometry::on_pushButtonCenteHorizontal_clicked(){

	/// first we take the selection of all objects and let the bounding box calculate
	/// our new center point for all objects. Alternative would be the center point
	/// of all individual bounding boxes. but we might do this later
	///
	/// Then we take all selected objects and we calculate the local difference between
	/// the new center point (only z-value) and the old z-value. This is our local
	/// z-value for the translation. We then translate all vertexes of the surface
	///
	/// We have to differentiate between surfaces and sub surfaces
	IBKMK::Vector3D center, oldCenter;
	IBKMK::Vector3D bb = project().boundingBox(m_selSurfaces, m_selSubSurfaces, center);

	std::vector<VICUS::Surface>			modifiedSurfaces;
	std::set<const VICUS::Surface*>		handledSurfaces;

	std::vector<const VICUS::Surface*>		emptySurfs;
	std::vector<const VICUS::SubSurface*>	emptySubSurfs;

	for(const VICUS::Surface *surf : m_selSurfaces) {
		std::vector<const VICUS::Surface*> surfs;
		surfs.push_back(surf);
		IBKMK::Vector3D bbOld = project().boundingBox(surfs, emptySubSurfs, oldCenter);

		IBKMK::Vector3D translation (0,0,center.m_z - oldCenter.m_z);

		// we have now our translation
		std::vector<IBKMK::Vector3D> vertexes = surf->polygon3D().vertexes();
		for ( IBKMK::Vector3D & v : vertexes ) {
			// use just this instead of making a QVetor3D
			v += translation;
		}
		VICUS::Surface modS(*surf);
		modS.setPolygon3D( VICUS::Polygon3D(vertexes) );

		if(!modS.geometry().isValid())
			modS.healGeometry(vertexes);

		modifiedSurfaces.push_back(modS);
	}
	for(const VICUS::SubSurface *subSurf : m_selSubSurfaces) {

		// we keep our original surface
		VICUS::Surface *parentSurf = dynamic_cast<VICUS::Surface*>(subSurf->m_parent);
		if (parentSurf != nullptr && parentSurf->m_selected && parentSurf->m_visible)
			continue; //

		if(handledSurfaces.find(parentSurf) != handledSurfaces.end())
			continue; // surface already handled

		// parentSurf->m_selected = true; // !!!! only for now till we adjust the function !!!
		VICUS::Surface modS(*parentSurf);

		// we cache our poldon data
		IBKMK::Vector3D offset3d = modS.geometry().offset();
		const IBKMK::Vector3D &localX = modS.geometry().localX();
		const IBKMK::Vector3D &localY = modS.geometry().localY();

		std::vector<VICUS::SubSurface> newSubSurfs(modS.subSurfaces().size() );
		// now we also have to scale the sub surfaces
		for ( unsigned int i = 0; i<modS.subSurfaces().size(); ++i ) {

			if (!modS.subSurfaces()[i].m_selected || !modS.subSurfaces()[i].m_visible)
				continue; // skip deselected surfaces

			VICUS::SubSurface &subS = const_cast<VICUS::SubSurface &>(modS.subSurfaces()[i]);
			std::vector<const VICUS::SubSurface*> subSurfs;
			subSurfs.push_back(&subS);
			IBKMK::Vector3D bbOld = project().boundingBox(emptySurfs, subSurfs, oldCenter);

			IBKMK::Vector3D translation (0,0,center.m_z - oldCenter.m_z);

			qDebug() << "0\t0\t" << translation.m_z;

			Q_ASSERT(modS.subSurfaces().size() == modS.geometry().holes().size());
			newSubSurfs[i] = modS.subSurfaces()[i];

			// we only modify our selected sub surface
			std::vector<IBKMK::Vector2D> newSubSurfVertexes (subS.m_polygon2D.vertexes().size());

			for ( unsigned int j=0; j<subS.m_polygon2D.vertexes().size(); ++j ) {
					IBKMK::Vector2D v2D = subS.m_polygon2D.vertexes()[j];

					// we now calculate the 3D points of the sub surface
					// afterwards we scale up the surface
					IBKMK::Vector3D v3D = offset3d + localX * v2D.m_x + localY * v2D.m_y;

					Vic3D::Transform3D t;
					t.translate(IBKVector2QVector(translation) );
					t.setTranslation(t.toMatrix()*IBKVector2QVector(v3D) );
					v3D = QVector2IBKVector(t.translation() );

					// and we calculate back the projection on the plane
					// we have to take the offset of our new scaled polygon
					IBKMK::planeCoordinates(modS.geometry().offset(), localX, localY, v3D, newSubSurfVertexes[j].m_x, newSubSurfVertexes[j].m_y);

			}
			newSubSurfs[i].m_polygon2D = newSubSurfVertexes;
		}
		// we cache that we already handled all selected sub surfaces of the surface
		// handledSurfaces.insert(parentSurf);

		// we update the 2D polyline
		modS.setSubSurfaces(newSubSurfs);
		handledSurfaces.insert(parentSurf);

		modifiedSurfaces.push_back(modS);
	}

	// in case operation was executed without any selected objects - should be prevented
	if (modifiedSurfaces.empty())
		return;

	SVUndoModifySurfaceGeometry * undoSurf = new SVUndoModifySurfaceGeometry(tr("Translated surface geometry"), modifiedSurfaces );
	undoSurf->push();

}

