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

#include "QtExt_LanguageStringEditWidget3.h"
#include "ui_QtExt_LanguageStringEditWidget3.h"

namespace QtExt {

LanguageStringEditWidget3::LanguageStringEditWidget3(const std::string& currentLanguage, bool showLanguageSelection, QWidget *parent) :
	QWidget(parent),
	ui(new Ui::LanguageStringEditWidget3),
	m_currentLang(currentLanguage),
	m_3rdLang("it")
{
	ui->setupUi(this);

	ui->labelLanguageSelection->setVisible(showLanguageSelection);
	ui->comboBoxThirdLanguage->setVisible(showLanguageSelection);
	if(showLanguageSelection) {
		ui->comboBoxThirdLanguage->addItems(QStringList() << "cz" << "de" << "fr" << "es" << "it" << "pl" << "ru" << "zh");
	}

	ui->labelCurrentLanguageCaption->setText(QString::fromStdString(m_currentLang));
	ui->labelThirdLanguageCaption->setText(QString::fromStdString(m_3rdLang));

	// special handling if current language is en
	if (m_currentLang == "en") {
		ui->lineEditEnglish->setVisible(false);
		ui->labelEnglishLanguageCaption->setVisible(false);
	}

	connect(ui->lineEditCurrent, SIGNAL(editingFinished()), this, SIGNAL(editingFinished()));
	connect(ui->lineEditEnglish, SIGNAL(editingFinished()), this, SIGNAL(editingFinished()));
	connect(ui->lineEdit3rdLanguage, SIGNAL(editingFinished()), this, SIGNAL(editingFinished()));

}

LanguageStringEditWidget3::~LanguageStringEditWidget3() {
	delete ui;
}

void LanguageStringEditWidget3::set(const IBK::MultiLanguageString& str) {
	m_string = str;
	ui->lineEditEnglish->setText(QString::fromUtf8(m_string.string("en").c_str()));
	ui->lineEditCurrent->setText(QString::fromUtf8(m_string.string(m_currentLang).c_str()));
	ui->lineEdit3rdLanguage->setText(QString::fromUtf8(m_string.string(m_3rdLang).c_str()));
}

void LanguageStringEditWidget3::set3rdLanguage(const std::string& lang) {
	if(lang != m_3rdLang) {
		m_3rdLang = lang;
		ui->labelThirdLanguageCaption->setText(QString::fromStdString(m_3rdLang));
		ui->lineEdit3rdLanguage->setText(QString::fromUtf8(m_string.string(m_3rdLang).c_str()));
		int index = ui->comboBoxThirdLanguage->findText(QString::fromStdString(lang));
		ui->comboBoxThirdLanguage->setCurrentIndex(index);
	}
}

const IBK::MultiLanguageString& LanguageStringEditWidget3::string() const {
	return m_string;
}

void LanguageStringEditWidget3::on_lineEditEnglish_textChanged(const QString &arg1) {
	m_string.setString(arg1.toUtf8().data(), "en");
	emit textChanged(m_string);
}

void LanguageStringEditWidget3::on_lineEditCurrent_textChanged(const QString &arg1) {
	m_string.setString(arg1.toUtf8().data(), m_currentLang);
	emit textChanged(m_string);
}

void LanguageStringEditWidget3::on_lineEdit3rdLanguage_textChanged(const QString &arg1) {
	m_string.setString(arg1.toUtf8().data(), m_3rdLang);
	emit textChanged(m_string);
}

void LanguageStringEditWidget3::on_comboBoxThirdLanguage_currentTextChanged(const QString &arg1) {
	set3rdLanguage(arg1.toStdString());
}

} // namespace QtExt
