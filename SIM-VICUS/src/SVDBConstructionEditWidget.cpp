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

#include "SVDBConstructionEditWidget.h"
#include "ui_SVDBConstructionEditWidget.h"

#include <QtExt_ConstructionLayer.h>
#include <QtExt_ConstructionView.h>
#include <QtExt_ConstructionViewWidget.h>
#include <QtExt_Locale.h>
#include <QtExt_LanguageHandler.h>
#include <SV_Conversions.h>

#include <VICUS_KeywordListQt.h>

#include "SVStyle.h"
#include "SVConstants.h"
#include "SVDBConstructionTableModel.h"
#include "SVDatabaseEditDialog.h"
#include "SVMainWindow.h"

SVDBConstructionEditWidget::SVDBConstructionEditWidget(QWidget * parent) :
	SVAbstractDatabaseEditWidget(parent),
	m_ui(new Ui::SVDBConstructionEditWidget),
	m_current(nullptr)
{
	m_ui->setupUi(this);
	layout()->setMargin(0);
	m_ui->gridLayout->setMargin(4);
	m_ui->verticalLayout->setMargin(4);

	m_ui->lineEditName->initLanguages(QtExt::LanguageHandler::instance().langId().toStdString(), THIRD_LANGUAGE, true);
	m_ui->lineEditName->setDialog3Caption(tr("Construction identification name"));

	m_ui->tableWidget->setColumnCount(5);
	SVStyle::formatDatabaseTableView(m_ui->tableWidget);
	m_ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectItems);
	m_ui->tableWidget->setSortingEnabled(false);

	QStringList headerLabels;
	headerLabels << tr("Material") << tr("Width [cm]") << tr("rho [kg/m3]") << tr("cT [J/kgK]") << tr("lambda [W/mK]");
	m_ui->tableWidget->setHorizontalHeaderLabels(headerLabels);

	m_ui->tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
	for (int i=1; i<4; ++i) {
		m_ui->tableWidget->horizontalHeader()->setSectionResizeMode(i, QHeaderView::ResizeToContents);
//		m_ui->tableWidget->horizontalHeader()->resizeSection(i, fm.boundingRect(headerLabels[i]).width()+ 8);
	}
//	m_ui->tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);

	m_ui->tableWidget->horizontalHeader()->resizeSection(1, 80);

	// *** populate combo boxes ***

	m_ui->comboBoxInsulationKind->blockSignals(true);
	for (int i=0; i<=VICUS::Construction::NUM_IK; ++i) {
		QString description = VICUS::KeywordListQt::Description("Construction::InsulationKind", i);
		m_ui->comboBoxInsulationKind->addItem(description, i);
	}
	m_ui->comboBoxInsulationKind->blockSignals(false);

	m_ui->comboBoxMaterialKind->blockSignals(true);
	for (int i=0; i<=VICUS::Construction::NUM_MK; ++i) {
		QString description = VICUS::KeywordListQt::Description("Construction::MaterialKind", i);
		m_ui->comboBoxMaterialKind->addItem(description, i);
	}
	m_ui->comboBoxMaterialKind->blockSignals(false);

	m_ui->comboBoxConstructionUsage->blockSignals(true);
	for(int i=0; i<=VICUS::Construction::NUM_UT; ++i) {
		QString description = VICUS::KeywordListQt::Description("Construction::UsageType", i);
		m_ui->comboBoxConstructionUsage->addItem(description, i);
	}
	m_ui->comboBoxConstructionUsage->blockSignals(false);


	// for changing thickness
	connect(m_ui->tableWidget, SIGNAL(itemChanged(QTableWidgetItem *)),
		this, SLOT(tableItemChanged(QTableWidgetItem *)));
	// for update construction view selection if table selection has changed
	connect(m_ui->tableWidget, SIGNAL(itemClicked(QTableWidgetItem*)),
			this, SLOT(tableItemClicked(QTableWidgetItem *)));
	connect(m_ui->tableWidget, SIGNAL(cellDoubleClicked(int,int)),
			this, SLOT(onCellDoubleClicked(int,int)));



	// for update table selection if construction view selection has changed
	connect(m_ui->widgetConstructionView, SIGNAL(layerSelected(int)), this, SLOT(constructionViewlayerSelected(int)));

	// connect edit buttons from construction view
	connect(m_ui->widgetConstructionView, SIGNAL(assignMaterial(int)), this, SLOT(constructionViewAssign_material(int)));
	connect(m_ui->widgetConstructionView, SIGNAL(insertLayer(int,bool)), this, SLOT(constructionViewInsert_layer(int,bool)));
	connect(m_ui->widgetConstructionView, SIGNAL(moveLayer(int,bool)), this, SLOT(constructionViewMove_layer(int,bool)));
	connect(m_ui->widgetConstructionView, SIGNAL(removelayer(int)), this, SLOT(constructionViewRemove_layer(int)));

	setMinimumWidth(800); /// TODO : this should be font-size dependent?

	updateInput(-1); // update widget for status "nothing selected"
}


SVDBConstructionEditWidget::~SVDBConstructionEditWidget() {
	delete m_ui;
}


void SVDBConstructionEditWidget::setup(SVDatabase * db, SVAbstractDatabaseTableModel * dbModel) {
	m_db = db;
	m_dbModel = dynamic_cast<SVDBConstructionTableModel*>(dbModel);
}


void SVDBConstructionEditWidget::updateInput(int id) {
	m_current = nullptr; // disable edit triggers
	if (id == -1) {
		// disable all controls
		setEnabled(false);

		// clear all inputs

		m_ui->lineEditName->setString(IBK::MultiLanguageString());
		m_ui->lineEditUValue->setText("");
		m_ui->spinBoxLayerCount->setValue(0);
		m_ui->tableWidget->setRowCount(0); // Must be called after spinBoxLayerCount->setValue(0); !

		// disable the line edits (they change background color)
		m_ui->lineEditName->setEnabled(false);

		m_ui->comboBoxMaterialKind->setCurrentIndex(-1);
		m_ui->comboBoxInsulationKind->setCurrentIndex(-1);
		m_ui->comboBoxConstructionUsage->setCurrentIndex(-1);

		m_ui->widgetConstructionView->clear();
		return;
	}
	// re-enable all controls
	setEnabled(true);

	// retrieve selected construction from DB
	VICUS::Construction * con = const_cast<VICUS::Construction *>(m_db->m_constructions[(unsigned int)id]);
	m_current = con;

	// now update the GUI controls

	// construction name and layer count
	m_ui->lineEditName->setString(con->m_displayName);
	int n = std::max<int>(1, con->m_materialLayers.size());
	m_ui->spinBoxLayerCount->setValue(n);

	// kinds
	int indexIK = m_ui->comboBoxInsulationKind->findData(con->m_insulationKind);
	if (indexIK == -1)
		indexIK = VICUS::Construction::NUM_IK;
	m_ui->comboBoxInsulationKind->setCurrentIndex(indexIK);
	int indexMK = m_ui->comboBoxMaterialKind->findData(con->m_materialKind);
	if (indexMK == -1)
		indexMK = VICUS::Construction::NUM_MK;
	m_ui->comboBoxMaterialKind->setCurrentIndex(indexMK);
	int indexUT = m_ui->comboBoxConstructionUsage->findData(con->m_usageType);
	if(indexUT == -1)
		indexUT = VICUS::Construction::NUM_UT;
	m_ui->comboBoxConstructionUsage->setCurrentIndex(indexUT);

	// update read-only/enabled states
	m_ui->lineEditName->setEnabled(!con->m_builtIn);
	m_ui->spinBoxLayerCount->setEnabled(!con->m_builtIn);
	m_ui->comboBoxInsulationKind->setEnabled(!con->m_builtIn);
	m_ui->comboBoxMaterialKind->setEnabled(!con->m_builtIn);
	m_ui->comboBoxConstructionUsage->setEnabled(!con->m_builtIn);

	// set palette
	QPalette pal;
	if (con->m_builtIn)
		pal.setColor(QPalette::Base, SVStyle::instance().m_readOnlyEditFieldBackground);
	m_ui->spinBoxLayerCount->setPalette(pal);

	on_spinBoxLayerCount_valueChanged(n);
	updateTable();
	updateConstructionView();
}


void SVDBConstructionEditWidget::updateTable() {
	Q_ASSERT(m_current != nullptr);
	int n = m_ui->spinBoxLayerCount->value();

	if (n != m_ui->tableWidget->rowCount()+2) // mind: one extra on top and bottom for side indicators
		on_spinBoxLayerCount_valueChanged(n);
	m_ui->tableWidget->blockSignals(true);

	// first table widget item is always "Side A"

	// fill first row
	QFont f = m_ui->tableWidget->font();
	f.setItalic(true);
	for (int i=0; i<m_ui->tableWidget->columnCount(); ++i) {
		QTableWidgetItem * item = new QTableWidgetItem();
		if (i == 0)
			item->setText(tr("Side A"));
		item->setFlags(Qt::ItemFlags()); // disabled
		item->setFont(f);
		m_ui->tableWidget->setItem(0,i,item);
	}

	for (int i=0; i<(int)m_current->m_materialLayers.size(); ++i) {
		const VICUS::MaterialLayer & layer = m_current->m_materialLayers[i];
		const VICUS::Material * mat = m_db->m_materials[layer.m_idMaterial];
		if (mat != nullptr) {
			QTableWidgetItem * item = new QTableWidgetItem(QtExt::MultiLangString2QString(mat->m_displayName));
			if (m_current->m_builtIn) {
				item->setFlags(Qt::ItemIsEnabled);
				item->setBackground(QBrush(SVStyle::instance().m_readOnlyEditFieldBackground));
			}
			else {
				item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			}
			m_ui->tableWidget->setItem(i+1,0,item);

			item = new QTableWidgetItem(QString("%L1").arg(layer.m_thickness.value*100, 0, 'f', 1)); // thickness in cm
			item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
			if (m_current->m_builtIn) {
				item->setFlags(Qt::ItemIsEnabled);
				item->setBackground(QBrush(SVStyle::instance().m_readOnlyEditFieldBackground));
			}
			else {
				item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
			}
			m_ui->tableWidget->setItem(i+1,1,item);

			item = new QTableWidgetItem(QString("%L1").arg(mat->m_para[VICUS::Material::P_Density].value, 0, 'f', 0));
			item->setFlags(Qt::ItemIsEnabled);
			item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
			item->setBackground(QBrush(SVStyle::instance().m_readOnlyEditFieldBackground));
			m_ui->tableWidget->setItem(i+1,2,item);
			item = new QTableWidgetItem(QString("%L1").arg(mat->m_para[VICUS::Material::P_HeatCapacity].value, 0, 'f', 0));
			item->setFlags(Qt::ItemIsEnabled);
			item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
			item->setBackground(QBrush(SVStyle::instance().m_readOnlyEditFieldBackground));
			m_ui->tableWidget->setItem(i+1,3,item);
			item = new QTableWidgetItem(QString("%L1").arg(mat->m_para[VICUS::Material::P_Conductivity].value, 0, 'f', 4));
			item->setFlags(Qt::ItemIsEnabled);
			item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
			item->setBackground(QBrush(SVStyle::instance().m_readOnlyEditFieldBackground));
			m_ui->tableWidget->setItem(i+1,4,item);
		}
		else {
			QTableWidgetItem * item = new QTableWidgetItem(tr("<select material>"));
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
			m_ui->tableWidget->setItem(i+1,0,item);
			item = new QTableWidgetItem(QString("%L1").arg(layer.m_thickness.value*100, 0, 'f', 1));
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
			m_ui->tableWidget->setItem(i+1,1,item);
			item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

			item = new QTableWidgetItem("");
			item->setFlags(Qt::ItemIsEnabled);
			m_ui->tableWidget->setItem(i+1,2,item);
			item->setBackground(QBrush(SVStyle::instance().m_readOnlyEditFieldBackground));
			item = new QTableWidgetItem("");
			item->setFlags(Qt::ItemIsEnabled);
			m_ui->tableWidget->setItem(i+1,3,item);
			item->setBackground(QBrush(SVStyle::instance().m_readOnlyEditFieldBackground));
			item = new QTableWidgetItem("");
			item->setFlags(Qt::ItemIsEnabled);
			m_ui->tableWidget->setItem(i+1,4,item);
			item->setBackground(QBrush(SVStyle::instance().m_readOnlyEditFieldBackground));
		}
	}

	// fill last row
	for (int i=0; i<m_ui->tableWidget->columnCount(); ++i) {
		QTableWidgetItem * item = new QTableWidgetItem();
		if (i == 0)
			item->setText(tr("Side B"));
		item->setFlags(Qt::ItemFlags()); // disabled
		item->setFont(f);
		m_ui->tableWidget->setItem(m_current->m_materialLayers.size()+1,i,item);
	}

	m_ui->tableWidget->blockSignals(false);
	// update UValue and thermal mass
	updateUValue();
	m_ui->tableWidget->resizeColumnToContents(0);
	m_ui->tableWidget->resizeColumnToContents(2);
	m_ui->tableWidget->resizeColumnToContents(3);
	m_ui->tableWidget->resizeColumnToContents(4);
}


void SVDBConstructionEditWidget::updateConstructionView() {
	m_ui->widgetConstructionView->clear();
	Q_ASSERT(m_current != nullptr);

	QVector<QtExt::ConstructionLayer> layers;
	for (unsigned int i=0; i<m_current->m_materialLayers.size(); ++i) {
		QtExt::ConstructionLayer layer;
		unsigned int matID = m_current->m_materialLayers[i].m_idMaterial;
		const VICUS::Material * mat = m_db->m_materials[matID];
		if (mat != nullptr) {
			layer.m_name = QString::fromStdString(mat->m_displayName("de", true));
//			layer.m_color = mat->m_color;
		}
		else {
			layer.m_name = tr("<select material>");
		}

		layer.m_width = m_current->m_materialLayers[i].m_thickness.value;
//		if (!layer.m_color.isValid())
			layer.m_color = QtExt::ConstructionView::ColorList[i % 12];
		layer.m_id = (int)matID;
		layers.push_back(layer);
	}
	if(SVSettings::instance().m_theme == SVSettings::TT_White)
		m_ui->widgetConstructionView->setBackground(Qt::white);
	else
		m_ui->widgetConstructionView->setBackground(Qt::black);
	m_ui->widgetConstructionView->setData(layers, m_current->m_builtIn, tr("Side A"), tr("Side B"),
										  QtExt::ConstructionGraphicsScene::VI_All);
}


void SVDBConstructionEditWidget::on_spinBoxLayerCount_valueChanged(int val) {
	if (m_current == nullptr) return; // m_current is nullptr, when nothing is selected and controls are defaulted to "empty"
	m_ui->tableWidget->setRowCount(val+2);

	// only update, if number of layers has changed
	if ((int)m_current->m_materialLayers.size() != val) {
		// update content of table widget based on data in m_current
		while (m_current->m_materialLayers.size() < static_cast<unsigned int>(val)) {
			unsigned int defaultMaterialId = 0;
			if (!m_db->m_materials.empty())
				defaultMaterialId = m_db->m_materials.begin()->first;
			VICUS::MaterialLayer matLay(0.01, defaultMaterialId);
			m_current->m_materialLayers.push_back(matLay);
		}
		// shrink vectors
		m_current->m_materialLayers.resize(val);
		modelModify();
		updateTable();
		updateConstructionView();
	}
}


void SVDBConstructionEditWidget::on_lineEditName_editingFinished() {
	Q_ASSERT(m_current != nullptr);

	if (m_current->m_displayName != m_ui->lineEditName->string()) {
		m_current->m_displayName = m_ui->lineEditName->string();
		modelModify();
	}
}

void SVDBConstructionEditWidget::on_comboBoxInsulationKind_currentIndexChanged(int) {
	if (m_current == nullptr) return; // m_current is nullptr, when nothing is selected and controls are defaulted to "empty"

	VICUS::Construction::InsulationKind ik = static_cast<VICUS::Construction::InsulationKind>(m_ui->comboBoxInsulationKind->currentData().toInt());
	if (ik != m_current->m_insulationKind) {
		m_current->m_insulationKind = ik;
		modelModify();
	}
}

void SVDBConstructionEditWidget::on_comboBoxMaterialKind_currentIndexChanged(int ) {
	if (m_current == nullptr) return; // m_current is nullptr, when nothing is selected and controls are defaulted to "empty"

	VICUS::Construction::MaterialKind mk = static_cast<VICUS::Construction::MaterialKind>(m_ui->comboBoxMaterialKind->currentData().toInt());
	if (mk != m_current->m_materialKind) {
		m_current->m_materialKind = mk;
		modelModify();
	}
}


void SVDBConstructionEditWidget::on_comboBoxConstructionUsage_currentIndexChanged(int ) {
	if (m_current == nullptr) return; // m_current is nullptr, when nothing is selected and controls are defaulted to "empty"

	VICUS::Construction::UsageType ut = static_cast<VICUS::Construction::UsageType>(m_ui->comboBoxConstructionUsage->currentData().toInt());
	if (ut != m_current->m_usageType) {
		m_current->m_usageType = ut;
		modelModify();
	}
}


// change layer thickness
void SVDBConstructionEditWidget::tableItemChanged(QTableWidgetItem * item) {
	Q_ASSERT(item->column() == 1);
	Q_ASSERT(m_current != nullptr);

	bool ok;
	double val = QtExt::Locale().toDouble(item->text(), &ok); // val in cm
	int row = item->row();
	unsigned int materialLayerIdx = (unsigned int)row - 1; // Mind: first row is "outside" marker
	Q_ASSERT(materialLayerIdx < m_current->m_materialLayers.size());
	if (!ok || val < 0.1) {
		if (!ok) {
			QTableWidgetItem * item2 = m_ui->tableWidget->item(row, 1);
			item2->setBackground(QBrush(SVStyle::instance().m_errorEditFieldBackground));
			item2->setToolTip(tr("Invalid number format, please enter a valid decimal number!"));
			//m_ui->tableWidget->setItem(row, 1, item2);
		}
		else {
			QTableWidgetItem * item2 = m_ui->tableWidget->item(row, 1);
			item2->setBackground(QBrush(SVStyle::instance().m_errorEditFieldBackground));
			item2->setToolTip(tr("Layer widths must be larger than 1 mm. Ignore smaller layers, "
								 "they will not affect the thermal storage mass!"));
			//m_ui->tableWidget->setItem(row, 1, item2);
		}
		updateUValue();
		m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
		return;
	}
	else {
		QTableWidgetItem * item2 = m_ui->tableWidget->item(row, 1);
		item2->setBackground(QBrush());
	}
	double valM = val / 100.0; // internal thickness in m
	// we only accept changes up to 0.1 mm as different
	if (!IBK::nearly_equal<4>(m_current->m_materialLayers[materialLayerIdx].m_thickness.value, valM)) {
		m_current->m_materialLayers[materialLayerIdx].m_thickness.value = valM;
		modelModify();
	}
	updateUValue();
	updateConstructionView();
}


void SVDBConstructionEditWidget::tableItemClicked(QTableWidgetItem * item) {
	// ignore all but the first two columns
	if (item->column() > 1)
		return;
	// first and last row are ignored
	if (item->row() == 0 || item->row() == m_ui->tableWidget->rowCount()-1)
		return;

	Q_ASSERT(m_current != nullptr);

	unsigned int row = item->row();
	m_ui->widgetConstructionView->selectLayer(row-1); // mind: first row is "outside" label
}


void SVDBConstructionEditWidget::onCellDoubleClicked(int row, int col) {
	if (col != 0)
		return;

	// first and last row are ignored
	if (row == 0 || row == m_ui->tableWidget->rowCount()-1)
		return;

	if (m_current == nullptr || m_current->m_builtIn)
		return;

	showMaterialSelectionDialog(row-1); // mind: first row is "outside" label
}


void SVDBConstructionEditWidget::updateUValue() {
	Q_ASSERT(m_current != nullptr);


	double UValue;
	bool validUValue = m_current->calculateUValue(UValue, m_db->m_materials, 0.13, 0.04);

	// write U-Value
	if (!m_ui->lineEditUValue->isEnabled()) {
		m_ui->lineEditUValue->setText("");
	}
	else {
		// problem calculating?
		if (!validUValue)
			m_ui->lineEditUValue->setText("---");
		else
			m_ui->lineEditUValue->setText(QString("%L1").arg(UValue, 0, 'f', 3));
	}
}


void SVDBConstructionEditWidget::showMaterialSelectionDialog(int index) {
	Q_ASSERT(index >= 0);

	// get material edit dialog (owned/managed by main window)
	SVDatabaseEditDialog * matSelect = SVMainWindow::instance().dbMaterialEditDialog();
	// ask to select a material
	unsigned int matId = matSelect->select(m_current->m_materialLayers[(unsigned int)index].m_idMaterial);
	if (matId == VICUS::INVALID_ID)
		return; // dialog was canceled, no change here
	if (matId != m_current->m_materialLayers[(unsigned int)index].m_idMaterial) {
		m_current->m_materialLayers[(unsigned int)index].m_idMaterial = matId;
		m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
	}
	updateTable();
	updateConstructionView();
}


void SVDBConstructionEditWidget::constructionViewlayerSelected(int index) {
	Q_ASSERT(index != -1);

	m_ui->tableWidget->selectRow(index);
}


void SVDBConstructionEditWidget::constructionViewAssign_material(int index) {
	Q_ASSERT(index >= 0 && index < (int)m_current->m_materialLayers.size());

	showMaterialSelectionDialog(index);
}


void SVDBConstructionEditWidget::constructionViewInsert_layer(int index, bool left) {
	Q_ASSERT(index >= 0 && index < (int)m_current->m_materialLayers.size());

	unsigned int defaultMaterialId = 0;
	if (!m_db->m_materials.empty())
		defaultMaterialId = m_db->m_materials.begin()->first;
	if (left) {
		m_current->m_materialLayers.insert(m_current->m_materialLayers.begin()+index,
										   VICUS::MaterialLayer(0.1, defaultMaterialId));
	}
	else {
		m_current->m_materialLayers.insert(m_current->m_materialLayers.begin()+index+1,
										   VICUS::MaterialLayer(0.1, defaultMaterialId));
	}

	m_ui->spinBoxLayerCount->setValue((int)m_current->m_materialLayers.size());
	modelModify();
	updateTable();
	updateConstructionView();
}


void SVDBConstructionEditWidget::constructionViewRemove_layer(int index) {
	Q_ASSERT(index >= 0 && index < (int)m_current->m_materialLayers.size());

	m_current->m_materialLayers.erase(m_current->m_materialLayers.begin()+index);

	m_ui->spinBoxLayerCount->setValue((int)m_current->m_materialLayers.size());
	modelModify();
	updateTable();
	updateConstructionView();
}


void SVDBConstructionEditWidget::constructionViewMove_layer(int index, bool left) {
	/// FIXME: these checks should be asserts
	if (index == -1)
		return;
	if (index == 0 && left)
		return;
	if ((index == (int)m_current->m_materialLayers.size() - 1) && !left)
		return;

	if (left) {
		std::iter_swap(m_current->m_materialLayers.begin()+index, m_current->m_materialLayers.begin()+index-1);
	}
	else {
		std::iter_swap(m_current->m_materialLayers.begin()+index, m_current->m_materialLayers.begin()+index+1);
	}

	modelModify();
	updateTable();
	updateConstructionView();
}



void SVDBConstructionEditWidget::modelModify() {
	m_db->m_constructions.m_modified = true;
	m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
}
