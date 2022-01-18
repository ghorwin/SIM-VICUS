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

#ifndef QtExt_BrowseFilenameWidgetH
#define QtExt_BrowseFilenameWidgetH

#include <QWidget>

class QLineEdit;
class QToolButton;

namespace QtExt {

/*! A widget that provides a line edit for entering a file name/path manually and a browse button that opens
	a file selection dialog to manually select the file.
	\code
	// to select existing files only
	filenameWidget->setup("myfile.txt", true, true, tr("Text files (*.txt);;All files (*.*)"));

	// to existing directories
	filenameWidget->setup("myfile.txt", false, true, QString());

	// to select directories that may not yet exist
	filenameWidget->setup("myfile.txt", false, false, QString());
	\endcode
*/
class BrowseFilenameWidget : public QWidget {
	Q_OBJECT
public:
	explicit BrowseFilenameWidget(QWidget *parent = nullptr);

	/*! Sets up line edit. */
	void setup(const QString & filename, bool filenameMode, bool fileMustExist, const QString & filter, bool dontUseNativeFilenameDialog);

	/*! Sets a filename in the line edit. */
	void setFilename(const QString & filename);

	/*! Returns the filename currently held in the line edit. */
	QString filename() const;

	/*! The line edit (to set tab order). */
	QLineEdit		*lineEdit() { return m_lineEdit; }
	/*! The tool button (to set tab order). */
	QToolButton		*toolBtn() { return m_toolBtn; }

	/*! When set to read-only, button is disabled and line edit is made read-only. */
	void setReadOnly(bool readOnly);

signals:
	/*! Emitted when filename in line edit has changed. */
	void editingFinished();

	/*! Emitted, when return was pressed in line edit (to complete the editing of the filename). */
	void returnPressed();

private slots:
	void onToolBtnClicked();

private:
	/*! If true, tool button requests a file, otherwise a directory. */
	bool			m_filenameMode;
	/*! If true, the file must exist when browsing for the file/directory. */
	bool			m_fileMustExist;
	/*! Filter to use in file dialog (only applicable for filename-mode). */
	QString			m_filter;
	/*! If true, the Qt-own file dialog is used. */
	bool			m_dontUseNativeFilenameDialog;

	/*! The line edit. */
	QLineEdit		*m_lineEdit;
	/*! The tool button. */
	QToolButton		*m_toolBtn;
};

} // namespace QtExt

#endif // QtExt_BrowseFilenameWidgetH
