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

#include "SVLogFileDialog.h"
#include "ui_SVLogFileDialog.h"

#include <QFileInfo>
#include <QPushButton>
#include <QDialogButtonBox>

#include "SVSettings.h"
#include "SVProjectHandler.h"

SVLogFileDialog::SVLogFileDialog(QWidget *parent) :
	QDialog(parent, Qt::Dialog | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint),
	m_ui(new Ui::SVLogFileDialog),
	m_pushButtonOpenFileInTextEditor(nullptr),
	m_pushButtonReloadProject(nullptr)
{
	m_ui->setupUi(this);
	resize(1400,600);

	m_pushButtonOpenLogInTextEditor = m_ui->buttonBox->addButton(tr("Open log in text editor..."), QDialogButtonBox::ActionRole);
	m_pushButtonOpenFileInTextEditor = m_ui->buttonBox->addButton(tr("Open project file in text editor..."), QDialogButtonBox::ActionRole);
	m_pushButtonReloadProject = m_ui->buttonBox->addButton(tr("Reload project"), QDialogButtonBox::ActionRole);

	connect(m_pushButtonOpenLogInTextEditor, &QPushButton::clicked,		this, &SVLogFileDialog::onEditLogClicked);
	connect(m_pushButtonOpenFileInTextEditor, &QPushButton::clicked,	this, &SVLogFileDialog::onOpenFileClicked);
	connect(m_pushButtonReloadProject, &QPushButton::clicked,			this, &SVLogFileDialog::onReloadprojectClicked);
}


SVLogFileDialog::~SVLogFileDialog() {
	delete m_ui;
}


void SVLogFileDialog::setLogFile(const QString & logfilepath, QString projectfilepath, bool editFileButtonVisible) {
	m_logFilePath = logfilepath;
	m_projectFilePath = projectfilepath;
	m_ui->logWidget->showLogFile(m_logFilePath);
	setWindowTitle(QFileInfo(m_logFilePath).fileName());
	if (editFileButtonVisible) {
		m_pushButtonOpenFileInTextEditor->setVisible(true);
		m_pushButtonReloadProject->setVisible(true);
		m_ui->labelOpenFileError->setVisible(true);
		m_ui->labelOpenFileError->setText(tr("Error opening file '%1'.").arg(projectfilepath));
	}
	else {
		m_pushButtonOpenFileInTextEditor->setVisible(false);
		m_pushButtonReloadProject->setVisible(false);
		m_ui->labelOpenFileError->setVisible(false);
	}
}


void SVLogFileDialog::onOpenFileClicked() {
	SVSettings::instance().openFileInTextEditor(this, m_projectFilePath);
}


void SVLogFileDialog::onEditLogClicked() {
	SVSettings::instance().openFileInTextEditor(this, m_logFilePath);
}


void SVLogFileDialog::onReloadprojectClicked() {
	FUNCID(SVLogFileDialog::onReloadprojectClicked);

	SVProjectHandler::instance().setReload();
	IBK::IBK_Message("\n------------------------------------------------------\n",IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::IBK_Message(IBK::FormatString("Reload project '%1'\n").arg(IBK::Path(m_projectFilePath.toStdString())),IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::IBK_Message("------------------------------------------------------\n\n",IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	close();
}
