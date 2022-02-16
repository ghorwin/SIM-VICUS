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

#ifndef QtExt_ColorButtonH
#define QtExt_ColorButtonH

#include <QPushButton>
#include <QColor>

namespace QtExt {

/*! A tool button with special color background that can be used to select this color
*/
class ColorButton : public QPushButton {
	Q_OBJECT
public:
	explicit ColorButton(QWidget *parent = nullptr);

	/*! Sets the color and triggers a repaint. */
	void setColor(const QColor & c);

	/*! Returns the currently selected color. */
	const QColor color() const { return m_color; }

	/*! Sets a special "disabled" state - button will be disabled
		but being painted as regular.
	*/
	void setReadOnly(bool readOnly);

	/*! Overloaded to enable change of appearance (gray color). */
	void setEnabled(bool enabled);

	/*! Use this function to enable/disable use of native color dialog. */
	void setDontUseNativeDialog(bool dontUseNativeDialog);

signals:
	/*! Emitted, when color has been changed by user. */
	void colorChanged();

protected:
	virtual void paintEvent( QPaintEvent* ) override;

private slots:
	void onClicked();

private:
	/*! Holds the color to be drawn on the button. */
	QColor	m_color;

	bool	m_readOnly;
	bool	m_dontUseNativeDialog;
};

} // namespace QtExt


#endif // QtExt_ColorButtonH
