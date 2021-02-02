#include "SVDBNetworkComponentEditWidget.h"
#include "ui_SVDBNetworkComponentEditWidget.h"

#include <VICUS_KeywordList.h>
#include <VICUS_KeywordListQt.h>
#include <VICUS_NetworkComponent.h>

#include <NANDRAD_HydraulicNetworkComponent.h>

#include <QtExt_LanguageHandler.h>

#include "SVSettings.h"
#include "SVDBNetworkComponentTableModel.h"
#include "SVDBConstructionEditDialog.h"
#include "SVDBBoundaryConditionEditDialog.h"
#include "SVMainWindow.h"

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
		// TODO : Hauke, update next line as described in email
		m_ui->comboBoxComponentType->addItem(VICUS::KeywordListQt::Description("NetworkComponent::ModelType", i), i);
	m_ui->comboBoxComponentType->blockSignals(false);

	// no headers
	m_ui->tableWidgetParameters->horizontalHeader()->setVisible(false);
	m_ui->tableWidgetParameters->verticalHeader()->setVisible(false);
	m_ui->tableWidgetParameters->setColumnCount(3);

	updateInput(-1);
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

	std::vector<unsigned int> para = NANDRAD::HydraulicNetworkComponent::requiredParameter(nandradModelType);

	m_ui->tableWidgetParameters->setRowCount(para.size());
	for (unsigned int i=0; i<para.size(); ++i) {
		QTableWidgetItem * item = new QTableWidgetItem(VICUS::KeywordListQt::Keyword("NetworkComponent::para_t", (int)para[i]));
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		m_ui->tableWidgetParameters->setItem((int)i, 0, item);
		try {
			IBK::Unit ioUnit(VICUS::KeywordListQt::Unit("NetworkComponent::para_t", (int)para[i]));
			// Mind: unit conversion needed if keyword-list unit does not match base SI unit
			item = new QTableWidgetItem(QString("%L1").arg(m_currentComponent->m_para[i].get_value(ioUnit)));
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
			m_ui->tableWidgetParameters->setItem((int)i, 1, item);
			item = new QTableWidgetItem();
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			m_ui->tableWidgetParameters->setItem((int)i, 2, item);
		}
		catch (IBK::Exception & ex) {
			IBK::IBK_Message(ex.what(), IBK::MSG_ERROR, FUNC_ID);
		}
	}

	// TODO : Hauke, implement table item delegate if needed

	// for built-ins, disable editing/make read-only
	bool isEditable = !comp->m_builtIn;
	m_ui->lineEditName->setReadOnly(!isEditable);
	m_ui->pushButtonComponentColor->setReadOnly(!isEditable);
	m_ui->comboBoxComponentType->setEnabled(isEditable);


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

