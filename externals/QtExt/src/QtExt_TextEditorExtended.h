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

#ifndef QtExt_TextEditorExtendedH
#define QtExt_TextEditorExtendedH

#include <QPlainTextEdit>


namespace QtExt {

/*! Extended plain text editor with line numbering and additional features.*/
class TextEditorExtended : public QPlainTextEdit
{
public:
	TextEditorExtended(QWidget *parent = nullptr);

	/*! Paint event for the line numbering area.*/
	void lineNumberAreaPaintEvent(QPaintEvent *event);

	/*! Calculates width of line numbering area based on current font and total number of lines.*/
	int lineNumberAreaWidth();

	/*! Set background color for the given row area.*/
	void colorRows(unsigned int startRow, unsigned int endRow, QColor color);

	/*! Clears all row coloring areas.*/
	void clearRowColoring();

	/*! Add a column marker rect.*/
	void addColumnMarker(int row, int col, int width, const QColor& color = Qt::blue);

	void clearColumnMarker();

protected:
	/*! Adapt the line numbering area.*/
	void resizeEvent(QResizeEvent *event) override;

	/*! Add additional graphical elements.*/
	void paintEvent(QPaintEvent* event) override;

private:
	struct ColumnMarker {
		ColumnMarker(TextEditorExtended*	edit) :
			m_edit(edit),
			m_startLine(0),
			m_startColumn(0),
			m_width(0),
			m_startLinePos(0),
			m_startColumnPos(0),
			m_endColumnPos(0),
			m_color(Qt::blue)
		{}

		ColumnMarker(TextEditorExtended*	edit, int line, int column, int width) :
			m_edit(edit),
			m_startLine(line),
			m_startColumn(column),
			m_width(width),
			m_startLinePos(0),
			m_startColumnPos(0),
			m_endColumnPos(0),
			m_color(Qt::blue)
		{}

		void updatePosition();

		TextEditorExtended*	m_edit;
		int					m_startLine;
		int					m_startColumn;
		int					m_width;
		int					m_startLinePos;
		int					m_startColumnPos;
		int					m_endColumnPos;
		QColor				m_color;
	};

	/*! Updates the line numbering area width in case of change in edit block number.*/
	void updateLineNumberAreaWidth(int newBlockCount);

	/*! Highlights the current line with yellow color.*/
	void highlightCurrentLine();

	/*! Updates the line numbering area in case of change in edit content.*/
	void updateLineNumberArea(const QRect &, int);

	/*! Update internal line height. The editor should have only one font without formats.*/
	void updateHeightWidth();

	QWidget *				m_lineNumberArea;	///< Widget with line numbers on left hjand side
	int						m_headerEndRow;
	QList<ColumnMarker>		m_columnMarkers;
	int						m_lineHeight;
	int						m_topMargin;
	int						m_charWidth;
	int						m_leftMargin;
};

class LineNumberAereaWidget : public QWidget
{
	Q_OBJECT
public:
	explicit LineNumberAereaWidget(TextEditorExtended *editor);

	QSize sizeHint() const override;

protected:
	void paintEvent(QPaintEvent *event) override;

private:
	TextEditorExtended*	m_editor;
};

} // namespace QtExt


#endif // QtExt_TextEditorExtendedH
