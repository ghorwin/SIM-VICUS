/*	QtExt - Qt-based utility classes and functions (extends Qt library)

	Copyright (c) 2014-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Heiko Fechner    <heiko.fechner -[at]- tu-dresden.de>
	  Andreas Nicolai

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.

	Dieses Programm ist Freie Software: Sie können es unter den Bedingungen
	der GNU General Public License, wie von der Free Software Foundation,
	Version 3 der Lizenz oder (nach Ihrer Wahl) jeder neueren
	veröffentlichten Version, weiter verteilen und/oder modifizieren.

	Dieses Programm wird in der Hoffnung bereitgestellt, dass es nützlich sein wird, jedoch
	OHNE JEDE GEWÄHR,; sogar ohne die implizite
	Gewähr der MARKTFÄHIGKEIT oder EIGNUNG FÜR EINEN BESTIMMTEN ZWECK.
	Siehe die GNU General Public License für weitere Einzelheiten.

	Sie sollten eine Kopie der GNU General Public License zusammen mit diesem
	Programm erhalten haben. Wenn nicht, siehe <https://www.gnu.org/licenses/>.
*/

#include "QtExt_LanguageStringEditWidget1.h"
#include "ui_QtExt_LanguageStringEditWidget1.h"

#include <QLayout>
#include <QDialogButtonBox>
#include <QPushButton>

#include "QtExt_LanguageStringEditWidget3.h"

namespace QtExt {

LanguageStringEditWidget1::LanguageStringEditWidget1(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::LanguageStringEditWidget1),
	m_currentLang("en"),
	m_3rdLang("it"),
	m_showLanguageSelection(false),
	m_readOnly(false)
{
	ui->setupUi(this);

	connect(ui->lineEdit, SIGNAL(editingFinished()), this, SIGNAL(editingFinished()));
}

LanguageStringEditWidget1::~LanguageStringEditWidget1() {
	delete ui;
}

void LanguageStringEditWidget1::initLanguages(const std::string& currentLanguage, const std::string& thirdLanguage,
											  bool showLanguageSelection) {
	m_currentLang = currentLanguage;
	m_3rdLang = thirdLanguage;
	m_showLanguageSelection = showLanguageSelection;
}

void LanguageStringEditWidget1::setString(const IBK::MultiLanguageString& str) {
	m_string = str;
	ui->lineEdit->setText(QString::fromStdString(m_string.string(m_currentLang, "en")));
}

void LanguageStringEditWidget1::set3rdLanguage(const std::string& lang) {
	m_3rdLang = lang;
}

const IBK::MultiLanguageString& LanguageStringEditWidget1::string() const {
	return m_string;
}

void LanguageStringEditWidget1::setReadOnly(bool readOnly) {
	m_readOnly = readOnly;
	ui->lineEdit->setReadOnly(readOnly);
}

void LanguageStringEditWidget1::on_lineEdit_textChanged(const QString &arg1) {
	m_string.setString(arg1.toStdString(), m_currentLang);
	emit textChanged(m_string);
}

void LanguageStringEditWidget1::on_toolButton_clicked() {
	LanguageStringEditDialog3 dlg(m_currentLang, m_showLanguageSelection);
	dlg.setWindowTitle(m_dialog3Caption);
	dlg.set3rdLanguage(m_3rdLang);
	dlg.set(m_string);
	dlg.setReadOnly(m_readOnly);
	if( dlg.exec() == QDialog::Accepted) {
		setString(dlg.string());
		emit editingFinished();
	}
}


// helper dialog

LanguageStringEditDialog3::LanguageStringEditDialog3(const std::string& currentLanguage, bool showLanguageSelection, QWidget *parent) :
	QDialog(parent)
{
	QVBoxLayout* layout = new QVBoxLayout(this);
	QDialogButtonBox* buttonBox = new QDialogButtonBox(this);
	QPushButton* accept = buttonBox->addButton(QDialogButtonBox::Ok);
	QPushButton* cancel = buttonBox->addButton(QDialogButtonBox::Cancel);
	m_widget = new LanguageStringEditWidget3(currentLanguage, showLanguageSelection, this);
	layout->addWidget(m_widget);
	layout->addWidget(buttonBox);
	/// TODO : resize dialog to some meaningful initial size
	resize(1000,300);

	connect(accept, SIGNAL(clicked()), this, SLOT(accept()));
	connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
}

void LanguageStringEditDialog3::set(const IBK::MultiLanguageString& str) {
	m_widget->set(str);
}

void LanguageStringEditDialog3::set3rdLanguage(const std::string& lang) {
	m_widget->set3rdLanguage(lang);
}

const IBK::MultiLanguageString& LanguageStringEditDialog3::string() const {
	return m_widget->string();
}

void LanguageStringEditDialog3::setReadOnly(bool readOnly) {
	m_widget->setReadOnly(readOnly);
}

} // namespace QtExt
