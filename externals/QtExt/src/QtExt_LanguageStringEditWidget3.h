#ifndef QtExt_LanguageStringEditWidget3H
#define QtExt_LanguageStringEditWidget3H

#include <QWidget>

#include <IBK_MultiLanguageString.h>

namespace QtExt {

namespace Ui {
class LanguageStringEditWidget3;
}

class LanguageStringEditWidget3 : public QWidget
{
	Q_OBJECT

public:
	explicit LanguageStringEditWidget3(const std::string& currentLanguage, bool showLanguageSelection, QWidget *parent = nullptr);
	~LanguageStringEditWidget3();


	void set(const IBK::MultiLanguageString& str);

	void set3rdLanguage(const std::string& lang);

	const IBK::MultiLanguageString& string() const;

signals:
	void editingFinished();
	void textChanged(const IBK::MultiLanguageString&);

private slots:
	void on_lineEditEnglish_textChanged(const QString &arg1);

	void on_lineEditCurrent_textChanged(const QString &arg1);

	void on_lineEdit3rdLanguage_textChanged(const QString &arg1);

	void on_comboBoxThirdLanguage_currentTextChanged(const QString &arg1);

private:
	Ui::LanguageStringEditWidget3 * ui;
	IBK::MultiLanguageString		m_string;
	std::string						m_currentLang;
	std::string						m_3rdLang;
};


} // namespace QtExt
#endif // QtExt_LanguageStringEditWidget3H
