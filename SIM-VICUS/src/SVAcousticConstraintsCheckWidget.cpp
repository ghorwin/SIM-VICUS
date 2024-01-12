#include "SVAcousticConstraintsCheckWidget.h"
#include "ui_SVAcousticConstraintsCheckWidget.h"

#include "SVStyle.h"
#include "SVProjectHandler.h"
#include "QtExt_Conversions.h"
#include "SVUndoTreeNodeState.h"
#include "SVViewStateHandler.h"
#include "SVGeometryView.h"
#include "Vic3DSceneView.h"

#include <VICUS_Project.h>

SVAcousticConstraintsCheckWidget::SVAcousticConstraintsCheckWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVAcousticConstraintsCheckWidget)
{
	m_ui->setupUi(this);

	// set up table widgets

	// for walls
	m_ui->tableWidgetAirBorneSound->setColumnCount(8);
	m_ui->tableWidgetAirBorneSound->setHorizontalHeaderLabels(QStringList()
													  << tr("Acoustic template A")
													  << tr("Acoustic Template B")
													  << tr("Acoustic component")
													  << tr("Actual air sound value")
													  << tr("Normal constraints")
													  << tr("Advanced constraints")
													  << tr("Structure type")
													  << "");
	SVStyle::formatDatabaseTableView(m_ui->tableWidgetAirBorneSound);
	m_ui->tableWidgetAirBorneSound->setSortingEnabled(false);
	m_ui->tableWidgetAirBorneSound->resizeColumnsToContents();


	//for ceilings
	m_ui->tableWidgetImpactSound->setColumnCount(8);
	m_ui->tableWidgetImpactSound->setHorizontalHeaderLabels(QStringList()
														 << tr("Acoustic template above")
														 << tr("Acoustic Template beneth")
														 << tr("Acoustic component")
														 << tr("Actual impact sound value")
														 << tr("Normal constraints")
														 << tr("Advanced constraints")
														 << tr("Structure type")
														 << "");

	SVStyle::formatDatabaseTableView(m_ui->tableWidgetImpactSound);

	m_ui->tableWidgetImpactSound->setSortingEnabled(false);
	m_ui->tableWidgetReverberation->setSortingEnabled(false);
	m_ui->tableWidgetImpactSound->resizeColumnsToContents();

//	double width = 130;
//	m_ui->tableWidgetImpactSound->setColumnWidth(CSP_AcousticTemplateB, 200);
//	m_ui->tableWidgetImpactSound->setColumnWidth(CSP_AcousticComponent, width);
//	m_ui->tableWidgetImpactSound->setColumnWidth(CSP_ActualAirSoundValue, width);
//	m_ui->tableWidgetImpactSound->setColumnWidth(CSP_AdvancedConstraints, width);
//	m_ui->tableWidgetImpactSound->setColumnWidth(CSP_NormalConstraints, width);
//	m_ui->tableWidgetImpactSound->setColumnWidth(CSP_SameStructure, width);
//	m_ui->tableWidgetImpactSound->setColumnWidth(CSP_SelectButton, 230);

//	m_ui->tableWidgetAirBorneSound->setColumnWidth(CSP_AcousticTemplateB, 200);
//	m_ui->tableWidgetAirBorneSound->setColumnWidth(CSP_AcousticComponent, width);
//	m_ui->tableWidgetAirBorneSound->setColumnWidth(CSP_ActualAirSoundValue, width);
//	m_ui->tableWidgetAirBorneSound->setColumnWidth(CSP_AdvancedConstraints, width);
//	m_ui->tableWidgetAirBorneSound->setColumnWidth(CSP_NormalConstraints, width);
//	m_ui->tableWidgetAirBorneSound->setColumnWidth(CSP_SameStructure, width);
//	m_ui->tableWidgetAirBorneSound->setColumnWidth(CSP_SelectButton, 230);

//	QHeaderView *headerView = m_ui->tableWidgetImpactSound->horizontalHeader();
//	headerView->setSectionResizeMode(CSP_AcousticTemplateA, QHeaderView::Stretch);

//	headerView = m_ui->tableWidgetAirBorneSound->horizontalHeader();
//	headerView->setSectionResizeMode(CSP_AcousticTemplateA, QHeaderView::Stretch);

//	m_ui->tableWidgetAirBorneSound->horizontalHeaderItem(CSP_AcousticTemplateA)->setTextAlignment(Qt::AlignLeft);
//	m_ui->tableWidgetAirBorneSound->horizontalHeaderItem(CSP_AcousticTemplateB)->setTextAlignment(Qt::AlignLeft);

//	m_ui->tableWidgetImpactSound->horizontalHeaderItem(CSP_AcousticTemplateA)->setTextAlignment(Qt::AlignLeft);
//	m_ui->tableWidgetImpactSound->horizontalHeaderItem(CSP_AcousticTemplateB)->setTextAlignment(Qt::AlignLeft);

	m_ui->checkBoxHideAirBourneSound->blockSignals(true);
	m_ui->checkBoxHideImpactSound->blockSignals(true);

	m_ui->checkBoxHideAirBourneSound->setChecked(false);
	m_ui->checkBoxHideImpactSound->setChecked(false);

	m_ui->checkBoxHideImpactSound->blockSignals(false);
	m_ui->checkBoxHideAirBourneSound->blockSignals(false);

	resize(1500, 800);

	m_ui->tableWidgetReverberation->setColumnCount(NUM_CRT);

	QStringList headers;
	headers << "Room ID"
			<< "Room name"
			<< "Acoustic template"
			<< "Reverberation goal [s]"
			<< "125 Hz [s]"
			<< "250 Hz [s]"
			<< "500 Hz [s]"
			<< "1000 Hz [s]"
			<< "2000 Hz [s]"
			<< "4000 Hz [s]";

	m_ui->tableWidgetReverberation->setHorizontalHeaderLabels(headers);
	SVStyle::formatDatabaseTableView(m_ui->tableWidgetReverberation);
	setWindowTitle(tr("Acoustic component check"));

	m_ui->tabWidget->setEnabled(false);
	m_ui->tabWidget->setCurrentIndex(0);
}


SVAcousticConstraintsCheckWidget::~SVAcousticConstraintsCheckWidget() {
	delete m_ui;
}


void SVAcousticConstraintsCheckWidget::showSurfaces(unsigned int surfaceAId, unsigned int surfaceBId) {
	std::set<unsigned int> surfIds;
	surfIds.insert(surfaceAId);
	surfIds.insert(surfaceBId);

	std::set<const VICUS::Object *> allObjects;
	project().selectObjects(allObjects, VICUS::Project::SG_All, false, true);
	std::set<unsigned int> selectedObjectIDs;
	for (const VICUS::Object * o : allObjects) {
		if (o->m_selected)
			selectedObjectIDs.insert(o->m_id);
	}

	SVUndoTreeNodeState * undo = new SVUndoTreeNodeState(tr("De-selecting objects"), SVUndoTreeNodeState::SelectedState, selectedObjectIDs, false);
	undo->push();
	undo = new SVUndoTreeNodeState(tr("Selecting objects"), SVUndoTreeNodeState::SelectedState, surfIds, true);
	undo->push();

	SVViewStateHandler::instance().m_geometryView->resetCamera(Vic3D::SceneView::CP_FindSelection);
}


void SVAcousticConstraintsCheckWidget::checkConstraints() {
	// ToDo Stephan: Proper result handling and viewing of lca results
	QMessageBox::information((QWidget *)this, tr("Acoustic calculation"), tr("This is an upcoming professional feature coming in Q1/2024."));
}

void SVAcousticConstraintsCheckWidget::checkReverberation() {

}


void SVAcousticConstraintsCheckWidget::updateTable() {


}


void SVAcousticConstraintsCheckWidget::on_pushButtonCheckConstraints_clicked() {
	checkConstraints();

	// m_ui->tabWidget->setEnabled(true);
}


void SVAcousticConstraintsCheckWidget::on_checkBoxHideAirBourneSound_stateChanged(int hideAirBourneSound) {
	m_hideAirBourneSound = (bool)hideAirBourneSound;
	// render constraints again
	updateTable();
}


void SVAcousticConstraintsCheckWidget::on_checkBoxHideImpactSound_stateChanged(int hideImpactSound) {
	m_hideImpactSound = (bool)hideImpactSound;
	// render constraints again
	updateTable();
}


