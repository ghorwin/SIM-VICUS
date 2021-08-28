/*	QtExt - Qt-based utility classes and functions (extends Qt library)

	Copyright (c) 2014-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Heiko Fechner
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 3 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

*/

#include "QtExt_BrowseFilenameWidget.h"

#include <QLineEdit>
#include <QToolButton>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QFileInfo>

#include <IBK_Path.h>

#include "QtExt_configuration.h"
#include "QtExt_Settings.h"

namespace QtExt {

BrowseFilenameWidget::BrowseFilenameWidget(QWidget *parent) :
	QWidget(parent),
	m_filenameMode(true),
	m_fileMustExist(true)
{
	m_lineEdit = new QLineEdit(this);
	m_toolBtn = new QToolButton(this);
	m_toolBtn->setText("...");
	QHBoxLayout * lay = new QHBoxLayout;
	lay->addWidget(m_lineEdit);
	lay->addWidget(m_toolBtn);
	setLayout(lay);
	lay->setMargin(0);

	connect(m_toolBtn, SIGNAL(clicked()), this, SLOT(onToolBtnClicked()));
	connect(m_lineEdit, SIGNAL(editingFinished()), this, SIGNAL(editingFinished()));
	connect(m_lineEdit, SIGNAL(returnPressed()), this, SIGNAL(returnPressed()));
}


void BrowseFilenameWidget::setup(const QString &filename, bool filenameMode, bool fileMustExist, const QString & filter, bool dontUseNativeFilenameDialog) {
	m_filenameMode = filenameMode;
	m_fileMustExist = fileMustExist;
	m_filter = filter;
	m_lineEdit->setText(filename);
	m_dontUseNativeFilenameDialog = dontUseNativeFilenameDialog;
}


void BrowseFilenameWidget::setReadOnly(bool readOnly) {
	m_toolBtn->setEnabled(!readOnly);
	m_lineEdit->setReadOnly(readOnly);
}


void BrowseFilenameWidget::setFilename(const QString & filename) {
	// replace potentially existing relative path paths
	// but that does not mean we always want an absolute path!!!
	IBK::Path f(filename.toStdString());
	f.removeRelativeParts();
	m_lineEdit->setText(QString::fromStdString(f.str()));
}


QString BrowseFilenameWidget::filename() const {
	return m_lineEdit->text().trimmed();
}


void BrowseFilenameWidget::onToolBtnClicked() {
	QString fn;
	blockSignals(true);
	if (m_filenameMode) {
		if (m_fileMustExist) {
			fn = QFileDialog::getOpenFileName(nullptr, tr("Select filename"), filename(), m_filter, nullptr,
											  m_dontUseNativeFilenameDialog ? QFileDialog::DontUseNativeDialog : QFileDialog::Options()
					);
		}
		else {
			fn = QFileDialog::getSaveFileName(this, tr("Select filename"), filename(), m_filter, nullptr,
											  m_dontUseNativeFilenameDialog ? QFileDialog::DontUseNativeDialog : QFileDialog::Options()
					);
		}
	}
	else {
		if (m_fileMustExist) {
			fn = QFileDialog::getExistingDirectory(this, tr("Select filename"), filename(),
												   m_dontUseNativeFilenameDialog ? QFileDialog::DontUseNativeDialog : QFileDialog::Options()
					);
		}
		else {
			fn = QFileDialog::getSaveFileName(this, tr("Select directory"), filename(), QString(), nullptr,
											  m_dontUseNativeFilenameDialog ? QFileDialog::DontUseNativeDialog : QFileDialog::Options()
					);
		}
	}
	blockSignals(false);
	if (!fn.isEmpty()) {
		m_lineEdit->setText(fn);
		emit editingFinished();
	}
}

} // namespace QtExt
