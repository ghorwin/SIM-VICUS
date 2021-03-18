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

#include <VICUS_KeywordList.h>


SVDBNetworkFluidEditWidget::SVDBNetworkFluidEditWidget(QWidget *parent) :
	SVAbstractDatabaseEditWidget(parent),
	m_ui(new Ui::SVDBNetworkFluidEditWidget)
{
	m_ui->setupUi(this);
	m_ui->verticalLayout->setMargin(0);

	m_ui->lineEditName->initLanguages(QtExt::LanguageHandler::instance().langId().toStdString(),THIRD_LANGUAGE, true);
	m_ui->lineEditName->setDialog3Caption(tr("Fluid identification name"));

	// table widget
	m_ui->tableWidgetViscosity->setColumnCount(2);
	m_ui->tableWidgetViscosity->verticalHeader()->setVisible(false);
	m_ui->tableWidgetViscosity->setHorizontalHeaderItem(0, new QTableWidgetItem("Temperature [C]"));
	m_ui->tableWidgetViscosity->setHorizontalHeaderItem(1, new QTableWidgetItem("Kinematic Viscosity [m/s²]"));

	SVStyle::formatDatabaseTableView(m_ui->tableWidgetViscosity);

	m_ui->tableWidgetViscosity->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	m_ui->tableWidgetViscosity->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);

	m_ui->tableWidgetViscosity->setSortingEnabled(false);
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

	// for built-ins, disable editing/make read-only
	bool isEditable = !fluid->m_builtIn;
	m_ui->lineEditName->setReadOnly(!isEditable);
	m_ui->pushButtonComponentColor->setReadOnly(!isEditable);
	m_ui->lineEditDensity->setReadOnly(!isEditable);
	m_ui->lineEditHeatCapacity->setReadOnly(!isEditable);
	m_ui->lineEditThermalConductivity->setReadOnly(!isEditable);

	// populate table widget with properties
	m_ui->tableWidgetViscosity->clearContents();
	IBK::LinearSpline &spline = m_currentFluid->m_kinematicViscosity.m_values;
	m_ui->tableWidgetViscosity->setRowCount((int)spline.size());

	m_ui->tableWidgetViscosity->blockSignals(true);
	for (unsigned int i=0; i<spline.size(); ++i) {

		QTableWidgetItem * item = new QTableWidgetItem(QString("%L1").arg(spline.x()[i]));
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		if (!isEditable)
			item->setFlags(item->flags() &  ~Qt::ItemIsEditable);
		m_ui->tableWidgetViscosity->setItem((int)i, 0, item);

		QTableWidgetItem * item2 = new QTableWidgetItem(QString("%L1").arg(spline.y()[i]));
		item2->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		if (!isEditable)
			item2->setFlags(item2->flags() &  ~Qt::ItemIsEditable);
		m_ui->tableWidgetViscosity->setItem((int)i, 1, item2);

	}
	m_ui->tableWidgetViscosity->blockSignals(false);
	m_ui->tableWidgetViscosity->resizeColumnsToContents();
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
	if (m_currentFluid->m_color != m_ui->pushButtonComponentColor->color()) {
		m_currentFluid->m_color = m_ui->pushButtonComponentColor->color();
		m_db->m_fluids.m_modified = true;
		m_dbModel->setItemModified(m_currentFluid->m_id); // tell model that we changed the data
	}
}

void SVDBNetworkFluidEditWidget::on_lineEditDensity_editingFinished()
{
	if (m_ui->lineEditDensity->isValid()){
		VICUS::KeywordList::setParameter(m_currentFluid->m_para, "NetworkFluid::para_t", VICUS::NetworkFluid::P_Density,
										 m_ui->lineEditDensity->value());
		m_db->m_fluids.m_modified = true;
		m_dbModel->setItemModified(m_currentFluid->m_id); // tell model that we changed the data
	}
}

void SVDBNetworkFluidEditWidget::on_lineEditHeatCapacity_editingFinished()
{
	if (m_ui->lineEditHeatCapacity->isValid()){
		VICUS::KeywordList::setParameter(m_currentFluid->m_para, "NetworkFluid::para_t", VICUS::NetworkFluid::P_HeatCapacity,
										 m_ui->lineEditHeatCapacity->value());
		m_db->m_fluids.m_modified = true;
		m_dbModel->setItemModified(m_currentFluid->m_id); // tell model that we changed the data
	}
}

void SVDBNetworkFluidEditWidget::on_lineEditThermalConductivity_editingFinished()
{
	if (m_ui->lineEditThermalConductivity->isValid()){
		VICUS::KeywordList::setParameter(m_currentFluid->m_para, "NetworkFluid::para_t", VICUS::NetworkFluid::P_Conductivity,
										 m_ui->lineEditThermalConductivity->value());
		m_db->m_fluids.m_modified = true;
		m_dbModel->setItemModified(m_currentFluid->m_id); // tell model that we changed the data
	}
}
