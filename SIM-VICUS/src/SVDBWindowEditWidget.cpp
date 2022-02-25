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

#include "SVDBWindowEditWidget.h"
#include "ui_SVDBWindowEditWidget.h"

#include <QSortFilterProxyModel>

#include <VICUS_KeywordListQt.h>

#include <QtExt_LanguageHandler.h>
#include <SV_Conversions.h>

#include "SVSettings.h"
#include "SVDBWindowTableModel.h"
#include "SVDatabaseEditDialog.h"
#include "SVMainWindow.h"
#include "SVConstants.h"

SVDBWindowEditWidget::SVDBWindowEditWidget(QWidget *parent) :
	SVAbstractDatabaseEditWidget(parent),
	m_ui(new Ui::SVDBWindowEditWidget)
{
	m_ui->setupUi(this);
	m_ui->masterLayout->setMargin(4);

	// style the table widget

	m_ui->lineEditName->initLanguages(QtExt::LanguageHandler::instance().langId().toStdString(), THIRD_LANGUAGE, true);
	m_ui->lineEditName->setDialog3Caption(tr("Window"));

	m_ui->pushButtonColor->setDontUseNativeDialog(SVSettings::instance().m_dontUseNativeDialogs);

	// Frame + Divider
	m_ui->comboBoxFrameMethod->blockSignals(true);
	m_ui->comboBoxDividerMethod->blockSignals(true);
	for (int i=0; i<VICUS::Window::NUM_M; ++i) {
		m_ui->comboBoxFrameMethod->addItem(VICUS::KeywordListQt::Description("Window::Method", i), i);
		m_ui->comboBoxDividerMethod->addItem(VICUS::KeywordListQt::Description("Window::Method", i), i);
	}
	m_ui->comboBoxFrameMethod->blockSignals(false);
	m_ui->comboBoxDividerMethod->blockSignals(false);

	// initial state is "nothing selected"
	updateInput(-1);
}


SVDBWindowEditWidget::~SVDBWindowEditWidget() {
	delete m_ui;
}


void SVDBWindowEditWidget::setup(SVDatabase * db, SVAbstractDatabaseTableModel * dbModel) {
	m_db = db;
	m_dbModel = dynamic_cast<SVDBWindowTableModel*>(dbModel);
}


void SVDBWindowEditWidget::updateInput(int id) {
	m_current = nullptr; // disable edit triggers

	m_ui->lineEditGlazingSystemName->setText("");

	m_ui->lineEditFrameMaterialName->setText("");
	m_ui->lineEditFrameMaterialThickness->setText("---");

	m_ui->lineEditDividerMaterialName->setText("");
	m_ui->lineEditDividerMaterialThickness->setText("---");

	if (id == -1) {
		// disable all controls
		setEnabled(false);

		// clear input controls
		m_ui->lineEditName->setString(IBK::MultiLanguageString());

		// property info fields
		m_ui->lineEditUValue->setText("");
		m_ui->lineEditSHGC->setText("");

		m_ui->pushButtonColor->setColor(Qt::black);

		return;
	}
	// re-enable all controls
	setEnabled(true);
	m_current = const_cast<VICUS::Window *>(m_db->m_windows[(unsigned int) id ]);
	// we must have a valid internal load model pointer
	Q_ASSERT(m_current != nullptr);

	m_ui->lineEditName->setString(m_current->m_displayName);
	m_ui->pushButtonColor->setColor(m_current->m_color);

	// now update the GUI controls

	m_ui->lineEditUValue->setText("---");
	m_ui->lineEditSHGC->setText("---");

	// for built-ins, disable editing/make read-only
	bool isEditable = !m_current->m_builtIn;

	// *** glazing system ***

	if (m_current->m_idGlazingSystem != VICUS::INVALID_ID) {
		VICUS::WindowGlazingSystem *glazSys = const_cast<VICUS::WindowGlazingSystem *>(m_db->m_windowGlazingSystems[m_current->m_idGlazingSystem]);
		if (glazSys != nullptr){
			m_ui->lineEditUValue->setText(QString("%L1").arg(glazSys->uValue(), 0, 'f', 4));
			m_ui->lineEditSHGC->setText(QString("%L1").arg(glazSys->SHGC(), 0, 'f', 4));
			m_ui->lineEditGlazingSystemName->setText(QtExt::MultiLangString2QString(glazSys->m_displayName));
		}
	}
	m_ui->lineEditFrameInput->setText("---");
	// *** frame ***
	int frameIdx = 0;
	switch(m_current->m_methodFrame) {
		case VICUS::Window::M_Fraction:{
			m_ui->labelFrameInput->setText(tr("Fraction of Farme:"));
			m_ui->labelFrameInputUnit->setText(tr("-"));
			m_ui->lineEditFrameInput->setup(0,0.99,tr("Frame fraction of the window"), false, true);
			double inputVal;
			try {
				inputVal = m_current->m_para[VICUS::Window::P_FrameFraction].get_value();
				if(inputVal <= 0 ||
						inputVal > 0.99){
					inputVal = 0.3;
					VICUS::KeywordList::setParameter(m_current->m_para,"Window::para_t", VICUS::Window::P_FrameFraction, inputVal);
					modelModify();
				}
			}  catch (...) {
				inputVal = 0.3;
				VICUS::KeywordList::setParameter(m_current->m_para,"Window::para_t", VICUS::Window::P_FrameFraction, inputVal);
				modelModify();
			}
			m_ui->lineEditFrameInput->setValue(inputVal);
			frameIdx = 1;
		}
		break;
		case VICUS::Window::M_ConstantWidth:{
			m_ui->labelFrameInput->setText(tr("Width:"));
			m_ui->labelFrameInputUnit->setText(tr("m"));
			m_ui->lineEditFrameInput->setup(0,2,tr("Frame width"), false, true);
			double inputVal;
			try {
				inputVal = m_current->m_para[VICUS::Window::P_FrameWidth].get_value();
				if(inputVal <= 0){
					inputVal = 0.06;
					VICUS::KeywordList::setParameter(m_current->m_para,"Window::para_t", VICUS::Window::P_FrameWidth, inputVal);
					modelModify();
				}
			}  catch (...) {
				inputVal = 0.06;
				VICUS::KeywordList::setParameter(m_current->m_para,"Window::para_t", VICUS::Window::P_FrameWidth, inputVal);
				modelModify();
			}
			m_ui->lineEditFrameInput->setValue(inputVal);
			frameIdx = 2;

		}
		break;
		case VICUS::Window::M_None:
			frameIdx = 0;
		break;
		case VICUS::Window::NUM_M: {
			m_current->m_methodFrame = VICUS::Window::M_None;
			frameIdx = 0;
		}
		break;

	}

	if (m_current->m_frame.m_id != VICUS::INVALID_ID && frameIdx > 0) {
		m_ui->lineEditFrameMaterialThickness->setup(0,2,tr("Material thichness"), false, true);
		m_ui->lineEditFrameMaterialThickness->setValue(m_current->m_frame.m_para[VICUS::WindowFrame::P_Thickness].get_value());
		VICUS::Material *mat = const_cast<VICUS::Material*>(m_db->m_materials[m_current->m_frame.m_idMaterial]);
		if (mat != nullptr)
			m_ui->lineEditFrameMaterialName->setText(QtExt::MultiLangString2QString(mat->m_displayName));
	}

	m_ui->comboBoxFrameMethod->blockSignals(true);
	m_ui->comboBoxFrameMethod->setCurrentIndex(frameIdx);
	m_ui->comboBoxFrameMethod->blockSignals(false);
	m_ui->comboBoxFrameMethod->setEnabled(isEditable);

	m_ui->lineEditFrameMaterialName->setEnabled(frameIdx>0);
	m_ui->lineEditFrameInput->setReadOnly(!isEditable && frameIdx>0);
	m_ui->lineEditFrameMaterialThickness->setReadOnly(!isEditable && frameIdx>0);
	m_ui->lineEditFrameMaterialThickness->setEnabled(frameIdx>0);
	m_ui->lineEditFrameInput->setEnabled(frameIdx>0);
	m_ui->toolButtonSelectFrameMaterial->setEnabled(isEditable && frameIdx > 0);

	// *** divider ***
	int dividerIdx = 0;
	switch(m_current->m_methodDivider) {
		case VICUS::Window::M_Fraction:{
			m_ui->labelDividerInput->setText(tr("Fraction of Divider:"));
			m_ui->labelDividerInputUnit->setText(tr("-"));
			double maxVal = 1;
			if(m_current->m_methodFrame == VICUS::Window::M_Fraction){
				maxVal = 0.99-m_ui->lineEditFrameInput->value();
			}
			m_ui->lineEditDividerInput->setup(0,0.99,tr("Divider fraction of the window"), false, true);
			double inputVal;
			try {
				inputVal = m_current->m_para[VICUS::Window::P_DividerFraction].get_value();
				if(inputVal <= 0 ||
						inputVal > maxVal){
					inputVal = std::min(0.05, maxVal);
					VICUS::KeywordList::setParameter(m_current->m_para,"Window::para_t", VICUS::Window::P_DividerFraction, inputVal);
					modelModify();
				}
			}  catch (...) {
				inputVal = std::min(0.05, maxVal);
				VICUS::KeywordList::setParameter(m_current->m_para,"Window::para_t", VICUS::Window::P_DividerFraction, inputVal);
				modelModify();
			}
			m_ui->lineEditDividerInput->setValue(inputVal);
			dividerIdx = 1;
		}
		break;
		case VICUS::Window::M_ConstantWidth:{
			m_ui->labelDividerInput->setText(tr("Width:"));
			m_ui->labelDividerInputUnit->setText(tr("m"));
			m_ui->lineEditDividerInput->setup(0,2,tr("Divider width"), false, true);
			double inputVal;
			try {
				inputVal = m_current->m_para[VICUS::Window::P_DividerWidth].get_value();
				if(inputVal <= 0){
					inputVal = 0.01;
					VICUS::KeywordList::setParameter(m_current->m_para,"Window::para_t", VICUS::Window::P_DividerWidth, inputVal);
					modelModify();
				}
			}  catch (...) {
				inputVal = 0.01;
				VICUS::KeywordList::setParameter(m_current->m_para,"Window::para_t", VICUS::Window::P_DividerWidth, inputVal);
				modelModify();
			}
			m_ui->lineEditDividerInput->setValue(inputVal);
			dividerIdx = 2;
		}
		break;
		case VICUS::Window::M_None:
			dividerIdx = 0;
		break;
		case VICUS::Window::NUM_M: {
			m_current->m_methodDivider = VICUS::Window::M_None;
			dividerIdx = 0;
		}
		break;

	}

	if (m_current->m_divider.m_id != VICUS::INVALID_ID && dividerIdx > 0) {
		m_ui->lineEditDividerMaterialThickness->setValue(m_current->m_frame.m_para[VICUS::WindowDivider::P_Thickness].get_value());
		VICUS::Material *mat = const_cast<VICUS::Material*>(m_db->m_materials[m_current->m_divider.m_idMaterial]);
		if (mat != nullptr)
			m_ui->lineEditDividerMaterialName->setText(QtExt::MultiLangString2QString(mat->m_displayName));
	}

	m_ui->comboBoxDividerMethod->blockSignals(true);
	m_ui->comboBoxDividerMethod->setCurrentIndex(dividerIdx);
	m_ui->comboBoxDividerMethod->blockSignals(false);

	m_ui->comboBoxDividerMethod->setEnabled(isEditable);
	m_ui->lineEditDividerInput->setReadOnly(!isEditable && dividerIdx>0);
	m_ui->lineEditDividerMaterialThickness->setReadOnly(!isEditable  && dividerIdx>0);
	m_ui->toolButtonSelectDividerMaterial->setEnabled(isEditable && dividerIdx > 0);
	m_ui->lineEditDividerInput->setEnabled(dividerIdx>0);
	m_ui->lineEditDividerMaterialThickness->setEnabled(dividerIdx>0);
	m_ui->lineEditDividerMaterialName->setEnabled(dividerIdx>0);

	m_ui->pushButtonColor->blockSignals(true);
	m_ui->pushButtonColor->setColor(m_current->m_color);
	m_ui->pushButtonColor->blockSignals(false);


	m_ui->lineEditName->setReadOnly(!isEditable);
	m_ui->pushButtonColor->setReadOnly(!isEditable);
	m_ui->toolButtonSelectGlazingSystemName->setEnabled(isEditable);


}

void SVDBWindowEditWidget::on_lineEditName_editingFinished(){
	Q_ASSERT(m_current != nullptr);
	if (m_current->m_displayName != m_ui->lineEditName->string()) {  // currentdisplayname is multilanguage string
		m_current->m_displayName = m_ui->lineEditName->string();
		modelModify();
	}
}


void SVDBWindowEditWidget::modelModify() {
	m_db->m_windows.m_modified = true;
	m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
}


void SVDBWindowEditWidget::on_toolButtonSelectGlazingSystemName_clicked() {
	// get glazing system edit dialog from mainwindow
	SVDatabaseEditDialog * glazSysEditDialog = SVMainWindow::instance().dbWindowGlazingSystemEditDialog();
	unsigned int conId = glazSysEditDialog->select(m_current->m_idGlazingSystem);
	if (conId !=  VICUS::INVALID_ID && conId != m_current->m_idGlazingSystem) {
		m_current->m_idGlazingSystem = conId;
		modelModify(); // tell model that we changed the data

	}
	updateInput((int)m_current->m_id);
}


void SVDBWindowEditWidget::on_pushButtonColor_colorChanged() {
	Q_ASSERT(m_current != nullptr);

	if (m_current->m_color != m_ui->pushButtonColor->color()) {
		m_current->m_color = m_ui->pushButtonColor->color();
		modelModify(); // tell model that we changed the data
	}
}


void SVDBWindowEditWidget::on_toolButtonSelectFrameMaterial_clicked(){
	// get material edit dialog from mainwindow
	SVDatabaseEditDialog * matEditDialog = SVMainWindow::instance().dbMaterialEditDialog();
	unsigned int matId = matEditDialog->select(m_current->m_frame.m_idMaterial);
	if (matId !=  VICUS::INVALID_ID && matId != m_current->m_frame.m_idMaterial) {
		m_current->m_frame.m_idMaterial = matId;
		modelModify(); // tell model that we changed the data,

	}
	updateInput((int)m_current->m_id);
}


void SVDBWindowEditWidget::on_toolButtonSelectDividerMaterial_clicked(){
	// get material edit dialog from mainwindow
	SVDatabaseEditDialog * matEditDialog = SVMainWindow::instance().dbMaterialEditDialog();
	unsigned int matId = matEditDialog->select(m_current->m_divider.m_idMaterial);
	if (matId !=  VICUS::INVALID_ID && matId != m_current->m_divider.m_idMaterial) {
		m_current->m_divider.m_idMaterial = matId;
		modelModify(); // tell model that we changed the data
	}
	updateInput((int)m_current->m_id);
}


void SVDBWindowEditWidget::on_lineEditFrameMaterialThickness_editingFinishedSuccessfully(){
	Q_ASSERT(m_current != nullptr);
		VICUS::KeywordList::setParameter(m_current->m_frame.m_para, "WindowFrame::para_t", VICUS::WindowFrame::P_Thickness, m_ui->lineEditFrameMaterialThickness->value());
		modelModify(); // tell model that we changed the data
		updateInput((int)m_current->m_id);
}


void SVDBWindowEditWidget::on_lineEditDividerMaterialThickness_editingFinishedSuccessfully(){
	Q_ASSERT(m_current != nullptr);
		VICUS::KeywordList::setParameter(m_current->m_divider.m_para, "WindowDivider::para_t", VICUS::WindowDivider::P_Thickness, m_ui->lineEditDividerMaterialThickness->value());
		modelModify(); // tell model that we changed the data
		updateInput((int)m_current->m_id);
}


void SVDBWindowEditWidget::on_lineEditDividerInput_editingFinishedSuccessfully() {
	Q_ASSERT(m_current != nullptr);
	bool isModi = true;
	if (m_current->m_methodDivider == VICUS::Window::M_ConstantWidth)
		VICUS::KeywordList::setParameter(m_current->m_para,"Window::para_t", VICUS::Window::P_DividerWidth, m_ui->lineEditDividerInput->value());
	else if(m_current->m_methodDivider == VICUS::Window::M_Fraction){
		double val = m_ui->lineEditDividerInput->value();
		VICUS::KeywordList::setParameter(m_current->m_para,"Window::para_t", VICUS::Window::P_DividerFraction, val);
		if(m_current->m_methodFrame == VICUS::Window::M_Fraction &&
				1 - val - m_current->m_para[VICUS::Window::P_FrameFraction].get_value() < 0.01)
			VICUS::KeywordList::setParameter(m_current->m_para,"Window::para_t", VICUS::Window::P_FrameFraction, 0.99 - val);
	}
	else
		isModi = false;
	if (isModi) {
		modelModify(); // tell model that we changed the data
		updateInput((int)m_current->m_id);
	}
}


void SVDBWindowEditWidget::on_lineEditFrameInput_editingFinishedSuccessfully() {
	Q_ASSERT(m_current != nullptr);
	bool isModi = true;
	if (m_current->m_methodFrame == VICUS::Window::M_ConstantWidth)
		VICUS::KeywordList::setParameter(m_current->m_para,"Window::para_t", VICUS::Window::P_FrameWidth, m_ui->lineEditFrameInput->value());
	else if(m_current->m_methodFrame == VICUS::Window::M_Fraction){
		double val = m_ui->lineEditFrameInput->value();
		VICUS::KeywordList::setParameter(m_current->m_para,"Window::para_t", VICUS::Window::P_FrameFraction, val);
		if(m_current->m_methodDivider == VICUS::Window::M_Fraction &&
				1 - val - m_current->m_para[VICUS::Window::P_DividerFraction].get_value() < 0.01)
			VICUS::KeywordList::setParameter(m_current->m_para,"Window::para_t", VICUS::Window::P_DividerFraction, 0.99 - val);

	}
	else
		isModi = false;
	if (isModi) {
		modelModify(); // tell model that we changed the data
		updateInput((int)m_current->m_id);
	}
}


void SVDBWindowEditWidget::on_comboBoxFrameMethod_currentIndexChanged(int index) {
	Q_ASSERT(m_current != nullptr);

	// update database but only if different from original
	if (index != (int)m_current->m_methodFrame) {
		m_current->m_frame.m_id = 1;
		m_current->m_methodFrame = static_cast<VICUS::Window::Method>(index);
		if (m_current->m_methodFrame == VICUS::Window::M_Fraction)
			m_ui->lineEditFrameInput->setValue(0.3);
		else if(m_current->m_methodFrame == VICUS::Window::M_ConstantWidth)
			m_ui->lineEditFrameInput->setValue(0.08);
		else
			m_current->m_frame.m_id = VICUS::INVALID_ID;
		modelModify(); // tell model that we changed the data
		updateInput((int)m_current->m_id);
	}
}


void SVDBWindowEditWidget::on_comboBoxDividerMethod_currentIndexChanged(int index) {
	Q_ASSERT(m_current != nullptr);

	// update database but only if different from original
	if (index != (int)m_current->m_methodDivider) {
		m_current->m_methodDivider = static_cast<VICUS::Window::Method>(index);
		if (m_current->m_methodDivider== VICUS::Window::M_Fraction || m_current->m_methodDivider== VICUS::Window::M_ConstantWidth)
			m_current->m_divider.m_id = 1;
		else
			m_current->m_divider.m_id = VICUS::INVALID_ID;
		modelModify(); // tell model that we changed the data
		updateInput((int)m_current->m_id);
	}
}
