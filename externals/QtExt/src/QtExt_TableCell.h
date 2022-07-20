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

#ifndef QtExt_TableCellH
#define QtExt_TableCellH

#include <Qt>
#include <QRectF>
#include <QString>
#include <QColor>

#include "QtExt_TextFrameInformations.h"

namespace QtExt {

class Table;

/*! \brief Class TableCell contains all properties for a cell in a table.*/
/*! This class is used in QtExt::TableWidget.*/
class TableCell
{
public:
	/*! Specifies the cell border.
		The first 4 elements can be used together (combined by using or).
		WholeFrame is the same like: \code LeftBorder | RightBorder | TopBorder | BottomBorder \endcode.
	*/
	enum CellBorder { LeftBorder =   0x0001,
					  RightBorder =  0x0002,
					  TopBorder =    0x0004,
					  BottomBorder = 0x0008,
					  WholeFrame =   0x000F
					};

	/*! Default constructor.
		\param width Line width for the cell frame.
	*/
	explicit TableCell(qreal width);

	/*! Copy constructor.*/
	TableCell(const TableCell&);

	/*! Copy assignment operator.*/
	TableCell& operator=(TableCell);

	/*! Specialized swap function. Used in copy assignment operator.*/
	void Swap(TableCell&);

	/*! Sets the width of the border \a border.
		\param border Border to be set (see CellBorder).
		\param width Border width (solid line). Must be a positive value or 0.
	*/
	void setBorderWidth(CellBorder border, qreal width);

	/*! Returns the border width.
		\param border Border to be set (see CellBorder).
	*/
	qreal borderWidth(CellBorder border) const;

	/*! Sets the alignment of the text in the cell.
		\param alignment One or a valid combination of alignments.
	*/
	void setAlignment(Qt::Alignment alignment);

	/*! Returns the text alignment.*/
	Qt::Alignment alignment() const;

	/*! Sets the cell text.
		It can be a simple text or a HTML sequence.
	*/
	void setText(const QString& text);

	/*! Returns the cell text or HTML sequence.*/
	QString text() const;

	/*! Returns the rectangle surronding the text.*/
	QRectF textRect() const;

	/*! Set the text rect.*/
	void setTextRect(const QRectF& rect);

	/*! Returns the maximum possible rectangle for text.*/
	QRectF maxTextRect() const;

	/*! Set the maximum possible rectangle for text.*/
	void setMaxTextRect(const QRectF& rect);

	/*! Returns the text size for a current maximum width.
		A value of -1 marks infinite width.
	*/
	QSizeF textSize(qreal maxWidth, bool adaptive);

	/*! Return the used maximum width for calculating text width.*/
	qreal textMaxWidth(qreal maxWidth, bool adaptive);

	/*! Returns the cell rectangle.*/
	QRectF cellRect() const;

	/*! Set the cell rect.*/
	void setCellRect(const QRectF& rect);

	/*! Returns the background color of the cell.*/
	QColor backgroundColor() const;

	/*! sets the background color of the cell.*/
	void setBackgroundColor(const QColor& col);

	/*! Returns the left margin.*/
	qreal leftMargin() const;

	/*! Sets the left margin.*/
	void setLeftMargin(qreal margin);

	/*! Returns the right margin.*/
	qreal rightMargin() const;

	/*! Sets the right margin.*/
	void setRightMargin(qreal margin);

	/*! Returns the top margin.*/
	qreal topMargin() const;

	/*! Sets the top margin.*/
	void setTopMargin(qreal margin);

	/*! Returns the bottom margin.*/
	qreal bottomMargin() const;

	/*! Sets the bottom margin.*/
	void setBottomMargin(qreal margin);

	/*! Returns the vertical.*/
	qreal verticalOffset() const;

	/*! Sets the vertical.*/
	void setVerticalOffset(qreal offset);

	/*! Returns the merge flag.*/
	bool merged() const  { return m_merged; }

	/*! Set merge state.*/
	void setMerged(bool merged);

	/*! Return if the text should be drawn vertically.*/
	bool verticalText() const { return m_verticalText; }

	/*! Set vertical text option.*/
	void setVerticalText(bool vertical);

	/*! Set the paint properties for getting text metrics.
		Call of this this function is necessary before call of textSize.
		\param textDocument should contain the correct paintDevice and font.
	*/
	void setPaintProperties(QTextDocument* textDocument, bool adaptive);

private:
	qreal			m_leftBorderWidth;		///< Width of the left border.
	qreal			m_rightBorderWidth;		///< Width of the right border.
	qreal			m_topBorderWidth;		///< Width of the top border.
	qreal			m_bottomBorderWidth;	///< Width of the bottom border.
	qreal			m_leftMargin;			///< Width of the left border.
	qreal			m_rightMargin;			///< Width of the right border.
	qreal			m_topMargin;			///< Width of the top border.
	qreal			m_bottomMargin;			///< Width of the bottom border.
	Qt::Alignment	m_alignment;			///< Text alignment.
	QString			m_text;					///< Cell text.
	QRectF			m_textRect;				///< Rectangle surrounding the text.
	QRectF			m_maxTextRect;			///< Maximum possible rectangle for text.
	QRectF			m_cellRect;				///< Cell rectangle.
	QColor			m_backgroundColor;		///< Background color (default white).
	bool			m_merged;				///< Cell is merged with one or more cells.
	qreal			m_verticalOffset;		///< Vertical offset for the text of a cell as factor of text height.
	TextFrameInformations	m_textInfos;	///< Helds possible textRects.
	bool			m_verticalText;			///< If true text should be drawn vertically

//	friend class Table;
};

/*! @file QtExt_TableCell.h
  @brief Contains the declaration of the class TableCell.
*/

} // namespace QtExt

#endif // QtExt_TableCellH
