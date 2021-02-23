#include "SVDBNetworkComponentEditWidget.h"
#include "ui_SVDBNetworkComponentEditWidget.h"

#include <VICUS_KeywordList.h>
#include <VICUS_KeywordListQt.h>
#include <VICUS_NetworkComponent.h>

#include <NANDRAD_HydraulicNetworkComponent.h>
#include <NANDRAD_KeywordList.h>

#include <QtExt_LanguageHandler.h>
#include "QtExt_Locale.h"

#include "SVSettings.h"
#include "SVDBNetworkComponentTableModel.h"
#include "SVDBConstructionEditDialog.h"
#include "SVDBBoundaryConditionEditDialog.h"
#include "SVMainWindow.h"
#include "SVStyle.h"

SVDBNetworkComponentEditWidget::SVDBNetworkComponentEditWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVDBNetworkComponentEditWidget)
{
	m_ui->setupUi(this);

	m_ui->lineEditName->initLanguages(QtExt::LanguageHandler::instance().langId().toStdString(), "fr", true);
	m_ui->lineEditName->setDialog3Caption(tr("Component identification name"));

	// enter categories into combo box
	// block signals to avoid getting "changed" calls
	m_ui->comboBoxComponentType->blockSignals(true);
	for (int i=0; i<VICUS::NetworkComponent::NUM_MT; ++i)
		m_ui->comboBoxComponentType->addItem(VICUS::KeywordListQt::Description("NetworkComponent::ModelType", i), i);
	m_ui->comboBoxComponentType->blockSignals(false);

	// no headers
	m_ui->tableWidgetParameters->horizontalHeader()->setVisible(false);
	m_ui->tableWidgetParameters->verticalHeader()->setVisible(false);
	m_ui->tableWidgetParameters->setColumnCount(3);
	m_ui->tableWidgetParameters->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	m_ui->tableWidgetParameters->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
	m_ui->tableWidgetParameters->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);

	SVStyle::formatDatabaseTableView(m_ui->tableWidgetParameters);
	m_ui->tableWidgetParameters->setSortingEnabled(false);

	// TODO : Hauke, implement table item delegate if needed

	updateInput(-1);

	// check if enums are identical
	Q_ASSERT(NANDRAD::HydraulicNetworkComponent::P_HydraulicDiameter == (int)VICUS::NetworkComponent::P_HydraulicDiameter);
	Q_ASSERT(NANDRAD::HydraulicNetworkComponent::P_PressureLossCoefficient == (int)VICUS::NetworkComponent::P_PressureLossCoefficient);
	Q_ASSERT(NANDRAD::HydraulicNetworkComponent::P_ExternalHeatTransferCoefficient == (int)VICUS::NetworkComponent::P_ExternalHeatTransferCoefficient);
	Q_ASSERT(NANDRAD::HydraulicNetworkComponent::P_PressureHead == (int)VICUS::NetworkComponent::P_PressureHead);
	Q_ASSERT(NANDRAD::HydraulicNetworkComponent::P_PumpEfficiency == (int)VICUS::NetworkComponent::P_PumpEfficiency);
	Q_ASSERT(NANDRAD::HydraulicNetworkComponent::P_MotorEfficiency == (int)VICUS::NetworkComponent::P_MotorEfficiency);
	Q_ASSERT(NANDRAD::HydraulicNetworkComponent::P_Volume == (int)VICUS::NetworkComponent::P_Volume);
	Q_ASSERT(NANDRAD::HydraulicNetworkComponent::P_UAValue == (int)VICUS::NetworkComponent::P_UAValue);
	Q_ASSERT(NANDRAD::HydraulicNetworkComponent::P_PipeMaxDiscretizationWidth == (int)VICUS::NetworkComponent::P_PipeMaxDiscretizationWidth);
	Q_ASSERT(NANDRAD::HydraulicNetworkComponent::P_CondenserMeanTemperature == (int)VICUS::NetworkComponent::P_CondenserMeanTemperature);
	Q_ASSERT(NANDRAD::HydraulicNetworkComponent::P_CarnotEfficiency == (int)VICUS::NetworkComponent::P_CarnotEfficiency);
	Q_ASSERT(NANDRAD::HydraulicNetworkComponent::NUM_P == (int)VICUS::NetworkComponent::NUM_P);

	Q_ASSERT(NANDRAD::HydraulicNetworkComponent::MT_StaticPipe == (int)VICUS::NetworkComponent::MT_StaticPipe);
	Q_ASSERT(NANDRAD::HydraulicNetworkComponent::MT_DynamicPipe == (int)VICUS::NetworkComponent::MT_DynamicPipe);
	Q_ASSERT(NANDRAD::HydraulicNetworkComponent::MT_HeatExchanger == (int)VICUS::NetworkComponent::MT_HeatExchanger);
	Q_ASSERT(NANDRAD::HydraulicNetworkComponent::MT_HeatPumpIdealCarnot == (int)VICUS::NetworkComponent::MT_HeatPumpIdealCarnot);

	Q_ASSERT(NANDRAD::HydraulicNetworkComponent::HT_HeatFluxConstant == (int)VICUS::NetworkComponent::HT_HeatFluxConstant);
	Q_ASSERT(NANDRAD::HydraulicNetworkComponent::HT_HeatFluxDataFile == (int)VICUS::NetworkComponent::HT_HeatFluxDataFile);
	Q_ASSERT(NANDRAD::HydraulicNetworkComponent::HT_TemperatureConstant == (int)VICUS::NetworkComponent::HT_TemperatureConstant);
	Q_ASSERT(NANDRAD::HydraulicNetworkComponent::HT_TemperatureDataFile == (int)VICUS::NetworkComponent::HT_TemperatureDataFile);
	Q_ASSERT(NANDRAD::HydraulicNetworkComponent::HT_HeatExchangeWithFMUTemperature == (int)VICUS::NetworkComponent::HT_HeatExchangeWithFMUTemperature);
	Q_ASSERT(NANDRAD::HydraulicNetworkComponent::HT_HeatExchangeWithZoneTemperature == (int)VICUS::NetworkComponent::HT_HeatExchangeWithZoneTemperature);
}


SVDBNetworkComponentEditWidget::~SVDBNetworkComponentEditWidget() {
	delete m_ui;
}


void SVDBNetworkComponentEditWidget::setup(SVDatabase * db, SVDBNetworkComponentTableModel * dbModel) {
	m_db = db;
	m_dbModel = dbModel;
}


void SVDBNetworkComponentEditWidget::updateInput(int id) {
	FUNCID(SVDBNetworkComponentEditWidget::updateInput);

	m_currentComponent = nullptr; // disable edit triggers

	if (id == -1) {
		// disable all controls - note: this does not disable signals of the components below
		setEnabled(false);

		// clear input controls
		m_ui->lineEditName->setString(IBK::MultiLanguageString());

		// construction property info fields
		m_ui->comboBoxComponentType->blockSignals(true);
		m_ui->comboBoxComponentType->setCurrentIndex(-1);
		m_ui->comboBoxComponentType->blockSignals(false);

		// Note: color button is disabled, hence color is gray
		return;
	}
	// re-enable all controls
	setEnabled(true);

	VICUS::NetworkComponent * comp = const_cast<VICUS::NetworkComponent*>(m_db->m_networkComponents[(unsigned int)id]);
	m_currentComponent = comp;

	// now update the GUI controls
	m_ui->lineEditName->setString(comp->m_displayName);

	m_ui->comboBoxComponentType->blockSignals(true);
	int typeIdx = m_ui->comboBoxComponentType->findData(comp->m_modelType);
	m_ui->comboBoxComponentType->setCurrentIndex(typeIdx);
	m_ui->comboBoxComponentType->blockSignals(false);

	m_ui->pushButtonComponentColor->blockSignals(true);
	m_ui->pushButtonComponentColor->setColor(m_currentComponent->m_color);
	m_ui->pushButtonComponentColor->blockSignals(false);

	// populate table widget with properties
	m_ui->tableWidgetParameters->clearContents();

	// only insert parameters that are actually needed for the current model type

	// NOTE: we assume that ModelType enums are the same in both objects!
	NANDRAD::HydraulicNetworkComponent::ModelType nandradModelType = (NANDRAD::HydraulicNetworkComponent::ModelType)m_currentComponent->m_modelType;
	NANDRAD::HydraulicNetworkComponent::HeatExchangeType heatExchangeType = (NANDRAD::HydraulicNetworkComponent::HeatExchangeType)
			m_currentComponent->m_heatExchangeType;

	std::vector<unsigned int> paraVec = NANDRAD::HydraulicNetworkComponent::requiredParameter(nandradModelType, heatExchangeType, 1);
	m_ui->tableWidgetParameters->setRowCount(paraVec.size());

	if (paraVec.empty())
		m_ui->groupBoxModelParameters->setEnabled(false);
	else
		m_ui->groupBoxModelParameters->setEnabled(true);

	m_ui->tableWidgetParameters->blockSignals(true);
	for (unsigned int i=0; i<paraVec.size(); ++i) {
		QTableWidgetItem * item = new QTableWidgetItem(VICUS::KeywordListQt::Keyword("NetworkComponent::para_t", (int)paraVec[i]));
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		m_ui->tableWidgetParameters->setItem((int)i, 0, item);
		try {
			IBK::Unit ioUnit(VICUS::KeywordListQt::Unit("NetworkComponent::para_t", (int)paraVec[i]));
			item = new QTableWidgetItem(QString::fromStdString(ioUnit.name()));
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			m_ui->tableWidgetParameters->setItem((int)i, 2, item);

			// Mind: unit conversion needed if keyword-list unit does not match base SI unit
			//       but add check if there is no value yet
			if (m_currentComponent->m_para[paraVec[i]].name.empty())
				item = new QTableWidgetItem(); // TODO : Hauke, set some meaningful initial value?
			else
				item = new QTableWidgetItem(QString("%L1").arg(m_currentComponent->m_para[paraVec[i]].get_value(ioUnit)));
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
			m_ui->tableWidgetParameters->setItem((int)i, 1, item);
		}
		catch (IBK::Exception & ex) {
			IBK::IBK_Message(ex.what(), IBK::MSG_ERROR, FUNC_ID);
		}
	}
	m_ui->tableWidgetParameters->blockSignals(false);
	m_ui->tableWidgetParameters->resizeColumnsToContents();

	// heat exchange type
	setupComboboxHeatExchangeType();
	m_ui->comboBoxHeatExchangeType->setCurrentIndex(m_ui->comboBoxHeatExchangeType->findData(
														m_currentComponent->m_heatExchangeType));

	// for built-ins, disable editing/make read-only
	bool isEditable = !comp->m_builtIn;
	m_ui->lineEditName->setReadOnly(!isEditable);
	m_ui->pushButtonComponentColor->setReadOnly(!isEditable);
	m_ui->comboBoxComponentType->setEnabled(isEditable);

}


void SVDBNetworkComponentEditWidget::setupComboboxHeatExchangeType()
{
	m_ui->comboBoxHeatExchangeType->clear();
	std::vector<unsigned int> hxTypes = NANDRAD::HydraulicNetworkComponent::availableHeatExchangeTypes(
				NANDRAD::HydraulicNetworkComponent::ModelType((int)m_currentComponent->m_modelType));
	m_ui->comboBoxHeatExchangeType->addItem("No Heat Exchange", NANDRAD::HydraulicNetworkComponent::NUM_HT);
	for (unsigned int type: hxTypes)
		m_ui->comboBoxHeatExchangeType->addItem(
					NANDRAD::KeywordList::Description("HydraulicNetworkComponent::HeatExchangeType", (int)type), type);


}


void SVDBNetworkComponentEditWidget::on_lineEditName_editingFinished(){
	Q_ASSERT(m_currentComponent != nullptr);

	if (m_currentComponent->m_displayName != m_ui->lineEditName->string()) {
		m_currentComponent->m_displayName = m_ui->lineEditName->string();
		m_db->m_networkComponents.m_modified = true;
		m_dbModel->setItemModified(m_currentComponent->m_id); // tell model that we changed the data
	}
}


void SVDBNetworkComponentEditWidget::on_comboBoxComponentType_currentIndexChanged(int /*index*/) {
	if (m_currentComponent == nullptr) return; // m_current is nullptr, when nothing is selected and controls are defaulted to "empty"

	VICUS::NetworkComponent::ModelType ct = static_cast<VICUS::NetworkComponent::ModelType>(m_ui->comboBoxComponentType->currentData().toInt());
	if (ct != m_currentComponent->m_modelType) {
		m_currentComponent->m_modelType = ct;
		m_db->m_networkComponents.m_modified = true;
		m_dbModel->setItemModified(m_currentComponent->m_id); // tell model that we changed the data
		updateInput((int)m_currentComponent->m_id);
	}
}


void SVDBNetworkComponentEditWidget::on_pushButtonComponentColor_colorChanged() {
	Q_ASSERT(m_currentComponent != nullptr);

	if (m_currentComponent->m_color != m_ui->pushButtonComponentColor->color()) {
		m_currentComponent->m_color = m_ui->pushButtonComponentColor->color();
		m_db->m_networkComponents.m_modified = true;
		m_dbModel->setItemModified(m_currentComponent->m_id); // tell model that we changed the data
	}
}


void SVDBNetworkComponentEditWidget::on_comboBoxHeatExchangeType_activated(const QString &arg1)
{
	Q_ASSERT(m_currentComponent != nullptr);

	m_currentComponent->m_heatExchangeType = VICUS::NetworkComponent::HeatExchangeType(
				m_ui->comboBoxHeatExchangeType->currentData().toUInt());
	m_db->m_networkComponents.m_modified = true;
	m_dbModel->setItemModified(m_currentComponent->m_id); // tell model that we changed the data
}


void SVDBNetworkComponentEditWidget::on_tableWidgetParameters_cellChanged(int row, int column) {

	QString errMsg("");

	// first check if it is a double
	QString text = m_ui->tableWidgetParameters->item(row, column)->text();
	bool ok = false;
	double val = QtExt::Locale().toDouble(text, &ok);
	// but also allow fall-back on C-locale
	if (!ok)
		val = text.toDouble(&ok);
	if (!ok)
		errMsg = "Only numbers allowed!";

	std::string parName = m_ui->tableWidgetParameters->item(row, 0)->text().toStdString();
	VICUS::NetworkComponent::para_t paraNum = VICUS::NetworkComponent::para_t(
												VICUS::KeywordList::Enumeration("NetworkComponent::para_t", parName));

	// now do parameter specific checks
	if (ok){
		IBK::Parameter parameter(VICUS::KeywordList::Keyword("NetworkComponent::para_t", paraNum), val,
								 VICUS::KeywordList::Unit("NetworkComponent::para_t", paraNum));
		try {
			NANDRAD::HydraulicNetworkComponent::checkModelParameter(parameter, paraNum);
		} catch (IBK::Exception &ex) {
			errMsg = ex.what();
			ok = false;
		}
	}

	// modify item and show message box
	if (!ok){
		m_ui->tableWidgetParameters->blockSignals(true);
		if (m_currentComponent->m_para[paraNum].empty())
			m_ui->tableWidgetParameters->item(row, column)->setText("");
		else
			m_ui->tableWidgetParameters->item(row, column)->setText(QString("%1")
																	.arg(m_currentComponent->m_para[paraNum].value));
		m_ui->tableWidgetParameters->blockSignals(false);
		QMessageBox msgBox(QMessageBox::Critical, "Invalid Value", errMsg, QMessageBox::Ok, this);
		msgBox.exec();
		return;
	}

	// finally set value
	VICUS::KeywordList::setParameter(m_currentComponent->m_para, "NetworkComponent::para_t", paraNum, val);
	m_db->m_networkComponents.m_modified = true;
	m_dbModel->setItemModified(m_currentComponent->m_id);
}


