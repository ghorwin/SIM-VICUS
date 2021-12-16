#include "SVDBSubNetworkEditWidget.h"
#include "ui_SVDBSubNetworkEditWidget.h"

#include "SVDBSubNetworkTableModel.h"
#include "SVStyle.h"
#include "SVMainWindow.h"
#include "SVDatabaseEditDialog.h"

#include <QtExt_Conversions.h>

#include <VICUS_KeywordList.h>
#include <VICUS_utilities.h>


SVDBSubNetworkEditWidget::SVDBSubNetworkEditWidget(QWidget *parent) :
	SVAbstractDatabaseEditWidget(parent),
	m_ui(new Ui::SVDBSubNetworkEditWidget)
{
	m_ui->setupUi(this);

	m_ui->tableWidgetElements->setColumnCount(1);
	m_ui->tableWidgetElements->setHorizontalHeaderLabels(QStringList() << tr("Network Elements"));
	SVStyle::formatDatabaseTableView(m_ui->tableWidgetElements);
	m_ui->tableWidgetElements->setSortingEnabled(false);
	m_ui->tableWidgetElements->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
	m_ui->tableWidgetElements->horizontalHeader()->resizeSection(0,20);
}

SVDBSubNetworkEditWidget::~SVDBSubNetworkEditWidget()
{
	delete m_ui;
}

void SVDBSubNetworkEditWidget::setup(SVDatabase *db, SVAbstractDatabaseTableModel *dbModel)
{
	m_db = db;
	m_dbModel = dynamic_cast<SVDBSubNetworkTableModel*>(dbModel);

	m_ui->lineEditComponent->setReadOnly(true);
	m_ui->lineEditController->setReadOnly(true);
}

void SVDBSubNetworkEditWidget::updateInput(int id)
{
	m_currentSubNet = nullptr; // disable edit triggers

	if (id == -1) {
		// disable all controls - note: this does not disable signals of the components below
		setEnabled(false);

		// clear input controls
		m_ui->lineEditSubNetworkName->setText(QString());
		m_ui->lineEditComponent->clear();
		m_ui->lineEditController->clear();
		m_ui->lineEditController->setDisabled(true);
		m_ui->tableWidgetElements->blockSignals(true);
		m_ui->tableWidgetElements->clear();
		m_ui->tableWidgetElements->blockSignals(false);

		// Note: color button is disabled, hence color is gray
		return;
	}

	// re-enable all controls
	setEnabled(true);

	// get current sub network
	m_currentSubNet = const_cast<VICUS::SubNetwork*>(m_db->m_subNetworks[(unsigned int)id]);
	Q_ASSERT(m_currentSubNet != nullptr);

	// now update the GUI controls
	m_ui->lineEditSubNetworkName->setString(m_currentSubNet->m_displayName);

	updateTableWidget();

	// for built-ins, disable editing/make read-only
	bool isEditable = !m_currentSubNet->m_builtIn;
	m_ui->lineEditComponent->setReadOnly(!isEditable);
	m_ui->lineEditController->setReadOnly(!isEditable);
	m_ui->lineEditElementName->setReadOnly(!isEditable);
	m_ui->lineEditSubNetworkName->setReadOnly(!isEditable);
	m_ui->toolButtonAdd->setEnabled(isEditable);
	m_ui->toolButtonRemove->setEnabled(isEditable);
	m_ui->toolButtonEditComponent->setEnabled(isEditable);
	m_ui->toolButtonEditController->setEnabled(isEditable);
}


void SVDBSubNetworkEditWidget::modelModify() {
	m_db->m_subNetworks.m_modified = true;
	m_dbModel->setItemModified(m_currentSubNet->m_id);
}


void SVDBSubNetworkEditWidget::updateTableWidget() {
	m_ui->tableWidgetElements->blockSignals(true);
	m_ui->tableWidgetElements->clear();
	m_ui->tableWidgetElements->blockSignals(false);

	if (m_currentSubNet == nullptr)
		return;

	int row = 0;
	m_ui->tableWidgetElements->blockSignals(true);
	m_ui->tableWidgetElements->setRowCount(m_currentSubNet->m_elements.size());
	for (const VICUS::NetworkElement &el: m_currentSubNet->m_elements){
		QTableWidgetItem * item = new QTableWidgetItem();
		item->setText(el.m_displayName);
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		m_ui->tableWidgetElements->setItem(row, 0, item);
		++row;
	}

	if (m_ui->tableWidgetElements->rowCount() > 0){
		m_ui->tableWidgetElements->selectRow(m_ui->tableWidgetElements->rowCount()-1);
	}
	m_ui->tableWidgetElements->blockSignals(false);

	updateElementProperties();
}

void SVDBSubNetworkEditWidget::updateElementProperties()
{
	Q_ASSERT(m_currentSubNet != nullptr);

	// get current row = idx of current element
	m_currentElementIdx = m_ui->tableWidgetElements->currentRow();


	// TODO Hauke: disable controller line edit if the according component can not use a controller

	// rework entire widget ???


	// if we have at least one element: update infos
	if (m_currentElementIdx >= 0){

		const VICUS::NetworkElement &elem = m_currentSubNet->m_elements[(unsigned int)m_currentElementIdx];

		// checkBox
		m_ui->checkBoxElementHasHeatExchange->setChecked(elem.m_id == m_currentSubNet->m_idHeatExchangeElement);

		// line edits
		m_ui->groupBoxEditElement->setEnabled(true);
		m_ui->lineEditElementName->setText(elem.m_displayName);
		if (m_db->m_networkComponents[elem.m_componentId] != nullptr)
			m_ui->lineEditComponent->setText(QtExt::MultiLangString2QString(
												 m_db->m_networkComponents[elem.m_componentId]->m_displayName));
		else
			m_ui->lineEditComponent->clear();

		if (m_db->m_networkControllers[elem.m_controlElementId] != nullptr)
			m_ui->lineEditController->setText(QtExt::MultiLangString2QString(
												  m_db->m_networkControllers[elem.m_controlElementId]->m_displayName));
		else
			m_ui->lineEditController->clear();

	}
	else
		m_ui->groupBoxEditElement->setEnabled(false);
}

void SVDBSubNetworkEditWidget::setInletOutletIds()
{
	Q_ASSERT(m_currentSubNet != nullptr);

	// set inlet and outlet ids of elements
	// first elements inletId shall be INLET_ID and last elements outletID shall be OUTLET_ID
	for (unsigned int i = 0; i < m_currentSubNet->m_elements.size(); ++i){
		m_currentSubNet->m_elements[i].m_inletNodeId = i;
		m_currentSubNet->m_elements[i].m_outletNodeId = i+1;
		if (i==0)
			m_currentSubNet->m_elements[i].m_inletNodeId = VICUS::SubNetwork::INLET_ID;
		if (i==m_currentSubNet->m_elements.size()-1)
			m_currentSubNet->m_elements[i].m_outletNodeId = VICUS::SubNetwork::OUTLET_ID;
	}
}


void SVDBSubNetworkEditWidget::on_lineEditSubNetworkName_editingFinished()
{
	Q_ASSERT(m_currentSubNet != nullptr);
	if (m_currentSubNet->m_displayName != m_ui->lineEditSubNetworkName->string()) {
		m_currentSubNet->m_displayName = m_ui->lineEditSubNetworkName->string();
		modelModify();
	}
	updateInput((int)m_currentSubNet->m_id);
}


void SVDBSubNetworkEditWidget::on_pushButtonColor_clicked()
{
	Q_ASSERT(m_currentSubNet != nullptr);
	if (m_currentSubNet->m_color != m_ui->pushButtonColor->color()) {
		m_currentSubNet->m_color = m_ui->pushButtonColor->color();
		modelModify();
	}
	updateInput((int)m_currentSubNet->m_id);
}


void SVDBSubNetworkEditWidget::on_toolButtonAdd_clicked()
{
	Q_ASSERT(m_currentSubNet != nullptr);

	if (m_currentSubNet->m_elements.size()>10)
		return;
	VICUS::NetworkElement el;
	el.m_id = VICUS::uniqueId(m_currentSubNet->m_elements);
	el.m_displayName = QString("new element %1").arg(el.m_id);
	m_currentSubNet->m_elements.push_back(el);
	setInletOutletIds();
	modelModify();
	updateTableWidget();
}

void SVDBSubNetworkEditWidget::on_toolButtonRemove_clicked()
{
	if (m_currentSubNet->m_elements.size()==0)
		return;
	m_currentSubNet->m_elements.pop_back();
	setInletOutletIds();
	modelModify();
	updateTableWidget();
}

void SVDBSubNetworkEditWidget::on_tableWidgetElements_itemSelectionChanged()
{
	Q_ASSERT(m_currentSubNet != nullptr);
	updateElementProperties();
}


void SVDBSubNetworkEditWidget::on_lineEditElementName_editingFinished()
{
	if (m_currentElementIdx >= 0){
		m_currentSubNet->m_elements[(unsigned int)m_currentElementIdx].m_displayName =
																	m_ui->lineEditElementName->text();
		modelModify();
		updateTableWidget();
	}
}

void SVDBSubNetworkEditWidget::on_toolButtonEditComponent_clicked()
{
	Q_ASSERT((unsigned int)m_currentElementIdx < m_currentSubNet->m_elements.size());

	unsigned int compId = m_currentSubNet->m_elements[(unsigned int)m_currentElementIdx].m_componentId;
	unsigned int newId = SVMainWindow::instance().dbNetworkComponentEditDialog()->select(compId);

	// if dialog was canceled do nothing
	if (newId == VICUS::INVALID_ID)
		return;

	if (newId != compId){
		m_currentSubNet->m_elements[(unsigned int)m_currentElementIdx].m_componentId = newId;
		modelModify();
		updateElementProperties();
	}
}

void SVDBSubNetworkEditWidget::on_toolButtonEditController_clicked()
{
	Q_ASSERT((unsigned int)m_currentElementIdx < m_currentSubNet->m_elements.size());

	unsigned int ctrId = m_currentSubNet->m_elements[(unsigned int)m_currentElementIdx].m_controlElementId;
	unsigned int newId = SVMainWindow::instance().dbNetworkControllerEditDialog()->select(ctrId);

	// if dialog was canceled do nothing
	if (newId == VICUS::INVALID_ID)
		return;

	if (newId != ctrId){
		m_currentSubNet->m_elements[(unsigned int)m_currentElementIdx].m_controlElementId = newId;
		modelModify();
		updateElementProperties();
	}

}


void SVDBSubNetworkEditWidget::on_checkBoxElementHasHeatExchange_clicked(bool checked)
{
	Q_ASSERT((unsigned int)m_currentElementIdx < m_currentSubNet->m_elements.size());

	unsigned int hxId;
	if (checked)
		hxId = m_currentSubNet->m_elements[(unsigned int)m_currentElementIdx].m_id;
	else
		hxId = VICUS::INVALID_ID;

	if (hxId != m_currentSubNet->m_idHeatExchangeElement){
		m_currentSubNet->m_idHeatExchangeElement = hxId;
		modelModify();
		updateElementProperties();
	}
}
