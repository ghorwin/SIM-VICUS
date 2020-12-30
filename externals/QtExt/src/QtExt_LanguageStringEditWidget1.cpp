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
	m_showLanguageSelection(false)
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
	ui->lineEdit->setReadOnly(readOnly);
	ui->toolButton->setEnabled(!readOnly);
}

void LanguageStringEditWidget1::on_lineEdit_textChanged(const QString &arg1) {
	m_string.setString(arg1.toUtf8().data(), m_currentLang);
	emit textChanged(m_string);
}

void LanguageStringEditWidget1::on_toolButton_clicked() {
	LanguageStringEditDialog3 dlg(m_currentLang, m_showLanguageSelection);
	dlg.setWindowTitle(m_dialog3Caption);
	dlg.set3rdLanguage(m_3rdLang);
	dlg.set(m_string);
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

} // namespace QtExt
