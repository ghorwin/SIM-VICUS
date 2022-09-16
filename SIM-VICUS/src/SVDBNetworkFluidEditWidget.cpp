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

#include "SVDBNetworkFluidEditWidget.h"
#include "ui_SVDBNetworkFluidEditWidget.h"

#include <NANDRAD_HydraulicFluid.h>

#include <QtExt_LanguageHandler.h>
#include <QtExt_Locale.h>

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

	m_ui->pushButtonColor->setDontUseNativeDialog(SVSettings::instance().m_dontUseNativeDialogs);

	// table widget
	m_ui->tableWidgetViscosity->blockSignals(true);
	m_ui->tableWidgetViscosity->setColumnCount(2);
	m_ui->tableWidgetViscosity->verticalHeader()->setVisible(false);
	m_ui->tableWidgetViscosity->setHorizontalHeaderItem(0, new QTableWidgetItem(tr("Temperature [C]")));
	m_ui->tableWidgetViscosity->setHorizontalHeaderItem(1, new QTableWidgetItem(tr("Kinematic Viscosity [m²/s]")));
	SVStyle::formatDatabaseTableView(m_ui->tableWidgetViscosity);
	m_ui->tableWidgetViscosity->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	m_ui->tableWidgetViscosity->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
	m_ui->tableWidgetViscosity->setSortingEnabled(false);
	m_ui->tableWidgetViscosity->blockSignals(false);
}


SVDBNetworkFluidEditWidget::~SVDBNetworkFluidEditWidget() {
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

	m_ui->pushButtonColor->blockSignals(true);
	m_ui->pushButtonColor->setColor(m_currentFluid->m_color);
	m_ui->pushButtonColor->blockSignals(false);

	// for built-ins, disable editing/make read-only
	bool isEditable = !fluid->m_builtIn;
	m_ui->lineEditName->setReadOnly(!isEditable);
	m_ui->pushButtonColor->setReadOnly(!isEditable);
	m_ui->lineEditDensity->setReadOnly(!isEditable);
	m_ui->lineEditHeatCapacity->setReadOnly(!isEditable);
	m_ui->lineEditThermalConductivity->setReadOnly(!isEditable);
	m_ui->toolButtonAddViscosityPoint->setEnabled(isEditable);
	m_ui->toolButtonRemoveViscosityPoint->setEnabled(isEditable && m_currentFluid->m_kinematicViscosity.m_values.size()>2);


	// populate table widget with properties
	m_ui->tableWidgetViscosity->blockSignals(true);
	m_ui->tableWidgetViscosity->clearContents();
	IBK::LinearSpline &spline = m_currentFluid->m_kinematicViscosity.m_values;
	m_ui->tableWidgetViscosity->setRowCount((int)spline.size());

	for (unsigned int i=0; i<spline.size(); ++i) {

		QTableWidgetItem * item = new QTableWidgetItem(QString("%L1").arg(spline.x()[i]));
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
		if (!isEditable)
			item->setFlags(item->flags() &  ~Qt::ItemIsEditable);
		m_ui->tableWidgetViscosity->setItem((int)i, 0, item);

		QTableWidgetItem * item2 = new QTableWidgetItem(QString("%L1").arg(spline.y()[i]));
		item2->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
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
		modelModify();
	}
}

void SVDBNetworkFluidEditWidget::on_pushButtonColor_colorChanged()
{
	if (m_currentFluid->m_color != m_ui->pushButtonColor->color()) {
		m_currentFluid->m_color = m_ui->pushButtonColor->color();
		modelModify();
	}
}

void SVDBNetworkFluidEditWidget::on_lineEditDensity_editingFinished()
{
	if (m_ui->lineEditDensity->isValid()){
		VICUS::KeywordList::setParameter(m_currentFluid->m_para, "NetworkFluid::para_t", VICUS::NetworkFluid::P_Density,
										 m_ui->lineEditDensity->value());
		modelModify();
	}
}

void SVDBNetworkFluidEditWidget::on_lineEditHeatCapacity_editingFinished()
{
	if (m_ui->lineEditHeatCapacity->isValid()){
		VICUS::KeywordList::setParameter(m_currentFluid->m_para, "NetworkFluid::para_t", VICUS::NetworkFluid::P_HeatCapacity,
										 m_ui->lineEditHeatCapacity->value());
		modelModify();
	}
}

void SVDBNetworkFluidEditWidget::on_lineEditThermalConductivity_editingFinished()
{
	if (m_ui->lineEditThermalConductivity->isValid()){
		VICUS::KeywordList::setParameter(m_currentFluid->m_para, "NetworkFluid::para_t", VICUS::NetworkFluid::P_Conductivity,
										 m_ui->lineEditThermalConductivity->value());
		modelModify();
	}
}

void SVDBNetworkFluidEditWidget::modelModify() {
	m_db->m_fluids.m_modified = true;
	m_dbModel->setItemModified(m_currentFluid->m_id); // tell model that we changed the data
}


void SVDBNetworkFluidEditWidget::modifyViscosity() {

	std::vector<double> x;
	std::vector<double> y;
	for (int row=0; row<m_ui->tableWidgetViscosity->rowCount(); ++row) {
		QTableWidgetItem *item = m_ui->tableWidgetViscosity->item(row, 0);
		x.push_back(IBK::string2val<double>(item->text().toStdString()));
		QTableWidgetItem *item2 = m_ui->tableWidgetViscosity->item(row, 1);
		y.push_back(IBK::string2val<double>(item2->text().toStdString()));
	}

	// we need to check if the spline can be set
	bool ok=true;
	try {
		IBK::LinearSpline spl;
		spl.setValues(x,y);
	}  catch (IBK::Exception &ex) {
		QMessageBox::critical(this, tr("Error"), ex.what());
		ok = false;
	}

	if (ok) {
		m_currentFluid->m_kinematicViscosity.m_values.setValues(x, y);
		modelModify();
	}

	updateInput((int)m_currentFluid->m_id);
}


void SVDBNetworkFluidEditWidget::on_toolButtonAddViscosityPoint_clicked() {

	m_ui->tableWidgetViscosity->blockSignals(true);

	int rows = m_ui->tableWidgetViscosity->rowCount();
	std::string lastTemperatureStr = m_ui->tableWidgetViscosity->item(rows-1,0)->text().toStdString();
	double lastTemperature = IBK::string2val<double>(lastTemperatureStr);

	m_ui->tableWidgetViscosity->setRowCount(rows+1);
	QTableWidgetItem * item = new QTableWidgetItem(QString("%1").arg(lastTemperature + 10));
	item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
	m_ui->tableWidgetViscosity->setItem(rows,0,item);
	QTableWidgetItem * item2 = new QTableWidgetItem("1e-6");
	item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
	m_ui->tableWidgetViscosity->setItem(rows,1,item2);

	m_ui->tableWidgetViscosity->blockSignals(false);

	modifyViscosity();
}


void SVDBNetworkFluidEditWidget::on_toolButtonRemoveViscosityPoint_clicked() {
	int rows = m_ui->tableWidgetViscosity->rowCount();
	if (rows>2) {
		m_ui->tableWidgetViscosity->blockSignals(true);
		m_ui->tableWidgetViscosity->setRowCount(rows-1);
		m_ui->tableWidgetViscosity->blockSignals(false);
		modifyViscosity();
	}
}


void SVDBNetworkFluidEditWidget::on_tableWidgetViscosity_cellChanged(int row, int column) {

	QTableWidgetItem *item = m_ui->tableWidgetViscosity->item(row, column);
	QString text = item->text();
	bool ok = false;
	double val=0;

	// check number
	if (!text.isEmpty()) {
		val = QtExt::Locale().toDouble(text, &ok);
		if (!ok)
			val = text.toDouble(&ok);
		// no negative viscosity
		if (column==1)
			ok = val>0;
	}

	// set value back
	if (ok) {
		item->setText(QString("%1").arg(val));
		modifyViscosity();
	} else
		updateInput((int)m_currentFluid->m_id);
}

