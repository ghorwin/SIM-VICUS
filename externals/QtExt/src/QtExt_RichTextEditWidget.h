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

#ifndef QtExt_RichTextEditWidgetH
#define QtExt_RichTextEditWidgetH

#include <QWidget>
#include <QMap>
#include <QPointer>
#include <QTextEdit>

QT_BEGIN_NAMESPACE
class QAction;
class QComboBox;
class QFontComboBox;
class QTextCharFormat;
class QToolBar;
QT_END_NAMESPACE

namespace QtExt {


class SignalingTextEdit : public QTextEdit {
	Q_OBJECT
public:
	SignalingTextEdit(QWidget *parent) : QTextEdit(parent) {}

protected:
	/*! Emits the editingFinished() signal. */
	virtual void focusOutEvent(QFocusEvent * /*event*/) {
		emit editingFinished();
	}

signals:
	void editingFinished();
};

/*! A truncated version of the rich text edit example from Qt.
	Basically a widget with toolbars that allows rich text editing.
*/
class RichTextEditWidget : public QWidget {
	Q_OBJECT

public:
	RichTextEditWidget(QWidget *parent = nullptr);

	SignalingTextEdit *textEdit;

	/*! Returns the current HTML text in the widget without the DOCTYPE stuff in front of <html>. */
	QString htmlText() const;

signals:
	/*! Emitted, when user finishes editing (widget looses focus). */
	void editingFinished();

//protected:
//	/*! Emits the editingFinished() signal. */
//	virtual void focusOutEvent(QFocusEvent *event);
private slots:
	void textBold();
	void textUnderline();
	void textItalic();
	void textFamily(const QString &f);
	void textSize(const QString &p);
	void textStyle(int styleIndex);
	void textColor();
	void textAlign(QAction *a);

	void currentCharFormatChanged(const QTextCharFormat &format);
	void cursorPositionChanged();
	void clipboardDataChanged();

	void undo();
	void redo();
	void cut();
	void copy();
	void paste();

private:
	void setupEditActions();
	void setupTextActions();

	QToolBar * addToolBar(QString caption);

	void mergeFormatOnWordOrSelection(const QTextCharFormat &format);
	void fontChanged(const QFont &f);
	void colorChanged(const QColor &c);
	void alignmentChanged(Qt::Alignment a);

	QAction *actionTextBold;
	QAction *actionTextUnderline;
	QAction *actionTextItalic;
	QAction *actionTextColor;
	QAction *actionAlignLeft;
	QAction *actionAlignCenter;
	QAction *actionAlignRight;
	QAction *actionAlignJustify;
	QAction *actionUndo;
	QAction *actionRedo;
#ifndef QT_NO_CLIPBOARD
	QAction *actionCut;
	QAction *actionCopy;
	QAction *actionPaste;
#endif

	QComboBox *comboListFormat;
	QFontComboBox *comboFont;
	QComboBox *comboSize;

	QToolBar *tb;
};

} // namespace QtExt

#endif // QtExt_RichTextEditWidgetH
