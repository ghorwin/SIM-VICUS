#include "SVDBWindowGlazingSystemEditWidget.h"
#include "ui_SVDBWindowGlazingSystemEditWidget.h"

#include <QSortFilterProxyModel>

#include <VICUS_KeywordList.h>
#include <VICUS_KeywordListQt.h>

#include <IBK_physics.h>

#include <QtExt_LanguageHandler.h>
#include <QtExt_Conversions.h>

#include "SVSettings.h"
#include "SVDBWindowGlazingSystemTableModel.h"
#include "SVDatabaseEditDialog.h"
#include "SVMainWindow.h"
#include "SVConstants.h"
#include "SVStyle.h"

SVDBWindowGlazingSystemEditWidget::SVDBWindowGlazingSystemEditWidget(QWidget *parent) :
	SVAbstractDatabaseEditWidget(parent),
	m_ui(new Ui::SVDBWindowGlazingSystemEditWidget)
{
	m_ui->setupUi(this);
	m_ui->masterLayout->setMargin(4);

	// style the table widget

	m_ui->lineEditName->initLanguages(QtExt::LanguageHandler::instance().langId().toStdString(), THIRD_LANGUAGE, true);
	m_ui->lineEditName->setDialog3Caption(tr("Window Glazing System"));

	//header elements

	// set period table column sizes

	//add header to periods table
	m_ui->tableWidgetSHGC->setColumnCount(2);
	m_ui->tableWidgetSHGC->setRowCount(10);
	// Note: valid column is self-explanatory and does not need a caption
	m_ui->tableWidgetSHGC->setHorizontalHeaderLabels(QStringList() << tr("Angle") << QString() << tr("SHGC"));

	SVStyle::formatDatabaseTableView(m_ui->tableWidgetSHGC);

	m_ui->tableWidgetSHGC->setSortingEnabled(false);

	m_ui->tableWidgetSHGC->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
	QFontMetrics fm(m_ui->tableWidgetSHGC->horizontalHeader()->font());
	int width = fm.boundingRect(tr("Angle")).width();
	m_ui->tableWidgetSHGC->setColumnWidth(0, width);
	m_ui->tableWidgetSHGC->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);

	unsigned int i=9;
	while (true){
		m_ui->tableWidgetSHGC->setItem((int)i, 0, new QTableWidgetItem(QString::number((i+1)*10)));
		m_ui->tableWidgetSHGC->setItem((int)i, 1, new QTableWidgetItem(""));

		if(i)
			break;
		--i;
	}

	m_ui->comboBoxType->blockSignals(true);
	for (int i=0; i<VICUS::WindowGlazingSystem::NUM_MT; ++i)
		m_ui->comboBoxType->addItem(VICUS::KeywordListQt::Keyword("WindowGlazingSystem::modelType_t", i), i);
	m_ui->comboBoxType->blockSignals(false);

	// initial state is "nothing selected"
	updateInput(-1);
}


SVDBWindowGlazingSystemEditWidget::~SVDBWindowGlazingSystemEditWidget() {
	delete m_ui;
}


void SVDBWindowGlazingSystemEditWidget::setup(SVDatabase * db, SVAbstractDatabaseTableModel * dbModel) {
	m_db = db;
	m_dbModel = dynamic_cast<SVDBWindowGlazingSystemTableModel*>(dbModel);
}


void SVDBWindowGlazingSystemEditWidget::updateInput(int id) {
	m_current = nullptr; // disable edit triggers


	if (id == -1) {
		// disable all controls
		setEnabled(false);

		// clear input controls
		m_ui->lineEditName->setString(IBK::MultiLanguageString());

		// property info fields
		m_ui->lineEditUValue->setText("");
		m_ui->lineEditSHGC->setText("");
		m_ui->comboBoxType->blockSignals(true);
		m_ui->comboBoxType->setCurrentIndex(VICUS::WindowGlazingSystem::NUM_MT);
		m_ui->comboBoxType->blockSignals(false);

		m_ui->pushButtonWindowColor->setColor(Qt::black);

		return;
	}
	// re-enable all controls
	setEnabled(true);
	m_current = const_cast<VICUS::WindowGlazingSystem *>(m_db->m_windowGlazingSystems[(unsigned int) id ]);

	// we must have a valid internal load model pointer
	Q_ASSERT(m_current != nullptr);

	// now update the GUI controls

	// for built-ins, disable editing/make read-only
	bool isEditable = !m_current->m_builtIn;

	m_ui->lineEditUValue->setValue(m_current->m_para[VICUS::WindowGlazingSystem::P_ThermalTransmittance].get_value());

	m_ui->comboBoxType->blockSignals(true);
	if(m_current->m_modelType != VICUS::WindowGlazingSystem::NUM_MT){
		m_current->m_modelType = VICUS::WindowGlazingSystem::MT_Simple;
		modelModify();
		m_dbModel->setItemModified(m_current->m_id);
	}
	m_ui->comboBoxType->setCurrentIndex(m_current->m_modelType);
	m_ui->comboBoxType->blockSignals(false);

	if(m_current->m_modelType == VICUS::WindowGlazingSystem::MT_Simple){
		const IBK::LinearSpline &spline=m_current->m_splinePara[VICUS::WindowGlazingSystem::SP_SHGC].m_values;

		if(!spline.empty()){
			for(unsigned int i=0; i<10; ++i){
				double val = spline.value(i *
										  (m_current->m_splinePara[VICUS::WindowGlazingSystem::SP_SHGC].m_xUnit == IBK::Unit("Deg") ?
											  1 : IBK::DEG2RAD));
				m_ui->tableWidgetSHGC->item(9-i,1)->setText(QString::number(val));
			}
		}
	}
	else if(m_current->m_modelType == VICUS::WindowGlazingSystem::MT_Detailed){
		///TODO Stephan implement detailed model
	}
	m_ui->pushButtonWindowColor->blockSignals(true);
	m_ui->pushButtonWindowColor->setColor(m_current->m_color);
	m_ui->pushButtonWindowColor->blockSignals(false);


	m_ui->lineEditName->setReadOnly(!isEditable);
	m_ui->pushButtonWindowColor->setReadOnly(!isEditable);
	m_ui->lineEditSHGC->setReadOnly(!isEditable);
	m_ui->lineEditUValue->setReadOnly(!isEditable);
	m_ui->comboBoxType->setEnabled(isEditable);
	m_ui->toolButtonCreateSpline->setEnabled(false);	///TODO Dirk implement a function for SHGC

}

void SVDBWindowGlazingSystemEditWidget::on_lineEditName_editingFinished(){
	Q_ASSERT(m_current != nullptr);
	if (m_current->m_displayName != m_ui->lineEditName->string()) {  // currentdisplayname is multilanguage string
		m_current->m_displayName = m_ui->lineEditName->string();
		modelModify();
		m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
	}
}

void SVDBWindowGlazingSystemEditWidget::modelModify() {
	m_db->m_windowGlazingSystems.m_modified = true;
}

void SVDBWindowGlazingSystemEditWidget::on_pushButtonWindowColor_colorChanged() {

	Q_ASSERT(m_current != nullptr);

	if (m_current->m_color != m_ui->pushButtonWindowColor->color()) {
		m_current->m_color = m_ui->pushButtonWindowColor->color();
		modelModify(); // tell model that we changed the data
		m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
	}

}


void SVDBWindowGlazingSystemEditWidget::on_lineEditSHGC_editingFinished(){
	Q_ASSERT(m_current != nullptr);
	//do nothing
	// only for button create SHGC ....
}

void SVDBWindowGlazingSystemEditWidget::on_lineEditUValue_editingFinished(){
	Q_ASSERT(m_current != nullptr);
	if(m_ui->lineEditUValue->isValid()){
		VICUS::KeywordList::setParameter(m_current->m_para, "WindowGlazingSystem::para_t", VICUS::WindowGlazingSystem::P_ThermalTransmittance, m_ui->lineEditUValue->value());
		modelModify(); // tell model that we changed the data
		updateInput((int)m_current->m_id);
	}
}

void SVDBWindowGlazingSystemEditWidget::on_comboBoxType_currentIndexChanged(int index) {
	Q_ASSERT(m_current != nullptr);

	// update database but only if different from original
	if (index != (int)m_current->m_modelType)
	{
		m_current->m_modelType = static_cast<VICUS::WindowGlazingSystem::modelType_t>(index);
		modelModify(); // tell model that we changed the data
		updateInput((int)m_current->m_id);
	}
}
