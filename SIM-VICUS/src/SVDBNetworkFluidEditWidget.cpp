#include "SVDBNetworkFluidEditWidget.h"
#include "ui_SVDBNetworkFluidEditWidget.h"

#include <NANDRAD_HydraulicFluid.h>

#include <QtExt_LanguageHandler.h>
#include <SVConstants.h>

#include "SVSettings.h"
#include "SVDBNetworkFluidTableModel.h"
#include "SVDatabaseEditDialog.h"
#include "SVMainWindow.h"
#include "SVStyle.h"


SVDBNetworkFluidEditWidget::SVDBNetworkFluidEditWidget(QWidget *parent) :
	SVAbstractDatabaseEditWidget(parent),
	m_ui(new Ui::SVDBNetworkFluidEditWidget)
{
	m_ui->setupUi(this);

	m_ui->lineEditName->initLanguages(QtExt::LanguageHandler::instance().langId().toStdString(),THIRD_LANGUAGE, true);
	m_ui->lineEditName->setDialog3Caption(tr("Component identification name"));

}

SVDBNetworkFluidEditWidget::~SVDBNetworkFluidEditWidget()
{
	delete m_ui;
}


void SVDBNetworkFluidEditWidget::setup(SVDatabase * db, SVAbstractDatabaseTableModel * dbModel) {
	m_db = db;
	m_dbModel = dynamic_cast<SVDBNetworkFluidTableModel*>(dbModel);
}


void SVDBNetworkFluidEditWidget::updateInput(int id) {
	FUNCID(SVDBNetworkComponentEditWidget::updateInput);

	m_currentFluid = nullptr; // disable edit triggers

	if (id == -1) {
		// disable all controls - note: this does not disable signals of the components below
		setEnabled(false);

		// clear input controls
		m_ui->lineEditName->setString(IBK::MultiLanguageString());
		m_ui->lineEditDensity->clear();
		m_ui->lineEditHeatCapacity->clear();
		m_ui->lineEditThermalConductivity->clear();

		// Note: color button is disabled, hence color is gray
		return;
	}
	// re-enable all controls
	setEnabled(true);

	VICUS::NetworkFluid * fluid = const_cast<VICUS::NetworkFluid*>(m_db->m_fluids[(unsigned int)id]);
	m_currentFluid = fluid;

	// now update the GUI controls
	m_ui->lineEditName->setString(fluid->m_displayName);
	m_ui->lineEditDensity->setValue(fluid->m_para[VICUS::NetworkFluid::P_Density].value);
	m_ui->lineEditHeatCapacity->setValue(fluid->m_para[VICUS::NetworkFluid::P_HeatCapacity].value);
	m_ui->lineEditThermalConductivity->setValue(fluid->m_para[VICUS::NetworkFluid::P_Conductivity].value);

	m_ui->pushButtonComponentColor->blockSignals(true);
	m_ui->pushButtonComponentColor->setColor(m_currentFluid->m_color);
	m_ui->pushButtonComponentColor->blockSignals(false);

	// only insert parameters that are actually needed for the current model type

	// for built-ins, disable editing/make read-only
	bool isEditable = !fluid->m_builtIn;
	m_ui->lineEditName->setReadOnly(!isEditable);
	m_ui->pushButtonComponentColor->setReadOnly(!isEditable);

}


void SVDBNetworkFluidEditWidget::on_lineEditName_editingFinished(){
	Q_ASSERT(m_currentFluid != nullptr);

	if (m_currentFluid->m_displayName != m_ui->lineEditName->string()) {
		m_currentFluid->m_displayName = m_ui->lineEditName->string();
		m_db->m_fluids.m_modified = true;
		m_dbModel->setItemModified(m_currentFluid->m_id); // tell model that we changed the data
	}
}

void SVDBNetworkFluidEditWidget::on_pushButtonComponentColor_colorChanged()
{

}

void SVDBNetworkFluidEditWidget::on_lineEditDensity_editingFinished()
{

}

void SVDBNetworkFluidEditWidget::on_lineEditHeatCapacity_editingFinished()
{

}

void SVDBNetworkFluidEditWidget::on_lineEditThermalConductivity_editingFinished()
{

}
