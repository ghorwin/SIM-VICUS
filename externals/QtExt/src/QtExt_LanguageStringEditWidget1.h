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

#ifndef QtExt_LanguageStringEditWidget1H
#define QtExt_LanguageStringEditWidget1H

#include <QWidget>
#include <QDialog>

#include <IBK_MultiLanguageString.h>

namespace QtExt {

class LanguageStringEditWidget3;

namespace Ui {
class LanguageStringEditWidget1;
}

/*! A widget to edit multi-language strings of type IBK::MultiLanguageString. */
class LanguageStringEditWidget1 : public QWidget {
	Q_OBJECT

public:
	explicit LanguageStringEditWidget1(QWidget *parent = nullptr);
	~LanguageStringEditWidget1();

	void initLanguages(const std::string& currentLanguage, const std::string& thirdLanguage, bool showLanguageSelection);
	void setString(const IBK::MultiLanguageString& str);
	void setDialog3Caption(const QString & caption) { m_dialog3Caption = caption; }

	void set3rdLanguage(const std::string& lang);

	const IBK::MultiLanguageString& string() const;

	/*! Sets the line-edit into read-only mode and disables the push-button. */
	void setReadOnly(bool readOnly);

public slots:
	void setText(const QString & text) {
		m_string.setString(text.toStdString(), m_currentLang);
	}

signals:
	void editingFinished();
	void textChanged(const IBK::MultiLanguageString&);

private slots:
	void on_lineEdit_textChanged(const QString &arg1);

	/*! The ... button that opens the 3-languages input dialog. */
	void on_toolButton_clicked();

private:
	Ui::LanguageStringEditWidget1 * ui;
	IBK::MultiLanguageString		m_string;
	/*! Caption to be used on 3-language input dialog. */
	QString							m_dialog3Caption;
	std::string						m_currentLang;
	std::string						m_3rdLang;
	bool							m_showLanguageSelection;
	bool							m_readOnly;
};


/*! This is a dialog as wrapper around LanguageStringEditWidget3.
	\todo move to separate file
*/
class LanguageStringEditDialog3 : public QDialog {
	Q_OBJECT
public:
	explicit LanguageStringEditDialog3(const std::string& currentLanguage, bool showLanguageSelection, QWidget *parent = 0);

	void set(const IBK::MultiLanguageString& str);
	void set3rdLanguage(const std::string& lang);

	const IBK::MultiLanguageString& string() const;

	/*! Sets the line-edit into read-only mode and disables the push-button. */
	void setReadOnly(bool readOnly);


private:
	LanguageStringEditWidget3* m_widget;
};

} // namespace QtExt
#endif // QtExt_LanguageStringEditWidget1H
