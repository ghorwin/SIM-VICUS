#include "SVSmartIntersectionDialog.h"
#include "SVDatabaseEditDialog.h"
#include "SVMainWindow.h"
#include "ui_SVSmartIntersectionDialog.h"

#include <IBK_MessageHandlerRegistry.h>

#include <QtExt_Conversions.h>

#include <QScrollBar>
#include <QMessageBox>
#include <QProgressDialog>

#include "SVProjectHandler.h"
#include "SVSettings.h"
#include "SVUndoModifyBuildingTopology.h"


class SmartClippingProgress : public RC::Notification {
public:
	void notify() override {}
	void notify(double percentage) override;

	char				pad[7]; // fix padding, silences compiler warning
	QProgressDialog		*m_prgDlg = nullptr;
};

void SmartClippingProgress::notify(double percentage) {
	m_prgDlg->setValue((int)(m_prgDlg->maximum() * percentage));
	qApp->processEvents();
	if (m_prgDlg->wasCanceled())
		m_aborted = true;
}


SVSmartIntersectionDialog::SVSmartIntersectionDialog(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::SVSmartIntersectionDialog)
{
	m_ui->setupUi(this);

	m_maxAngle = 5;
	m_maxDistance = 0.5;

	m_ui->lineEditConnectSurfacesMaxDistance->setup(0, 10000, tr("Maximum distances between surfaces to enable connection in [m]."),
													false, true);
	m_ui->lineEditConnectSurfacesMaxDistance->setValue(m_maxDistance);
	m_ui->lineEditConnectSurfacesMaxAngle->setup(0, 45, tr("Maximum angle between surfaces to enable connection in [Deg]."),
												 false, true);
	m_ui->lineEditConnectSurfacesMaxAngle->setValue(m_maxAngle);

	m_db = &SVSettings::instance().m_db;

	unsigned int id = m_db->m_components.begin()->second.m_id;

	m_predefinedComponentIds[RC::VicusClipper::PDC_Ceiling]			= id;
	m_predefinedComponentIds[RC::VicusClipper::PDC_ExteriorWall]	= id;
	m_predefinedComponentIds[RC::VicusClipper::PDC_Floor]			= id;
	m_predefinedComponentIds[RC::VicusClipper::PDC_Roof]			= id;
	m_predefinedComponentIds[RC::VicusClipper::PDC_InteriorWall]	= id;

	updateUi();
}


SVSmartIntersectionDialog::~SVSmartIntersectionDialog()
{
	delete m_ui;
}


SVSmartIntersectionDialog::ClippingResults SVSmartIntersectionDialog::clipProject() {
	m_ui->pushButtonApply->setEnabled(false);
	m_ui->plainTextEdit->clear();

	int res = exec();
	if (res == QDialog::Rejected)
		return CancelledClipping;

	return m_returnCode;
}


SVClippingMessageHandler::SVClippingMessageHandler(QObject *parent, QPlainTextEdit *plainTextEdit) :
	QObject(parent),
	m_plainTextEdit(plainTextEdit)
{
	m_defaultMsgHandler = dynamic_cast<SVMessageHandler *>(IBK::MessageHandlerRegistry::instance().messageHandler());
	Q_ASSERT(m_defaultMsgHandler != nullptr);
	IBK::MessageHandlerRegistry::instance().setMessageHandler(this);
	m_plainTextEdit->clear();
}


SVClippingMessageHandler::~SVClippingMessageHandler() {
	IBK::MessageHandlerRegistry::instance().setMessageHandler(m_defaultMsgHandler);
}


void SVClippingMessageHandler::msg(const std::string& msg,
								   IBK::msg_type_t t,
								   const char * func_id,
								   int verbose_level)
{
	if (msg.empty())
		return;
	std::string msg2;
	if (msg[msg.size()-1] == '\n')
		msg2 = msg.substr(0,msg.size()-1);
	else
		msg2 = msg;

	switch (t) {
	case IBK::MSG_WARNING :
		msg2 = "<span style=\"color:#e0c000\">" + msg2 + "</span><br>";
		m_plainTextEdit->appendHtml(QString::fromStdString(msg2));
		break;

	case IBK::MSG_ERROR :
		msg2 = "<span style=\"color:#d00000\">" + msg2 + "</span><br>";
		m_plainTextEdit->appendHtml(QString::fromStdString(msg2));
		break;

	default:
		m_plainTextEdit->appendPlainText(QString::fromStdString(msg2));
	}
	m_plainTextEdit->verticalScrollBar()->setValue(m_plainTextEdit->verticalScrollBar()->maximum());

	m_defaultMsgHandler->msg(msg, t, func_id, verbose_level);
	if (verbose_level > m_requestedConsoleVerbosityLevel)
		return;
}


void SVSmartIntersectionDialog::on_pushButtonStartClipping_clicked() {

	// check input
	if (!m_ui->lineEditConnectSurfacesMaxDistance->isValid()) {
		QMessageBox::critical(this, QString(), tr("Please enter valid parameters!"));
		m_ui->lineEditConnectSurfacesMaxDistance->selectAll();
		m_ui->lineEditConnectSurfacesMaxDistance->setFocus();
		return;
	}
	// check input
	if (!m_ui->lineEditConnectSurfacesMaxAngle->isValid()) {
		QMessageBox::critical(this, QString(), tr("Please enter valid parameters!"));
		m_ui->lineEditConnectSurfacesMaxAngle->selectAll();
		m_ui->lineEditConnectSurfacesMaxAngle->setFocus();
		return;
	}
	double maxDist = m_ui->lineEditConnectSurfacesMaxDistance->value();
	double maxAngle = m_ui->lineEditConnectSurfacesMaxAngle->value();

	SVClippingMessageHandler msgHandler(this, m_ui->plainTextEdit);

	const VICUS::Project &prj = SVProjectHandler::instance().project();
	m_componentInstances = std::vector<VICUS::ComponentInstance>();
	m_subSurfaceComponentInstances = std::vector<VICUS::SubSurfaceComponentInstance>();

	SmartClippingProgress progressNotifyer;
	progressNotifyer.m_prgDlg = new QProgressDialog(tr("Smart-Clipping"), tr("Cancel"), 0, 100, this);

	m_ui->groupBoxConnectSurfaces->setEnabled(false);
	m_ui->groupBoxProjectSelection->setEnabled(false);

	try {
		bool onlySelected = m_ui->radioButtonSelectedGeometry->isChecked();
		RC::VicusClipper vicusClipper(prj.m_buildings, prj.m_componentInstances, maxAngle, maxDist, prj.nextUnusedID(), onlySelected);
		vicusClipper.findParallelSurfaces(&progressNotifyer);
		vicusClipper.findSurfacesInRange(&progressNotifyer);
		vicusClipper.clipSurfaces(&progressNotifyer);
		vicusClipper.setPrj(SVProjectHandler::instance().project());

		for (unsigned int i=0; i<RC::VicusClipper::PredefinedComponentType::NUM_PDC; ++i)
			vicusClipper.setStandardConstruction((RC::VicusClipper::PredefinedComponentType)i, m_predefinedComponentIds[i]);

		vicusClipper.createComponentInstances(&progressNotifyer, m_ui->checkBoxCreateComp->isChecked(),
											  m_ui->checkBoxReplaceComponentInstances->isChecked());

		m_componentInstances = *vicusClipper.vicusCompInstances();
		m_subSurfaceComponentInstances = *vicusClipper.vicusSubSurfCompInstances();
		m_buildings = vicusClipper.vicusBuildings();

		QMessageBox::information(this, tr("Smart-Clipping"),
								 tr("Smart-Clipping has been performed successfully. Please review the import log for warnings!"));
	}
	catch (IBK::Exception &ex) {
		IBK::IBK_Message(IBK::FormatString("Smart-clipping encountered an error"), IBK::MSG_ERROR);
		QMessageBox::critical((QWidget*)parent(), tr("Smart-Clipping error"), tr("Error during smart-clipping:\n%1").arg(ex.what()));
		m_ui->groupBoxConnectSurfaces->setEnabled(true);
		m_ui->groupBoxProjectSelection->setEnabled(true);
		m_ui->pushButtonApply->setEnabled(false);
		progressNotifyer.m_prgDlg->hide();
		return;
	}

	m_ui->pushButtonApply->setEnabled(true);
	m_ui->groupBoxConnectSurfaces->setEnabled(true);
	m_ui->groupBoxProjectSelection->setEnabled(true);
}


void SVSmartIntersectionDialog::on_pushButtonApply_clicked() {
	m_returnCode = AcceptClipping;
	accept();
}


void SVSmartIntersectionDialog::on_pushButtonCancel_clicked() {
	m_returnCode = CancelledClipping;
	reject();
}


const std::vector<VICUS::ComponentInstance> &SVSmartIntersectionDialog::componentInstances() const {
	return m_componentInstances;
}


const std::vector<VICUS::Building> &SVSmartIntersectionDialog::buildings() const {
	return m_buildings;
}


void SVSmartIntersectionDialog::updateStandardConstruction(const RC::VicusClipper::PredefinedComponentType &type) {
	// get construction edit dialog from mainwindow
	SVDatabaseEditDialog * compEditDialog = SVMainWindow::instance().dbComponentEditDialog();
	unsigned int previousId = m_predefinedComponentIds[type];
	unsigned int compId = compEditDialog->select(previousId);
	if (compId != VICUS::INVALID_ID && compId != previousId)
		m_predefinedComponentIds[type] = compId;

	updateUi();
}


void SVSmartIntersectionDialog::updateUi() {
	const VICUS::Component *comp = m_db->m_components[m_predefinedComponentIds[RC::VicusClipper::PDC_InteriorWall]];
	Q_ASSERT(comp != nullptr);
	m_ui->lineEditInteriorWallName->setText(QtExt::MultiLangString2QString(comp->m_displayName));

	comp = m_db->m_components[m_predefinedComponentIds[RC::VicusClipper::PDC_ExteriorWall]];
	Q_ASSERT(comp != nullptr);
	m_ui->lineEditExteriorWallName->setText(QtExt::MultiLangString2QString(comp->m_displayName));

	comp = m_db->m_components[m_predefinedComponentIds[RC::VicusClipper::PDC_Ceiling]];
	Q_ASSERT(comp != nullptr);
	m_ui->lineEditCeilingName->setText(QtExt::MultiLangString2QString(comp->m_displayName));

	comp = m_db->m_components[m_predefinedComponentIds[RC::VicusClipper::PDC_Roof]];
	Q_ASSERT(comp != nullptr);
	m_ui->lineEditRoofName->setText(QtExt::MultiLangString2QString(comp->m_displayName));

	comp = m_db->m_components[m_predefinedComponentIds[RC::VicusClipper::PDC_Floor]];
	Q_ASSERT(comp != nullptr);
	m_ui->lineEditFloorName->setText(QtExt::MultiLangString2QString(comp->m_displayName));
}

const std::vector<VICUS::SubSurfaceComponentInstance> &SVSmartIntersectionDialog::subSurfaceComponentInstances() const {
	return m_subSurfaceComponentInstances;
}


void SVSmartIntersectionDialog::on_toolButtonSelectInteriorWall_clicked() {
	updateStandardConstruction(RC::VicusClipper::PDC_InteriorWall);
}


void SVSmartIntersectionDialog::on_toolButtonSelectExteriorWall_clicked() {
	updateStandardConstruction(RC::VicusClipper::PDC_ExteriorWall);
}


void SVSmartIntersectionDialog::on_toolButtonSelectCeiling_clicked() {
	updateStandardConstruction(RC::VicusClipper::PDC_Ceiling);
}


void SVSmartIntersectionDialog::on_toolButtonSelectFloor_clicked() {
	updateStandardConstruction(RC::VicusClipper::PDC_Floor);
}

void SVSmartIntersectionDialog::on_toolButtonSelectRoof_clicked() {
	updateStandardConstruction(RC::VicusClipper::PDC_Roof);
}

