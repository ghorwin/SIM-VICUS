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

	/*! Sets the line-edit into read-only mode and disables the push-button. */
	void setReadOnly(bool readonly);

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
