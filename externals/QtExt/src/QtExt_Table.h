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

#ifndef QtExt_TableH
#define QtExt_TableH

#include <vector>

#include <QObject>
#include <QVector>
#include <QLineF>
#include <QFont>

#include "QtExt_TableCell.h"
#include "QtExt_CellSizeFormater.h"
#include "QtExt_MergedCells.h"
#include "QtExt_LineProperties.h"

class QPainter;
class QTextDocument;

namespace QtExt {

class TablePrepare {
public:
	TablePrepare() :
		m_cols(1),
		m_rows(2),
		m_outerFrameWidth(1),
		m_innerFrameWidth(1),
		m_margin(0),
		m_width(100),
		m_backGround(Qt::white),
		m_headerCount(1)
	{}

	void setSize(unsigned int cols, unsigned int rows, unsigned int headerRows) {
		m_cols = cols;
		m_rows = rows;
		m_headerCount = headerRows;
	}

	TablePrepare get(unsigned int cols, unsigned int rows, unsigned int headerRows, qreal width) {
		TablePrepare res = *this;
		res.setSize(cols, rows, headerRows);
		res.m_width = width;
		return res;
	}

	unsigned int	m_cols;
	unsigned int	m_rows;
	qreal			m_outerFrameWidth;
	qreal			m_innerFrameWidth;
	qreal			m_margin;
	double			m_width;
	QColor			m_backGround;
	bool			m_headerCount;
};


/*! \brief Class Table allows to create and draw a table with HTML-formated text.*/
class Table : public QObject
{
Q_OBJECT
Q_DISABLE_COPY(Table)

public:
	/*! Constructor for use of shared textDocuments.
		\param textDocument QTextDocument pointer. If 0 a new textDocument will be created and this class instance has ownership.
		\param size Table size.
		\param parent Parent object is responsible for delete.
	*/
	Table(QTextDocument* textDocument, bool adaptive, QSize size = QSize(), QObject *parent = 0);

	/*! Destructor.
		Deletes m_textDocument if necessary.
	*/
	~Table();

//	/*! Clones the actual table.*/
//	Table* clone() const;

	/*! Set the table from given prepare object.*/
	void set(const TablePrepare& prep);

	/*! Sets the table size.
		\param cols Column count.
		\param rows Row count.
	*/
	void setColumnsRows(unsigned int cols, unsigned int rows, unsigned int headerRows = 0);

	/*! Set the visible table rectangle (width and height).*/
	void setTableSize(QSize size);

	/*! Returns the current table size.*/
	QSizeF tableSize() const;

	/*! Returns the table rectangle. The height will be recalculated by using given width.
		\param paintDevice Current paint device.
		\param width Preferred width of the table. If 0 width of paintDevice will be used.
	*/
	QRectF tableRect(QPaintDevice* paintDevice, qreal width = 0);

	/*! Sets the text for the given cell. It can be a normal tect or HTML-formated text.
		\param col Column.
		\param row Row.
		\param text Simple text or HTML sequence.
	*/
	void setCellText(unsigned int col, unsigned int row, const QString& text, Qt::Alignment alignment = Qt::AlignLeft);

	/*! Returns the cell spacing.*/
	qreal spacing() const;

	/*! Sets a new cell spacing.
		\param spacing New cell spacing.
	*/
	void setSpacing(qreal spacing);

	/*! Returns the external table margin.*/
	qreal margin() const;

	/*! Sets a new external table margin.
		\param margin New external table margin (default is 2.0).
	*/
	void setMargin(qreal margin);

	/*! Returns a reference to the cell with the given coordinates.
		\param col Column number.
		\param row Row number.
		Column and row number must be valid (\sa colCount \sa rowCount).
	*/
	TableCell& cell(unsigned int col, unsigned int row);

	/*! Returns a const reference to the cell with the given coordinates.
		\param col Column number.
		\param row Row number.
		Column and row number must be valid (\sa colCount \sa rowCount).
	*/
	const TableCell& cell(unsigned int col, unsigned int row) const;

	/*! Returns the the column count.*/
	unsigned int colCount() const { return m_cols; }

	/*! Returns the the row count.*/
	unsigned int rowCount() const { return m_rows; }

	/*! Sets the background color for the whole table.
		\param color Color for background
		\param allCells If true the background of all cells will also be set.
	*/
	void setBackgroundColor(const QColor& col, bool allCells = false);

	/*! Returns the background color of the whole table.*/
	QColor backgroundColor() const;

	/*! Creates a merge area.
		\param col First column (left).
		\param row First row (top).
		\param numCols Number of columns for merging. Should be bigger than 1.
		\param numRows Number of rows for merging. Should be bigger than 1.
		At least one of numCols or numRows must be greater 1 otherwise it creates an error.
	*/
	bool mergeCells(unsigned int col, unsigned int row, unsigned int numCols, unsigned int numRows);

	/*! Returns the size format type for the given column.
		\param col Column.
		The col parameter must be valid.
	*/
	qreal columnSize(unsigned int col) const;

	/*! Returns the size for the given row.
		\param row Row.
		The row parameter must be valid.
	*/
	qreal rowSize(unsigned int row) const;

	/*! Returns the size for the given column.
		\param col Column.
		The col parameter must be valid.
	*/
	CellSizeFormater::FormatType columnSizeFormat(unsigned int col) const;

	/*! Returns the size format type for the given row.
		\param row Row.
		The row parameter must be valid.
	*/
	CellSizeFormater::FormatType rowSizeFormat(unsigned int row) const;

	/*! Set the size format type for the given column.
		\param col Column index.
		\param format New format type. Only CellSizeFormater::Fixed and CellSizeFormater::AutoMinimum are supported.
		The col parameter must be valid. If the format isn't valid nothing will happen (\sa CellSizeFormater).
	*/
	void setColumnSizeFormat(unsigned int col, CellSizeFormater::FormatType format, qreal fixedSize = 0);

	/*! Set all columns to fixed formats which have a size in the given vector greater than 0.
		The size must be given in percent (of the current width).
	*/
	void setFixedColumnSizes(std::vector<qreal> sizes);

	/*! Set the size format type for the given row.
		\param row Row index.
		\param format New format type. Only CellSizeFormater::Fixed and CellSizeFormater::AutoMinimum are supported.
		The row parameter must be valid. If the format isn't valid nothing will happen (\sa CellSizeFormater).
	*/
	void setRowSizeFormat(unsigned int row, CellSizeFormater::FormatType format, qreal fixedSize = 0);

	/*! Set the margins for all cells in the given column.
		Positiv margin values or 0 set a new margin. Negative values keep the old margin.
		The column number must be valid.
		\param col Column number.
		\param leftMargin Margin on left side.
		\param rightMargin Margin on right side.
		The col parameter must be valid.
	*/
	void setColumnMargins(unsigned int col, qreal leftMargin, qreal rightMargin);

	/*! Set the margins for all cells in the given row.
		Positiv margin values or 0 set a new margin. Negative values keep the old margin.
		The row number must be valid.
		\param row Row number.
		\param topMargin Margin on top side.
		\param bottomMargin Margin on bottom side.
		The row parameter must be valid.
	*/
	void setRowMargins(unsigned int row, qreal topMargin, qreal bottomMargin);

	/*! Sets a line width for the outer table frame.
		\param width Line width for frame. Must not be below 0.
	*/
	void setOuterFrameWidth(qreal width);

	/*! Sets a line width for the inner table frame. Set also the frame width of all cells.
		\param width Line width for frame. Must not be below 0.
	*/
	void setInnerFrameWidth(qreal width);

	/*! Set the border line width between two cols in a given row range.
		\param leftCol Index of left column (right border will be changed)
		\param rowStart First row
		\param rowEnd Last row
		\param lineWidth Width of the border line. Use 0 for delting border
	*/
	void setColumnBorderWidth(unsigned int leftCol, unsigned int rowStart, unsigned int rowEnd, qreal lineWidth);

	/*! Returns the default font.*/
	QFont defaultFont() const;

	/*! Sets a default font.*/
	void setDefaultFont(const QFont& font);

	/*! Sets the default style sheet (css).
		If new style sheet is different a complete adjustment is necessary.
		Prefer to use this function before add any text.
	*/
	void setDefaultStyleSheet ( const QString & sheet );

	/*! Calculates the column widths and row heights according to the current format options and size of the table.
		\param paintDevice Current paint device.
	*/
	void adjust(QPaintDevice* paintDevice);

	/*! Forces a rapint of the table.*/
	void repaint();

	/*! Clears the whole content and set column and row count to 0.
	   Before use this table at least setColumnsRows and setTableSize must be called.
	*/
	void clear();

	/*! Draw the whole table with the given painter.
		\param painter Painter used for drawing.
		\param pos Left/Top corner.
	*/
	void drawTable(QPainter* painter, const QPointF& pos);

	/*! Returns the internal text document.*/
	const QTextDocument* textDocument() const { return m_textDocument; }

	/*! Calculates how many tables are necessary in order to fit on areas with the given size.
		It return a vector of indexes of end rows for each sub table.
	*/
	std::vector<unsigned int> fittingTableRows(QPaintDevice* paintDevice, qreal hfirst, qreal hrest) const;

	/*! Create number of tables which fits in the given heights.*/
	std::vector<Table*> fittingTables(QPaintDevice* paintDevice, qreal hfirst, qreal hrest);

	/*! Return if the adaptive mode is switched on.*/
	bool adaptive() const { return m_adaptive; }

	/*! Draw a frame around the given cell area with given width.*/
	static void frameRect(Table& table, int cLeft, int cright, int rTop, int rBottom, int lineWidth);

signals:
	void changed();

private:

	QTextDocument*							m_textDocument;			///< Internal document for formating text.
	bool									m_textDocumentOwner;	///< True if the class instance is owner of the text document.
	unsigned int							m_cols;					///< Coumn count.
	unsigned int							m_rows;					///< Row count.
	unsigned int							m_headerRows;			///< Number of header rows
	std::vector<std::vector<TableCell> >	m_cells;				///< Cell array.
	QVector<CellSizeFormater>				m_columnWidths;			///< Column format vector.
	QVector<CellSizeFormater>				m_rowHeights;			///< Row format vector.
	qreal									m_spacing;				///< Global spacing.
	qreal									m_margin;				///< External table margins.
	qreal									m_outerFrameWidth;		///< Frame width for the outer table frame;
	qreal									m_innerFrameWidth;		///< Frame width for the inner table lines;
	QColor									m_background;			///< Background of the whole table;
	QVector<MergedCells>					m_mergedCells;			///< List of merging areas.
	QSize									m_size;					///< Size of the table rect.
	QVector<QVector<LineProperties> >		m_HLines;				///< Horizontal line array.
	QVector<QVector<LineProperties> >		m_VLines;				///< Vertical line array.
	qreal									m_scale;				///< Scale factor from screen resolution to paint device resoultion.
	bool									m_adjusted;				///< Saves the adjustment state.
	bool									m_adaptive;				///< Table cells are adaptive or not

	/*! Returns the surrounding text rectangle for the given text.
		\param text Text string.
		\param width Maximum text width.
	*/
	QRectF textRect(const QString& html, qreal width) const;

	/*! Paints HTML formated text at the given position.
		\param col Column number. Must be valid.
		\param row Row number. Must be valid.
		\param painter Painter used for drawing.
		Text and formating option are given by m_cells.
	*/
	void paintCellText(unsigned int col, unsigned int row, QPainter* painter, const QPointF& pos);

	/*! Paints the lines for all cells.
		\param painter Painter used for drawing.
	*/
	void paintCellRects(QPainter* painter, const QPointF& pos);

	/*! Returns the index of the given cell in the merge list or -1.*/
	int mergeIndex(unsigned int col, unsigned int row) const;

	/*! Returns true if the given cell is merged and the merged area has more than one row.*/
	bool multiRowMerge(unsigned int col, unsigned int row) const;

	/*! Returns true if the given cell is merged and the merged area has more than one column.*/
	bool multiColMerge(unsigned int col, unsigned int row) const;

	/*! Returns the scaled value for unusable horizontal space in the given cell.
		This is calculated from table spacing and margins.
		\param cell Current cell.
	*/
	qreal cellWidthSpace(const TableCell& cell) const;

	/*! Returns the scaled value for unusable vertical space in the given cell.
		This is calculated from table spacing and margins.
		\param cell Current cell.
	*/
	qreal cellHeightSpace(const TableCell& cell) const;

	/*! Set paint properties for all cells based on current text document.*/
	void setPaintProperties();

	/*! Calculates the maximum cell width in each column for unmerged cells based on text size and format properties.
		\param maxCellWidthsUnmerged Vector of maximum cell width for each column.
	*/
	void calcMaxCellWidthsUnmerged(QVector<qreal>& maxCellWidthsUnmerged);

	/*! Calculates cell rects for merged cells according necessary space for text.
		maxCellWidthsUnmerged can be changed if current space is not enough.
		This is only a preliminary calculation. Calculation of text space is without any constraints.
		Only the currently existing maximum cell width vector is taking into account.
		\param maxCellWidthsUnmerged Vector of maximum cell width for each column.
	*/
	void calcCellRectsMergedFromText(QVector<qreal>& maxCellWidthsUnmerged);

	/*! Calculates new column widths in case of available space is bigger then necessary.
		\param availableTotalWidth Available width for whole table.
		\param currentTotalWidth Current width of whole table.
		\param maxCellWidthsUnmerged Vector of maximum cell width for each column.
	*/
	void calcColumnWidthsExpanding(qreal availableTotalWidth, qreal currentTotalWidth, const QVector<qreal>& maxCellWidthsUnmerged);

	/*! Calculates new column widths in case of available space is smaller then necessary.
		\param availableTotalWidth Available width for whole table.
		\param currentTotalWidth Current width of whole table.
		\param maxCellWidthsUnmerged Vector of maximum cell width for each column.
	*/
	void calcColumnWidthsShrinking(qreal availableTotalWidth, qreal currentTotalWidth, const QVector<qreal>& maxCellWidthsUnmerged);

	/*! Calculates width for cell rects for merged cells according current column widths (m_columnWidths).
		The rect height is set to 0. The cell rect is not valid after this calculation.
		It is necessary to calculate cell heights later (calcCellRectsHeightMergedFromRowHeights).
	*/
	void calcCellRectsWidthMergedFromColWidth();

	/*! Calculates the maximum cell height in rows for the given text and fixed column width.
		\param maxCellHeightUnmerged Vector of maximum cell height for each row.
	*/
	void calcMaxCellHeightsUnmerged(QVector<qreal>& maxCellHeightUnmerged);

	/*! Calculates height of cell rects for merged cells according necessary space for text.
		maxCellWidthsUnmerged can be changed if current space is not enough.
		The currently existing cell width is taken into account for text height calculation.
		Therefore it must be precalculated.
		\param maxCellWidthsUnmerged Vector of maximum cell width for each column.
	*/
	void calcCellRectsHeightMergedFromRowHeights(QVector<qreal>& maxCellHeightUnmerged);

	/*! Calculates new row heights in case of available space is bigger then necessary.
		\param availableTotalHeight Available height for whole table (page height).
		\param currentTotalHeight Current height of whole table.
		\param maxCellHeightUnmerged Vector of maximum cell heights for each row.
	*/
	void calcRowHeightsExpanding(qreal availableTotalHeight, qreal currentTotalHeight, const QVector<qreal>& maxCellHeightUnmerged);

	/*! Set cell rects for all cells without consideration of merging
		based on current vectors for columnWidths and rowHeights.
	*/
	void setCellRects();

	/*! Calculates the maximum available space (rectangle) for text based on current cell rects, margins and spacing.
		Call of setCellRects before this function.
	*/
	void setCellMaximumTextRects();

	/*! Set cell rects for cells in merged areas based on current vectors for columnWidths and rowHeights
		and from current cell rects.
		Call of setCellRects before this function.
	*/
	void setCellRectsMerged();

	/*! Calculates the maximum available space (rectangle) for merged areas
		based on cellRects and mergedCellRects.
		Call of setCellRects and before setCellRectsMerged this function.
	*/
	void setCellMaximumTextRectsMerged();

	/*! Calculates the rectangle inside the cell for text based on maximum text rect.
		Call of setCellMaximumTextRects before this function.
	*/
	void setCellTextRects();

	/*! Calculates the rectangle inside a merged cell area for text based on maximum text rect.
		Call of setCellMaximumTextRectsMerged before this function.
	*/
	void setCellTextRectsMerged();

	/*! Fills the horizontal lines matrix.
		Calculates begin and end positions and thickness based on current cell rects.
		Call of setCellRects before this function.
	*/
	void calcHorizontalLines();

	/*! Fills the vertical lines matrix.
		Calculates begin and end positions and thickness based on current cell rects.
		Call of setCellRects before this function.
	*/
	void calcVerticalLines();

	/*! Create a table which contains only the rows from startRow to endRow and the header rows.*/
	Table* createSubTable(unsigned int startRow, unsigned int endRow);
};

/*! @file QtExt_Table.h
  @brief Contains the declaration of the class Table.
*/

} // namespace QtExt

#endif // QtExt_TableH
