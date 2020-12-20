#include "SVDBConstructionEditWidget.h"

#include <QHeaderView>
#include <QDebug>
#include <QMessageBox>
#include <QCloseEvent>
#include <QFileDialog>

#include "ui_SVDBConstructionEditWidget.h"

#include <QtExt_ConstructionLayer.h>
#include <QtExt_ConstructionView.h>
#include <QtExt_ConstructionViewWidget.h>
#include <QtExt_Locale.h>

#include "SVStyle.h"

#include "SVDBConstructionTableModel.h"
//#include "MaterialSelectorDialog.h"

SVDBConstructionEditWidget::SVDBConstructionEditWidget(QWidget * parent) :
	QWidget(parent),
	m_ui(new Ui::SVDBConstructionEditWidget),
	m_current(nullptr)
{
	m_ui->setupUi(this);
	layout()->setMargin(0);

	m_ui->labelNameEn->setText("");
	m_ui->labelNameDe->setText("");
	m_ui->labelNameEn->setPixmap( QPixmap(":/gfx/icons/en.png") );
	m_ui->labelNameDe->setPixmap( QPixmap(":/gfx/icons/de.png") );

	m_ui->tableWidget->setColumnCount(5);
	QStringList headerLabels;
	headerLabels << tr("Material") << tr("Width [cm]") << tr("rho [kg/m3]") << tr("cT [J/kgK]") << tr("lambda [W/mK]");
	m_ui->tableWidget->setHorizontalHeaderLabels(headerLabels);
	m_ui->tableWidget->verticalHeader()->setVisible(false);

	/// TODO : move style adjustment to SVStyle class
	QFont f(m_ui->tableWidget->horizontalHeader()->font());
	f.setBold(true);
	QFontMetrics fm(f);
#if defined(Q_OS_MAC)
	int height = fm.lineSpacing()+ 6;
#else
	int height = fm.lineSpacing()+ 3;
#endif
	m_ui->tableWidget->verticalHeader()->setDefaultSectionSize(height);

	m_ui->tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
	for (int i=1; i<4; ++i) {
		m_ui->tableWidget->horizontalHeader()->setSectionResizeMode(i, QHeaderView::Fixed);
		m_ui->tableWidget->horizontalHeader()->resizeSection(i, fm.width(headerLabels[i])+ 8);
	}
	m_ui->tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);

	//	m_ui->tableWidget->horizontalHeader()->resizeSection(1, 100);
#if 0
	for(int i=0; i<=VICUS::Construction::Num_IK; ++i) {
		VICUS::ConstructionType::InsulationKind ik = static_cast<VICUS::ConstructionType::InsulationKind>(i);
		m_ui->comboBoxInsulationKind->addItem(VICUS::ConstructionType::insulationKindString(ik), ik);
	}
	m_ui->comboBoxInsulationKind->setCurrentIndex(VICUS::ConstructionType::Num_IK);

	for(int i=0; i<=VICUS::ConstructionType::Num_MK; ++i) {
		VICUS::ConstructionType::MaterialKind mk = static_cast<VICUS::ConstructionType::MaterialKind>(i);
		m_ui->comboBoxMaterialKind->addItem(VICUS::ConstructionType::materialKindString(mk), mk);
	}
	m_ui->comboBoxMaterialKind->setCurrentIndex(VICUS::ConstructionType::Num_MK);

	for(int i=0; i<=VICUS::ConstructionType::Num_CK; ++i) {
		VICUS::ConstructionType::ConstructionKind ck = static_cast<VICUS::ConstructionType::ConstructionKind>(i);
		m_ui->comboBoxConstructionKind->addItem(VICUS::ConstructionType::constructionKindString(ck), ck);
	}
	m_ui->comboBoxConstructionKind->setCurrentIndex(VICUS::ConstructionType::Num_CK);

	for(const QString& str : m_db->constructionKeys1()) {
		m_ui->comboBoxUserKey1->addItem(str);
	}
	m_ui->comboBoxUserKey1->setCurrentIndex(-1);

	for(const QString& str : m_db->constructionKeys2()) {
		m_ui->comboBoxUserKey2->addItem(str);
	}
	m_ui->comboBoxUserKey2->setCurrentIndex(-1);
#endif

	// register change events for user keys

	connect(m_ui->comboBoxUserKey1->lineEdit(), SIGNAL(editingFinished()), SLOT(onUserKey1EditingFinished()));
	connect(m_ui->comboBoxUserKey2->lineEdit(), SIGNAL(editingFinished()), SLOT(onUserKey2EditingFinished()));

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


void SVDBConstructionEditWidget::setup(SVDatabase * db, SVDBConstructionTableModel * dbModel) {
	m_db = db;
	m_dbModel = dbModel;
}


void SVDBConstructionEditWidget::updateInput(int id) {
	m_current = nullptr; // disable edit triggers
	// find the construction data object matching the index
	if (id == -1) {
		// clear all input and disable controls
		m_ui->lineEditNameEn->setText("");
		m_ui->lineEditNameDe->setText("");
		m_ui->lineEditUValue->setText("");
		m_ui->lineEditUValue->setText("");
		m_ui->spinBoxLayerCount->setValue(0);
		m_ui->lineEditNameEn->setReadOnly(true);
		m_ui->lineEditNameDe->setReadOnly(true);
		m_ui->spinBoxLayerCount->setEnabled(false);
		m_ui->spinBoxLayerCount->setValue(0);
		m_ui->tableWidget->setRowCount(0);
		return;
	}
	VICUS::Construction * conType = const_cast<VICUS::Construction *>(m_db->m_constructions[(unsigned int)id]);

	// now update the GUI controls

	// construction name and layer count
	m_ui->lineEditNameEn->setText(QString::fromStdString(conType->m_displayName("en")));
	m_ui->lineEditNameDe->setText(QString::fromStdString(conType->m_displayName("de")));
	int n = std::max<int>(1, conType->m_materialLayers.size());
	m_ui->spinBoxLayerCount->setValue(n);
#if 0
	// kinds
	int indexIK = m_ui->comboBoxInsulationKind->findData(conType->m_insulationKind);
	if(indexIK == -1)
		indexIK = VICUS::ConstructionType::Num_IK;
	m_ui->comboBoxInsulationKind->setCurrentIndex(indexIK);
	int indexMK = m_ui->comboBoxMaterialKind->findData(conType->m_materialKind);
	if(indexMK == -1)
		indexMK = VICUS::ConstructionType::Num_MK;
	m_ui->comboBoxMaterialKind->setCurrentIndex(indexMK);
	int indexCK = m_ui->comboBoxConstructionKind->findData(conType->m_constructionKind);
	if(indexCK == -1)
		indexCK = VICUS::ConstructionType::Num_CK;
	m_ui->comboBoxConstructionKind->setCurrentIndex(indexCK);
	int indexK1 = m_ui->comboBoxUserKey1->findText(conType->m_userKey1);
	if(indexK1 == -1) {
		if(!conType->m_userKey1.isEmpty()) {
			m_ui->comboBoxUserKey1->addItem(conType->m_userKey1);
			indexK1 = m_ui->comboBoxUserKey1->count() - 1;
		}
	}
	m_ui->comboBoxUserKey1->setCurrentIndex(indexK1);
	int indexK2 = m_ui->comboBoxUserKey2->findText(conType->m_userKey2);
	if(indexK2 == -1) {
		if(!conType->m_userKey2.isEmpty()) {
			m_ui->comboBoxUserKey2->addItem(conType->m_userKey2);
			indexK2 = m_ui->comboBoxUserKey2->count() - 1;
		}
	}
	m_ui->comboBoxUserKey2->setCurrentIndex(indexK2);

#endif

	// update read-only/enabled states
	m_ui->lineEditNameEn->setReadOnly(conType->m_builtIn);
	m_ui->lineEditNameDe->setReadOnly(conType->m_builtIn);
	m_ui->spinBoxLayerCount->setEnabled(!conType->m_builtIn);
	m_ui->comboBoxInsulationKind->setEnabled(!conType->m_builtIn);
	m_ui->comboBoxMaterialKind->setEnabled(!conType->m_builtIn);
	m_ui->comboBoxConstructionKind->setEnabled(!conType->m_builtIn);
	m_ui->comboBoxUserKey1->setEnabled(!conType->m_builtIn);
	m_ui->comboBoxUserKey2->setEnabled(!conType->m_builtIn);

	// set palette
	QPalette pal;
	if (conType->m_builtIn)
		pal.setColor(QPalette::Base, SVStyle::instance().m_readOnlyEditFieldBackground);
	m_ui->lineEditNameEn->setPalette(pal);
	m_ui->lineEditNameDe->setPalette(pal);
	m_ui->spinBoxLayerCount->setPalette(pal);

	m_current = conType;
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

	for (unsigned int i=0; i<m_current->m_materialLayers.size(); ++i) {
		const VICUS::MaterialLayer & layer = m_current->m_materialLayers[i];
		const VICUS::Material * mat = m_db->m_materials[layer.m_matId];
		if (mat != nullptr) {
			QTableWidgetItem * item = new QTableWidgetItem(QString::fromStdString(mat->m_displayName("de", true)));
//			item->setData(IdRole, mat->m_id);
			if (m_current->m_builtIn) {
				item->setFlags(Qt::ItemIsEnabled);
				item->setBackground(QBrush(SVStyle::instance().m_readOnlyEditFieldBackground));
			}
			else {
//				item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
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
//			item->setData(IdRole, 0);
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
}


void SVDBConstructionEditWidget::updateConstructionView() {
	m_ui->widgetConstructionView->clear();
	Q_ASSERT(m_current != nullptr);

	QVector<QtExt::ConstructionLayer> layers;
	for (unsigned int i=0; i<m_current->m_materialLayers.size(); ++i) {
		QtExt::ConstructionLayer layer;
		unsigned int matID = m_current->m_materialLayers[i].m_matId;
		const VICUS::Material * mat = m_db->m_materials[matID];
		if (mat == nullptr) {
			layer.m_name = QString::fromStdString(mat->m_displayName("de", true));
			layer.m_color = mat->m_color;
		}
		else {
			layer.m_name = tr("<select material>");
		}

		layer.m_width = m_current->m_materialLayers[i].m_thickness.value;
		if (!layer.m_color.isValid())
			layer.m_color = QtExt::ConstructionView::ColorList[i % 12];
		layer.m_id = matID;
		layers.push_back(layer);
	}
	m_ui->widgetConstructionView->setData(layers, m_current->m_builtIn);
}


void SVDBConstructionEditWidget::on_spinBoxLayerCount_valueChanged(int val) {
	m_ui->tableWidget->setRowCount(val+2);
	if (m_current == nullptr)
		return; /// FIXME: this shouldn't be necessary, if signals are properly blocked during init
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
		m_db->m_constructions.m_modified = true;
		m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
		emit tableDataChanged();
		updateTable();
		updateConstructionView();
	}
}

void SVDBConstructionEditWidget::on_lineEditNameEn_editingFinished() {
	Q_ASSERT(m_current != nullptr);

	if (m_current->m_displayName("en") != m_ui->lineEditNameEn->text().toStdString()) {
		m_current->m_displayName.setString(m_ui->lineEditNameEn->text().toStdString(), "en");
		m_db->m_constructions.m_modified = true;
		m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
		emit tableDataChanged();
	}
}

void SVDBConstructionEditWidget::on_lineEditNameDe_editingFinished() {
	Q_ASSERT(m_current != nullptr);

	if (m_current->m_displayName("de") != m_ui->lineEditNameDe->text().toStdString()) {
		m_current->m_displayName.setString(m_ui->lineEditNameDe->text().toStdString(), "de");
		m_db->m_constructions.m_modified = true;
		m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
		emit tableDataChanged();
	}
}

void SVDBConstructionEditWidget::on_comboBoxInsulationKind_currentIndexChanged(int ) {
	Q_ASSERT(m_current != nullptr);


//	VICUS::ConstructionType::InsulationKind ik = static_cast<VICUS::ConstructionType::InsulationKind>(m_ui->comboBoxInsulationKind->currentData().toInt());
//	if(ik != m_current->m_insulationKind) {
//		m_current->m_insulationKind = ik;
//		m_db->setModified(true);
//		m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
//		emit tableDataChanged();
//	}
}

void SVDBConstructionEditWidget::on_comboBoxMaterialKind_currentIndexChanged(int ) {
	Q_ASSERT(m_current != nullptr);


//	VICUS::ConstructionType::MaterialKind mk = static_cast<VICUS::ConstructionType::MaterialKind>(m_ui->comboBoxMaterialKind->currentData().toInt());
//	if(mk != m_current->m_materialKind) {
//		m_current->m_materialKind = mk;
//		m_db->setModified(true);
//		m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
//		emit tableDataChanged();
//	}
}

void SVDBConstructionEditWidget::on_comboBoxConstructionKind_currentIndexChanged(int ) {
	Q_ASSERT(m_current != nullptr);

//	VICUS::ConstructionType::ConstructionKind ck = static_cast<VICUS::ConstructionType::ConstructionKind>(m_ui->comboBoxConstructionKind->currentData().toInt());
//	if(ck != m_current->m_constructionKind) {
//		m_current->m_constructionKind = ck;
//		m_db->setModified(true);
//		m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
//		emit tableDataChanged();
//	}
}

void SVDBConstructionEditWidget::on_comboBoxUserKey1_currentIndexChanged(int ) {
	Q_ASSERT(m_current != nullptr);


//	QString key1 = m_ui->comboBoxUserKey1->currentText();
//	if(key1 != m_current->m_userKey1) {
//		m_current->m_userKey1 = key1;
//		m_db->setModified(true);
//		m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
//	}
}

void SVDBConstructionEditWidget::on_comboBoxUserKey2_currentIndexChanged(int ) {
	Q_ASSERT(m_current != nullptr);

//	QString key2 = m_ui->comboBoxUserKey2->currentText();
//	if(key2 != m_current->m_userKey2) {
//		m_current->m_userKey2 = key2;
//		m_db->setModified(true);
//		m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
//	}
}

void SVDBConstructionEditWidget::onUserKey1EditingFinished() {
	Q_ASSERT(m_current != nullptr);

//	QString key1 = m_ui->comboBoxUserKey1->currentText();
//	if(key1 != m_current->m_userKey1) {
//		m_current->m_userKey1 = key1;
//		m_db->setModified(true);
//		m_db->addConstructionUserkey1(key1);
//		m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
//	}
}

void SVDBConstructionEditWidget::onUserKey2EditingFinished() {
	Q_ASSERT(m_current != nullptr);

//	QString key2 = m_ui->comboBoxUserKey2->currentText();
//	if(key2 != m_current->m_userKey2) {
//		m_current->m_userKey2 = key2;
//		m_db->setModified(true);
//		m_db->addConstructionUserkey2(key2);
//		m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
//	}
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
		m_db->m_constructions.m_modified = true;
		m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
	}
	updateUValue();
	updateConstructionView();
}


void SVDBConstructionEditWidget::tableItemClicked(QTableWidgetItem * item) {
	// ignore all but the first two columns
	if (item->column() > 1)
		return;

	Q_ASSERT(m_current != nullptr);

	unsigned int row = item->row();
	m_ui->widgetConstructionView->selectLayer(row-1); // mind: first row is "outside" label
}


void SVDBConstructionEditWidget::onCellDoubleClicked(int row, int col) {
	if (col != 0)
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
//	MaterialSelectorDialog matSelect(m_db, this);
//	matSelect.selectMaterial(m_current->m_materialIds[index]);
//	int res = matSelect.exec();
//	if( res == QDialog::Accepted) {
//		int matid = matSelect.selectedMaterialID();
//		if(matid > -1) {
//			m_current->m_materialIds[index] = matid;
//			m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
//			updateTable();
//			updateConstructionView();
//		}
//	}
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
	m_db->m_constructions.m_modified = true;
	m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
	updateTable();
	updateConstructionView();
}


void SVDBConstructionEditWidget::constructionViewRemove_layer(int index) {
	Q_ASSERT(index >= 0 && index < (int)m_current->m_materialLayers.size());

	m_current->m_materialLayers.erase(m_current->m_materialLayers.begin()+index);

	m_ui->spinBoxLayerCount->setValue((int)m_current->m_materialLayers.size());
	m_db->m_constructions.m_modified = true;
	m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
	updateTable();
	updateConstructionView();
}


void SVDBConstructionEditWidget::constructionViewMove_layer(int index, bool left) {
	/// FIXME: these checks should be asserts
	if(index == -1)
		return;
	if(index == 0 && left)
		return;
	if(index == m_current->m_materialLayers.size() - 1 && !left)
		return;

	if (left) {
		std::iter_swap(m_current->m_materialLayers.begin()+index, m_current->m_materialLayers.begin()+index-1);
	}
	else {
		std::iter_swap(m_current->m_materialLayers.begin()+index, m_current->m_materialLayers.begin()+index+1);
	}

	m_db->m_constructions.m_modified = true;
	m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
	updateTable();
	updateConstructionView();
}

