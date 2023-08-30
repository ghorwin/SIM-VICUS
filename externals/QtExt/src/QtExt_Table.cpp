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

#include <numeric>
#include <algorithm>

#include <QTextBlock>
#include <QTextDocument>
#include <QTextLayout>
#include <QPointF>
#include <QRectF>
#include <QPainter>
#include <QDebug>
#include <QWidget>
#include <QApplication>
#include <QDesktopWidget>
#include <QAbstractTextDocumentLayout>
#include "QtExt_Table.h"

namespace QtExt {

Table::Table(QTextDocument* textDocument, bool adaptive, QSize size, QObject *parent) :
	QObject(parent),
	m_textDocument(textDocument),
	m_textDocumentOwner(false),
	m_cols(0),
	m_rows(0),
	m_headerRows(0),
	m_spacing(2),
	m_margin(2),
	m_outerFrameWidth(0.0),
	m_innerFrameWidth(1.0),
	m_background(Qt::transparent),
	m_size(size),
	m_scale(1.0),
	m_adjusted(false),
	m_adaptive(adaptive)
{
	if( m_textDocument == nullptr) {
		m_textDocument = new QTextDocument;
		m_textDocumentOwner = true;
	}
}

Table::~Table() {
	if(m_textDocumentOwner)
		delete m_textDocument;
}

void Table::set(const TablePrepare& prep) {
	clear();
	setColumnsRows(prep.m_cols, prep.m_rows, prep.m_headerCount);
	setOuterFrameWidth(prep.m_outerFrameWidth);
	setInnerFrameWidth(prep.m_innerFrameWidth);
	setMargin(prep.m_margin);
	setTableSize(QSize(prep.m_width, -1));
	setBackgroundColor(Qt::white, true);
	if(prep.m_headerCount > 0) {
		for( unsigned int i=0; i<colCount(); ++i) {
			cell(i,prep.m_headerCount-1).setBorderWidth(QtExt::TableCell::BottomBorder, prep.m_outerFrameWidth);
		}
	}
}


void Table::clear() {
	m_cells.clear();
	m_HLines.clear();
	m_VLines.clear();
	m_columnWidths.clear();
	m_rowHeights.clear();
	m_adjusted = false;
	m_cols = 0;
	m_rows = 0;
	m_headerRows = 0;
	m_scale = 1.0;
	m_spacing = 2;
	m_margin = 2;
	m_outerFrameWidth = 0.0;
	m_innerFrameWidth = 1.0;
	m_background = QColor(Qt::transparent);
	m_textDocument->clear();
	m_size = QSize();
	m_mergedCells.clear();
}

void Table::setTableSize(QSize size) {
	// same size - no adjustment necessary
	if( m_size == size)
		return;

	m_size = size;
	m_adjusted = false;
	emit changed();
}

void Table::setColumnsRows(unsigned int cols, unsigned int rows, unsigned int headerRows) {
	// set here because has no influence on sizing
	m_headerRows = headerRows;

	// no change - no adjustment necessary
	if( m_cols == cols && m_rows == rows)
		return;

	m_cols = cols;
	m_rows = rows;

	std::vector<std::vector<TableCell> >(cols, std::vector<TableCell>(rows, TableCell(m_innerFrameWidth))).swap(m_cells);
	m_HLines = QVector<QVector<LineProperties> >(cols , QVector<LineProperties>(rows + 1));
	m_VLines = QVector<QVector<LineProperties> >(cols + 1, QVector<LineProperties>(rows));
	m_columnWidths.resize(cols);
	m_rowHeights = QVector<CellSizeFormater>(rows, CellSizeFormater(CellSizeFormater::AutoMinimum));
	m_adjusted = false;
//	emit changed();
}

void Table::setCellText(unsigned int col, unsigned int row, const QString& text, Qt::Alignment alignment) {
	Q_ASSERT(col < m_cols);
	Q_ASSERT(row < m_rows);

	m_cells[col][row].setText(text);
	m_cells[col][row].setAlignment(alignment);
	m_adjusted = false;
	emit changed();
}

TableCell& Table::cell(unsigned int col, unsigned int row) {
	Q_ASSERT(col < m_cols);
	Q_ASSERT(row < m_rows);

	m_adjusted = false;
	return m_cells[col][row];
}

const TableCell& Table::cell(unsigned int col, unsigned int row) const {
	Q_ASSERT(col < m_cols);
	Q_ASSERT(row < m_rows);

	return m_cells[col][row];
}

void Table::setSpacing(qreal spacing) {
	if( m_spacing != spacing) {
		m_spacing = spacing;
		m_adjusted = false;
		emit changed();
	}
}

qreal Table::margin() const {
	return m_margin;
}

void Table::setMargin(qreal margin) {
	if( m_margin != margin) {
		m_margin = margin;
		m_adjusted = false;
		emit changed();
	}
}

qreal Table::spacing() const {
	return m_spacing;
}

void Table::setBackgroundColor(const QColor& col, bool allCells) {
	if( m_background != col) {
		m_background = col;
	}
	if( allCells ) {
		for( unsigned int r=0; r<m_rows; ++r){
			for( unsigned int c=0; c<m_cols; ++c) {
				m_cells[c][r].setBackgroundColor(col);
			}
		}
	}
}

QColor Table::backgroundColor() const {
	return m_background;
}

int Table::mergeIndex(unsigned int col, unsigned int row) const {
	Q_ASSERT(col < m_cols);
	Q_ASSERT(row < m_rows);

	for( int i=0, count = m_mergedCells.size(); i<count; ++i) {
		const MergedCells& mc = m_mergedCells[i];
		if( col >= mc.firstCol && col <= mc.lastCol && row >= mc.firstRow && row <= mc.lastRow)
			return i;
	}
	return -1;
}

bool Table::multiColMerge(unsigned int col, unsigned int row) const {
	Q_ASSERT(col < m_cols);
	Q_ASSERT(row < m_rows);

	int index = mergeIndex(col, row);
	if( index == -1)
		return false;

	// no check for index necessary because mergeIndex cannot create a wrong one
	const MergedCells& mc = m_mergedCells[index];
	return mc.lastCol > mc.firstCol;
}

bool Table::multiRowMerge(unsigned int col, unsigned int row) const {
	Q_ASSERT(col < m_cols);
	Q_ASSERT(row < m_rows);

	int index = mergeIndex(col, row);
	if( index == -1)
		return false;

	// no check for index necessary because mergeIndex cannot create a wrong one
	const MergedCells& mc = m_mergedCells[index];
	return mc.lastRow > mc.firstRow;
}

bool Table::mergeCells(unsigned int col, unsigned int row, unsigned int numCols, unsigned int numRows) {
	Q_ASSERT(numCols > 1 || numRows > 1);

	const unsigned int lastCol = col + numCols - 1;
	const unsigned int lastRow = row + numRows - 1;
	if( lastCol >= m_cols || lastRow >= m_rows)
		return false;

	for( unsigned int j=row; j<row + numRows; ++j){
		for( unsigned int i=col; i<col + numCols; ++i) {
			if( m_cells[i][j].merged())
				return false;
		}
	}
	for( unsigned int j=row; j<row + numRows; ++j){
		for( unsigned int i=col; i<col + numCols; ++i) {
			m_cells[i][j].setMerged(true);
		}
	}
	m_mergedCells.push_back(MergedCells(col, row, col + numCols - 1, row + numRows - 1));
	m_mergedCells.back().m_verticalText = m_cells[col][row].verticalText();
	m_adjusted = false;
	return true;
}

qreal Table::columnSize(unsigned int col) const {
	Q_ASSERT(col < m_cols);

	return m_columnWidths[col].m_size;
}

qreal Table::rowSize(unsigned int row) const {
	Q_ASSERT(row < m_rows);

	return m_rowHeights[row].m_size;
}

CellSizeFormater::FormatType Table::columnSizeFormat(unsigned int col) const {
	Q_ASSERT(col < m_cols);

	return m_columnWidths[col].m_format;
}

CellSizeFormater::FormatType Table::rowSizeFormat(unsigned int row) const {
	Q_ASSERT(row < m_rows);

	return m_rowHeights[row].m_format;
}

void Table::setColumnSizeFormat(unsigned int col, CellSizeFormater::FormatType format, qreal fixedSize) {
	Q_ASSERT(col < m_cols);
	// currently only these two types are supported
	if(format == CellSizeFormater::TableWidth || format == CellSizeFormater::Unknown)
		return;

	if( m_columnWidths[col].m_format != format || format == CellSizeFormater::Fixed) {
		m_columnWidths[col].m_format = format;
		m_adjusted = false;
		if( format == CellSizeFormater::Fixed)
			m_columnWidths[col].m_fixedSize = fixedSize;
		emit changed();
	}
}

void Table::setFixedColumnSizes(std::vector<qreal> sizes) {
	m_adjusted = false;
	qreal width = m_size.width();
	unsigned int count = std::min((size_t)m_cols, sizes.size());
	for(size_t i=0; i<m_cols; ++i) {
		if(i<sizes.size()) {
			if(sizes[i] <= 0) {
				m_columnWidths[i].m_format = CellSizeFormater::AutoMinimum;
			}
			else {
				m_columnWidths[i].m_format = CellSizeFormater::Fixed;
				m_columnWidths[i].m_fixedSize = width * sizes[i] / 100.0;
			}
		}
		else {
			m_columnWidths[i].m_format = CellSizeFormater::AutoMinimum;
		}
	}
	emit changed();
}


void Table::setRowSizeFormat(unsigned int row, CellSizeFormater::FormatType format, qreal fixedSize) {
	Q_ASSERT(row < m_rows);
	// currently only these two types are supported
	if(format == CellSizeFormater::TableWidth || format == CellSizeFormater::Unknown)
		return;

	if( m_rowHeights[row].m_format != format || format == CellSizeFormater::Fixed) {
		m_rowHeights[row].m_format = format;
		m_adjusted = false;
		if( format == CellSizeFormater::Fixed)
			m_rowHeights[row].m_fixedSize = fixedSize;
		emit changed();
	}
}

void Table::setColumnMargins(unsigned int col, qreal leftMargin, qreal rightMargin) {
	Q_ASSERT( col < m_cols);

	for( unsigned int r=0; r<m_rows; ++r){
		if( leftMargin >= 0)
			m_cells[col][r].setLeftMargin(leftMargin);
		if( rightMargin >= 0)
			m_cells[col][r].setRightMargin(rightMargin);
	}
}

void Table::setRowMargins(unsigned int row, qreal topMargin, qreal bottomMargin) {
	Q_ASSERT( row < m_rows);

	for( unsigned int c=0; c<m_cols; ++c){
		if( topMargin >= 0)
			m_cells[c][row].setTopMargin(topMargin);
		if( bottomMargin >= 0)
			m_cells[c][row].setBottomMargin(bottomMargin);
	}
}

void Table::setOuterFrameWidth(qreal width) {
	Q_ASSERT(width >= 0);

	if( m_outerFrameWidth != width) {
		m_outerFrameWidth = width;
		m_adjusted = false;
		emit changed();
	}
}

void Table::setInnerFrameWidth(qreal width) {
	Q_ASSERT(width >= 0);

	if( m_innerFrameWidth != width) {
		m_innerFrameWidth = width;
		m_adjusted = false;
		for( unsigned int r=0; r<m_rows; ++r){
			for( unsigned int c=0; c<m_cols; ++c) {
				m_cells[c][r].setBorderWidth(TableCell::WholeFrame, width);
			}
		}
		emit changed();
	}
}

void Table::setColumnBorderWidth(unsigned int leftCol, unsigned int rowStart, unsigned int rowEnd, qreal lineWidth) {
	Q_ASSERT( rowStart < m_rows);
	Q_ASSERT( rowEnd < m_rows);
	Q_ASSERT( leftCol < m_cols);

	if(leftCol == m_cols-1)
		return;

	for(unsigned int ri=rowStart; ri<=rowEnd; ++ri) {
		m_cells[leftCol][ri].setBorderWidth(TableCell::RightBorder, lineWidth);
		m_cells[leftCol+1][ri].setBorderWidth(TableCell::LeftBorder, lineWidth);
	}
}


QFont Table::defaultFont() const {
	return m_textDocument->defaultFont();
}

void Table::setDefaultFont(const QFont& font) {
	m_textDocument->setDefaultFont(font);
	m_adjusted = false;
	emit changed();
}

void Table::setDefaultStyleSheet ( const QString & sheet ) {
	QString oldSheet = m_textDocument->defaultStyleSheet();
	if(oldSheet == sheet)
		return;

	m_textDocument->setDefaultStyleSheet(sheet);
	m_adjusted = false;
	emit changed();
}

void Table::repaint() {
	emit changed();
}

// private helper functions for adjust

qreal Table::cellWidthSpace(const TableCell& cell) const {
	return (2 * m_spacing + cell.leftMargin() + cell.rightMargin()) * m_scale;
}

qreal Table::cellHeightSpace(const TableCell& cell) const {
	return (2 * m_spacing + cell.topMargin() + cell.bottomMargin()) * m_scale;
}

void Table::setPaintProperties() {
	TextFrameInformations::m_pixelStep = std::max(m_rows / 20, 5u);
	for( unsigned int c=0; c<m_cols; ++c) {
		// set text properties from text document
		for( unsigned int r=0; r<m_rows; ++r) {
			m_cells[c][r].setPaintProperties(m_textDocument, m_adaptive);
		}
	}
}

void Table::calcMaxCellWidthsUnmerged(QVector<qreal>& maxCellWidthsUnmerged) {
	maxCellWidthsUnmerged.resize((int)m_cols);
	for( unsigned int c=0; c<m_cols; ++c) {
		// look for maximum sum of left and right margins in current column
		qreal maxMarginSum = 0;
		for( unsigned int r=0; r<m_rows; ++r) {
			const TableCell& cell = m_cells[c][r];
			if( !cell.merged()) {
				maxMarginSum = std::max(maxMarginSum, qreal(cell.leftMargin() + cell.rightMargin()));
			}
		}
		// get column width
		qreal tmw(0);
		// for fixed column width
		if( m_columnWidths[(int)c].m_format == CellSizeFormater::Fixed) {
			tmw = m_columnWidths[(int)c].m_fixedSize - (2 * m_spacing + maxMarginSum) * m_scale;;
		}
		// for automatic look for maximum cell width in the current column
		else {
			for( unsigned int r=0; r<m_rows; ++r) {
				TableCell& cell = m_cells[c][r];
				if( !cell.merged()) {
					QSizeF textSize = cell.textSize(-1, m_adaptive);
					tmw = std::max(tmw, textSize.width());
				}
			}
		}
		maxCellWidthsUnmerged[(int)c] = tmw + (2 * m_spacing + maxMarginSum) * m_scale;
	}
}

void Table::calcCellRectsMergedFromText(QVector<qreal>& maxCellWidthsUnmerged) {
	// Take text rectangles for merged cells and
	// adjust column widths in order to fit the text in merged cells
	for( int i=0, count = m_mergedCells.size(); i<count; ++i) {
		MergedCells& mc = m_mergedCells[i];
		qreal mw(0.0);
		// calc total width for all columns in range
		int notFixedCols = 0;
		for( unsigned int c=mc.firstCol; c<=mc.lastCol;++c) {
			mw += maxCellWidthsUnmerged[(int)c];
			if(m_columnWidths[c].m_format != CellSizeFormater::Fixed)
				++notFixedCols;
		}

		TableCell& leftTopCell = m_cells[mc.firstCol][mc.firstRow];
		TableCell& rightTopCell = m_cells[mc.lastCol][mc.firstRow];
		qreal widthMarginSum = leftTopCell.leftMargin() + rightTopCell.rightMargin();

		QSizeF textSize = leftTopCell.textSize(-1, m_adaptive);

		// fits the current text into the merged cell range?
		// calc difference between necessary space for text and current cell width
		qreal wdiff = mw - (2.0 * m_spacing + widthMarginSum) * m_scale - textSize.width();
		// make all columns in range bigger in order to create enough space
		if( wdiff < 0) {
			qreal additional = wdiff / notFixedCols * -1.0;
			mw = 0.0;
			for( unsigned int c=mc.firstCol; c<=mc.lastCol;++c) {
				if(m_columnWidths[c].m_format != CellSizeFormater::Fixed)
					maxCellWidthsUnmerged[(int)c] += additional;
				mw += maxCellWidthsUnmerged[(int)c];
			}
		}
		// save width of merge area (rect not valid)
		mc.m_cellRect = QRectF(0,0,mw,textSize.height());
	}
}

void Table::calcColumnWidthsExpanding(qreal availableTotalWidth, qreal currentTotalWidth, const QVector<qreal>& maxCellWidthsUnmerged) {
	// we have enough space
	// increase column widths in order to fit total size
	qreal restWidth = availableTotalWidth - currentTotalWidth;
	for( int c=0; c<(int)m_cols; ++c) {
		switch(m_columnWidths[c].m_format) {
			case CellSizeFormater::AutoMinimum: {
				m_columnWidths[c].m_size = maxCellWidthsUnmerged[c];
				break;
			}
			case CellSizeFormater::Fixed: {
				m_columnWidths[c].m_size = m_columnWidths[c].m_fixedSize;
				break;
			}
			case CellSizeFormater::TableWidth:
			case CellSizeFormater::Unknown: {
				m_columnWidths[c].m_size = maxCellWidthsUnmerged[c] + restWidth / m_cols;
				break;
			}
		}
	}
}

void Table::calcColumnWidthsShrinking(qreal availableTotalWidth, qreal currentTotalWidth, const QVector<qreal>& maxCellWidthsUnmerged) {
	// we have not enough space
	// shrink some column widths in order to fit total size
	/// \todo Doesnt work correctly, new implementation needed
	qreal missingSpace = currentTotalWidth - availableTotalWidth;

	// create temporary cell width vector for sorting
	QVector<qreal> tmaxWidths(maxCellWidthsUnmerged);
	std::sort(tmaxWidths.begin(), tmaxWidths.end());
	std::reverse(tmaxWidths.begin(), tmaxWidths.end());

	qreal save(0);
	qreal maxwidth(availableTotalWidth / m_cols);

	for( unsigned int c=1; c<m_cols; ++c) {
		save += tmaxWidths[0] - tmaxWidths[(int)c];
		if( save >= missingSpace) {
			qreal restsum = std::accumulate(tmaxWidths.begin() + c, tmaxWidths.end(), 0.0);
			maxwidth = (availableTotalWidth - restsum) / c;
			break;
		}
	}

	for( int c=0; c<(int)m_cols; ++c) {
		if( maxCellWidthsUnmerged[c] > maxwidth)
			m_columnWidths[c].m_size = maxwidth;
		else
			m_columnWidths[c].m_size = maxCellWidthsUnmerged[c];
	}
}

void Table::calcCellRectsWidthMergedFromColWidth() {
	// new calculation of cell widths for merged areas
	for( int i=0, count = m_mergedCells.size(); i<count; ++i) {
		MergedCells& mc = m_mergedCells[i];
		qreal mw(0.0);
		for( unsigned int c=mc.firstCol; c<=mc.lastCol;++c)
			mw += m_columnWidths[(int)c].m_size;
		// save width of merge area (rect not valid)
		mc.m_cellRect = QRectF(0,0,mw,0);
	}
}

void Table::calcMaxCellHeightsUnmerged(QVector<qreal>& maxCellHeightUnmerged) {
	maxCellHeightUnmerged.resize((int)m_rows);
	// Take text rectangles for all cells with width constraint and
	// creates a list of maximum cell height of each row
	for( unsigned int r=0; r<m_rows; ++r) {
		qreal tmh(0);
		qreal maxMarginSum = 0;
		for( unsigned int c=0; c<m_cols; ++c) {
			TableCell& cell = m_cells[c][r];
			if( !cell.merged()) {
				QSizeF textSize;
				if(cell.verticalText()) {
					textSize = cell.textSize(-1, m_adaptive);
				}
				else {
					textSize = cell.textSize(m_columnWidths[(int)c].m_size - cellWidthSpace(cell), m_adaptive);
				}
				tmh = std::max(tmh, textSize.height());
				maxMarginSum = std::max(maxMarginSum, cell.topMargin() + cell.bottomMargin());
			}
		}
		maxCellHeightUnmerged[(int)r] = tmh + (2 * m_spacing + maxMarginSum) * m_scale;
	}
}

void Table::calcCellRectsHeightMergedFromRowHeights(QVector<qreal>& maxCellHeightUnmerged) {
	for( int i=0, count = m_mergedCells.size(); i<count; ++i) {
		MergedCells& mc = m_mergedCells[i];
		TableCell& cell = m_cells[mc.firstCol][mc.firstRow];
		qreal mh(0.0);
		for( unsigned int r=mc.firstRow; r<=mc.lastRow;++r)
			mh += maxCellHeightUnmerged[(int)r];
		QSizeF textSize;
		if(cell.verticalText()) {
			textSize = cell.textSize(-1, m_adaptive);
		}
		else {
			textSize = cell.textSize(mc.m_cellRect.width(), m_adaptive);
		}
		qreal textHeight = textSize.height();
		qreal topMargin = cell.topMargin();
		qreal bottomMargin = cell.bottomMargin();

		qreal hdiff = mh - (2* m_spacing +topMargin + bottomMargin) * m_scale - textHeight;
		if( hdiff < 0) {
			qreal additional = hdiff / (mc.lastRow - mc.firstRow + 1) * -1.0;
			mh = 0.0;
			for( unsigned int r=mc.firstRow; r<=mc.lastRow;++r) {
				maxCellHeightUnmerged[(int)r] += additional;
				mh += maxCellHeightUnmerged[(int)r];
			}
		}
		// save height of merge area (rect not valid because position wrong)
		mc.m_cellRect = QRectF(0,0,mc.m_cellRect.width(), mh);
	}
}

void Table::calcRowHeightsExpanding(qreal availableTotalHeight, qreal currentTotalHeight, const QVector<qreal>& maxCellHeightUnmerged) {
	// compare minimum table height with current table height
	// adjust table height if necessary
	qreal restHeight = availableTotalHeight - currentTotalHeight;
	if( restHeight < 0)
		restHeight = 0;
	// we have enough space
	// increase column widths in order to fit total size
	for( int r=0; r<(int)m_rows; ++r) {
		switch(m_rowHeights[r].m_format) {
			case CellSizeFormater::AutoMinimum: {
				m_rowHeights[r].m_size = maxCellHeightUnmerged[r];
				break;
			}
			case CellSizeFormater::Fixed: {
				m_rowHeights[r].m_size = m_rowHeights[r].m_fixedSize;
				break;
			}
			case CellSizeFormater::TableWidth:
			case CellSizeFormater::Unknown: {
				m_rowHeights[r].m_size = maxCellHeightUnmerged[r] + restHeight / m_rows;
			}
		}
	}
}

void Table::setCellRects() {
	// Create rects for single cells without consideration of merging
	qreal xpos(m_margin * m_scale);
	for( unsigned int c=0; c<m_cols; ++c) {
		qreal ypos(m_margin * m_scale);
		for( unsigned int r=0; r<m_rows; ++r) {
			QRectF crect = QRectF(xpos, ypos, m_columnWidths[(int)c].m_size, m_rowHeights[(int)r].m_size);
			m_cells[c][r].setCellRect(crect);
			ypos +=  m_rowHeights[(int)r].m_size;
		}
		xpos += m_columnWidths[(int)c].m_size;
	}
}

void  Table::setCellMaximumTextRects() {
	for( unsigned int c=0; c<m_cols; ++c) {
		for( unsigned int r=0; r<m_rows; ++r) {
			TableCell& currentCell = m_cells[c][r];
			const QRectF& crect = currentCell.cellRect();
			qreal mtleft = crect.x() + (m_spacing + currentCell.leftMargin()) * m_scale;
			qreal mtwidth = crect.width() - (2 * m_spacing + currentCell.leftMargin() + currentCell.rightMargin()) * m_scale;
			qreal mttop = crect.y() + (m_spacing + currentCell.topMargin()) * m_scale;
			qreal mtheight = crect.height() - (2 * m_spacing + currentCell.topMargin() + currentCell.bottomMargin()) * m_scale;
			QRectF maxTextRect = QRectF( mtleft, mttop, mtwidth, mtheight);
			if(currentCell.verticalText())
				maxTextRect = maxTextRect.transposed();
			currentCell.setMaxTextRect(maxTextRect);
		}
	}
}

void  Table::calcHorizontalLines() {
	for( unsigned int c=0; c<m_cols; ++c) {
		for( unsigned int r=0; r<m_rows; ++r) {
			TableCell& currentCell = m_cells[c][r];
			const QRectF& crect = currentCell.cellRect();

			// width for border lines
			qreal hw = currentCell.borderWidth(TableCell::TopBorder);
			if( r > 0)
				hw = std::max(hw, m_cells[c][r - 1].borderWidth(TableCell::BottomBorder));

			int mindex = mergeIndex(c, r);
			if( mindex != -1) {
				const MergedCells& mc = m_mergedCells[mindex];
				if( r !=  mc.firstRow)
					hw = 0;
			}

			m_HLines[c][r] = LineProperties(QLineF(crect.topLeft(), crect.topRight()));
			m_HLines[c][r].m_width = hw;
		}
	}

	// bottom lines
	for( unsigned int c=0; c<m_cols; ++c) {
		const TableCell& currentCell = m_cells[c][m_rows-1];
		const QRectF& crect = currentCell.cellRect();
		m_HLines[c][m_rows] = LineProperties(QLineF(crect.bottomLeft(), crect.bottomRight()));
		m_HLines[c][m_rows].m_width = currentCell.borderWidth(TableCell::BottomBorder);
	}
}

void  Table::calcVerticalLines() {
	for( unsigned int c=0; c<m_cols; ++c) {
		for( unsigned int r=0; r<m_rows; ++r) {
			const TableCell& currentCell = m_cells[c][r];
			const QRectF& crect = currentCell.cellRect();

			m_VLines[c][r] = LineProperties(QLineF(crect.topLeft(), crect.bottomLeft()));

			// width for border lines
			qreal vw = currentCell.borderWidth(TableCell::LeftBorder);
			if( c > 0)
				vw = std::max(vw, m_cells[c - 1][r].borderWidth(TableCell::RightBorder));

			int mindex = mergeIndex(c, r);
			if( mindex != -1) {
				if( c !=  m_mergedCells[mindex].firstCol)
					vw = 0;
			}

			m_VLines[c][r].m_width = vw;
		}
	}

	// right border
	for( unsigned int r=0; r<m_rows; ++r) {
		const TableCell& currentCell = m_cells[m_cols-1][r];
		const QRectF& crect = currentCell.cellRect();
		m_VLines[m_cols][r] = LineProperties(QLineF(crect.topRight(), crect.bottomRight()));
		m_VLines[m_cols][r].m_width = currentCell.borderWidth(TableCell::RightBorder);
	}
}

void Table::setCellRectsMerged() {
	// Create rects for merge areas
	for( int i=0, count = m_mergedCells.size(); i<count; ++i) {
		MergedCells& mc = m_mergedCells[i];

		qreal width(0);
		for( unsigned int c=mc.firstCol; c<=mc.lastCol; ++c)
			width += m_columnWidths[c].m_size;

		qreal height(0);
		for( unsigned int r=mc.firstRow; r<=mc.lastRow; ++r)
			height += m_rowHeights[r].m_size;

		qreal left = m_cells[mc.firstCol][mc.firstRow].cellRect().left();
		qreal top = m_cells[mc.firstCol][mc.firstRow].cellRect().top();
		QRectF crect(left, top, width, height);
		mc.m_cellRect = crect;
	}
}

void Table::setCellMaximumTextRectsMerged() {
	// Create rects for merge areas
	for( int i=0, count = m_mergedCells.size(); i<count; ++i) {
		MergedCells& mc = m_mergedCells[i];
		const QRectF& crect = mc.m_cellRect;

		qreal leftMargin =  m_cells[mc.firstCol][mc.firstRow].leftMargin();
		qreal rightMargin = m_cells[mc.lastCol][mc.firstRow].rightMargin();
		qreal topMargin =  m_cells[mc.firstCol][mc.firstRow].topMargin();
		qreal bottomMargin = m_cells[mc.firstCol][mc.lastRow].bottomMargin();

		qreal mtleft = crect.x() + (m_spacing + leftMargin) * m_scale;
		qreal mtwidth = crect.width() - (2 * m_spacing + leftMargin + rightMargin) * m_scale;
		qreal mttop = crect.y() + m_spacing * m_scale;
		qreal mtheight = crect.height() - (2 * m_spacing + topMargin + bottomMargin) * m_scale;
		mc.m_maxTextRect = QRectF( mtleft, mttop, mtwidth, mtheight);
	}
}


void  Table::setCellTextRects() {
	for( unsigned int c=0; c<m_cols; ++c) {
		for( unsigned int r=0; r<m_rows; ++r) {
			TableCell& currentCell = m_cells[c][r];
			if( !currentCell.merged()) {
				const QRectF& maxTextRect = currentCell.maxTextRect();

				QSizeF textSize;
				if(currentCell.verticalText())
					textSize = currentCell.textSize(maxTextRect.height(), m_adaptive);
				else
					textSize = currentCell.textSize(maxTextRect.width(), m_adaptive);
				QRectF trect(maxTextRect.topLeft(), textSize);
//				if(currentCell.verticalText())
//					trect = trect.transposed();
				currentCell.setTextRect(trect);
			}
		}
	}
}

void Table::setCellTextRectsMerged() {
	// Create text rects for merge areas
	for( int i=0, count = m_mergedCells.size(); i<count; ++i) {
		MergedCells& mc = m_mergedCells[i];
		const QRectF& maxTextRect = mc.m_maxTextRect;

		QSizeF textSize;
		TableCell& leftTopCell = m_cells[mc.firstCol][mc.firstRow];
		if( leftTopCell.verticalText())
			textSize = m_cells[mc.firstCol][mc.firstRow].textSize(maxTextRect.height(), m_adaptive);
		else
			textSize = m_cells[mc.firstCol][mc.firstRow].textSize(maxTextRect.width(), m_adaptive);
		QRectF trect(maxTextRect.topLeft(), textSize);

		mc.m_textRect = trect;
	}
}


void Table::adjust(QPaintDevice* paintDevice) {
	// performance feature - if nothing is changed no adjustment necessary
	if( m_adjusted)
		return;

	m_textDocument->documentLayout()->setPaintDevice(paintDevice);
	m_scale = paintDevice->logicalDpiX() / QApplication::desktop()->screen()->logicalDpiX();

	setPaintProperties();


	// Take text rectangles for all cells without width constraint and
	// creates a list of maximum cell width of each column
	QVector<qreal> maxCellWidthsUnmerged;
	calcMaxCellWidthsUnmerged(maxCellWidthsUnmerged);

	// Take text rectangles for merged cells and
	// adjust column widths in order to fit the text in merged cells
	calcCellRectsMergedFromText(maxCellWidthsUnmerged);

	// calculates the minimum table width
	qreal tablewidth = m_size.width() - 2 * m_margin * m_scale;
	// check if size is set correctly
	Q_ASSERT(tablewidth > 0);

	qreal mwTotal = std::accumulate(maxCellWidthsUnmerged.begin(), maxCellWidthsUnmerged.end(), 0.0);

	// compare minimum table width with current table width
	// adjust column widths if necessary
	if( tablewidth >= mwTotal) {
		// we have enough space
		// increase column widths in order to fit total size
		calcColumnWidthsExpanding(tablewidth, mwTotal, maxCellWidthsUnmerged);
	}
	else {
		// we have not enough space
		// shrink some column widths in order to fit total size
		calcColumnWidthsShrinking(tablewidth, mwTotal, maxCellWidthsUnmerged);
	}

	// new calculation of cell widths for merged areas
	calcCellRectsWidthMergedFromColWidth();

	// Take text rectangles for all cells with width constraint and
	// creates a list of maximum cell height of each row
	QVector<qreal> maxCellHeightUnmerged;
	calcMaxCellHeightsUnmerged(maxCellHeightUnmerged);

	// Take text rectangles for merged cells and
	// adjust row heights in order to fit the text in merged cells
	calcCellRectsHeightMergedFromRowHeights(maxCellHeightUnmerged);

	// calculates the minimum table height
	qreal tableheight = m_size.height() - 2 * m_margin * m_scale;
	if(tableheight < 0) {
		// means we have more than enough height
		tableheight = 0;
	}

	qreal mhTotal(0.0);
	for( unsigned int r=0; r<m_rows; ++r)
		mhTotal += maxCellHeightUnmerged[r];


	// compare minimum table height with current table height
	// adjust table height if necessary
	// tableheight == 0 means no minimum exist

	// we have enough space
	///< \todo implement method in case of tables higher than page height
	calcRowHeightsExpanding(tableheight, mhTotal, maxCellHeightUnmerged);

	// calculate current total table height
	mhTotal = 0.0;
	for( unsigned int r=0; r<m_rows; ++r) {
		mhTotal += m_rowHeights[r].m_size;
	}
	m_size.setHeight(mhTotal + 2 * m_margin * m_scale);
	// set all cell rects
	setCellRects();

	// set text rect inseide cell rect
	setCellMaximumTextRects();

	calcHorizontalLines();

	calcVerticalLines();

	// Create rects for merge areas
	setCellRectsMerged();
	setCellMaximumTextRectsMerged();

	// create text rects for non merged cells
	setCellTextRects();
	setCellTextRectsMerged();

	m_adjusted = true;
}

QRectF Table::textRect(const QString& html, qreal width) const {
	// No text - no rect necessary
	if( html.isEmpty())
		return QRectF();

	m_textDocument->setTextWidth(width);
	m_textDocument->setDocumentMargin(0);
	m_textDocument->setHtml(html);
	qreal maxw = m_textDocument->idealWidth();		// forces creating of layout and text lines
	QSizeF ds = m_textDocument->documentLayout()->documentSize();
	QRectF rect(0, 0, maxw, ds.height());
	return rect;
}

void Table::paintCellText(unsigned int col, unsigned int row, QPainter* painter, const QPointF& pos) {
	Q_ASSERT(col < m_cols);
	Q_ASSERT(row < m_rows);

	// takes cell and text rect
	const TableCell& currentCell = m_cells[col][row];
	QString text = currentCell.text();
	// No text - nothing to paint
	if( text.isEmpty())
		return;

	QRectF cellRect = currentCell.cellRect();
	// text rect depends on vertical setting
	QRectF textRect =  currentCell.textRect();
	// max rect is always horizontal
	QRectF maxTextRect =  currentCell.maxTextRect();
	int mindex = mergeIndex(col, row);
	// if cell is part of merging area take cell and text rect from this
	if( mindex != -1) {
		MergedCells& mc = m_mergedCells[mindex];
		cellRect = mc.m_cellRect;
		textRect = mc.m_textRect;
		maxTextRect =  mc.m_maxTextRect;
	}

	// calculate rect and positions
	qreal xpos(maxTextRect.left());
	qreal xposRight = maxTextRect.right();
	qreal ypos(maxTextRect.top());

	qreal maxWidth = maxTextRect.width();
	qreal maxHeight = maxTextRect.height();

	qreal textWidth = textRect.width();
	qreal textHeight = textRect.height();
	bool tooLarge;
	if(currentCell.verticalText())
		tooLarge = textHeight > cellRect.width();
	else
		tooLarge = textWidth > cellRect.width();

	Qt::Alignment align = currentCell.alignment();

	if(currentCell.verticalText()) {
		QRectF mrt = maxTextRect.transposed();
		ypos += textHeight; // now top aligned

		if( align.testFlag(Qt::AlignHCenter) || align.testFlag(Qt::AlignCenter))
			xpos = xpos + (mrt.width() - textWidth) / 2.0;
		if( align.testFlag(Qt::AlignRight)) {
			if(!tooLarge)
				xpos = xposRight - textWidth;
		}
		if( align.testFlag(Qt::AlignBottom))
			ypos = mrt.bottom();

		if( align.testFlag(Qt::AlignVCenter))
			ypos = (mrt.height() - textHeight) / 2.0 + ypos;

	}
	else {
		if( align.testFlag(Qt::AlignHCenter) || align.testFlag(Qt::AlignCenter))
			xpos = xpos + (maxWidth - textWidth) / 2.0;
		if( align.testFlag(Qt::AlignRight)) {
			if(!tooLarge)
				xpos = xposRight - textWidth;
		}
		if( align.testFlag(Qt::AlignBottom))
			ypos = maxTextRect.bottom() - textHeight;
		if( align.testFlag(Qt::AlignVCenter))
			ypos = (maxTextRect.height() - textHeight) / 2.0 + maxTextRect.top();
	}

	// calculation of offset
	qreal voffset = currentCell.verticalOffset() * textHeight;
	ypos += voffset;

	// set document properties
	if(currentCell.verticalText())
		m_textDocument->setTextWidth(textHeight);
	else
		m_textDocument->setTextWidth(textWidth);
	m_textDocument->setDocumentMargin(0);
	text = "<meta charset=\"utf-8\"/>" + text;
	m_textDocument->setHtml(text);
//	m_textDocument->setDefaultStyleSheet("h1 {font-family:Times; font-weight:normal; font-size: large;}");
	m_textDocument->idealWidth();		// forces creating of layout and text lines

	painter->save();
	painter->translate(QPointF(xpos, ypos) + pos);
	if(currentCell.verticalText())
		painter->rotate(-90);
	QTextOption option = m_textDocument->defaultTextOption();
	QTextOption optionOrg = option;
	option.setAlignment(align);
	m_textDocument->setDefaultTextOption(option);
	if(tooLarge) {
		QRectF rect = maxTextRect.translated(-maxTextRect.x(), -maxTextRect.y());
		if(currentCell.verticalText())
			rect = rect.transposed();
		m_textDocument->drawContents(painter, rect);
	}
	else
		m_textDocument->drawContents(painter);
	m_textDocument->setDefaultTextOption(optionOrg);
	painter->restore();
}

void Table::paintCellRects(QPainter* painter, const QPointF& pos) {
	// set lines and pen
	painter->save();
	for( unsigned int c=0; c<m_cols; ++c) {
		for( unsigned int r=0; r<m_rows; ++r) {
			const TableCell& currentCell = m_cells[c][r];
			// paint cell background
			painter->fillRect(currentCell.cellRect().translated(pos), currentCell.backgroundColor());
		}
	}

	QPen pen = painter->pen();
	pen.setJoinStyle(Qt::MiterJoin);

	// draw horizontal lines
	for( int c=0; c<m_HLines.size(); ++c) {
		for( int r=0; r<m_HLines[c].size(); ++r) {
			qreal w = m_HLines[c][r].m_width * m_scale;
			if( w > 0) {
				pen.setWidthF(w);
				painter->setPen(pen);
				painter->drawLine(m_HLines[c][r].m_line.translated(pos));
			}
		}
	}

	// draw vertical lines
	for( int c=0; c<m_VLines.size(); ++c) {
		for( int r=0; r<m_VLines[c].size(); ++r) {
			qreal w = m_VLines[c][r].m_width * m_scale;
			if( w > 0) {
				pen.setWidthF(w);
				painter->setPen(pen);
				painter->drawLine(m_VLines[c][r].m_line.translated(pos));
			}
		}
	}
	painter->restore();
}

QRectF Table::tableRect(QPaintDevice* paintDevice, qreal width) {
	QRectF result;
	if( !paintDevice)
		return result;
	if(m_VLines.empty() || m_VLines.back().empty())
		return result;

	if( width <= 0)
		width = paintDevice->width();
	m_size = QSize((int)width, -1);

	if( !m_adjusted) {
		adjust(paintDevice);
	}

	qreal localeWidth = m_VLines.back().front().m_line.x1() - m_VLines.front().front().m_line.x1();
	qreal localeHeight = m_VLines.front().back().m_line.y2() - m_VLines.front().front().m_line.y1();
	result.setSize(QSizeF(localeWidth, localeHeight));
	return result;
}

QSizeF Table::tableSize() const {
	if(m_VLines.empty() || m_VLines.back().empty())
		return QSizeF();

	qreal localeWidth = m_VLines.back().front().m_line.x1() - m_VLines.front().front().m_line.x1();
	qreal localeHeight = m_VLines.front().back().m_line.y2() - m_VLines.front().front().m_line.y1();
	return QSizeF(localeWidth, localeHeight);
}

void Table::drawTable(QPainter* painter, const QPointF& pos) {
	// makes no sense to draw an empty table
	if( m_rows <= 0 || m_cols <= 0)
		return;

	Q_ASSERT(painter != nullptr);
	QPaintDevice* device = painter->device();
	Q_ASSERT(device != nullptr);

	painter->fillRect(QRectF(pos, m_size), m_background);

	if( m_size.width() <= 0)
		m_size = QSize(device->width(), -1);


	adjust(device);
	paintCellRects(painter, pos);
	for( unsigned int j=0; j<m_rows; ++j){
		for( unsigned int i=0; i<m_cols; ++i) {
			paintCellText(i, j, painter, pos);
		}
	}
	if( m_outerFrameWidth > 0) {
		QPen orgpen = painter->pen();
		QPen pen = painter->pen();
		pen.setWidthF(m_outerFrameWidth * m_scale);
		painter->setPen(pen);
		QPointF topleft = m_cells[0][0].cellRect().topLeft();
		QPointF bottomright = m_cells[m_cols - 1][m_rows - 1].cellRect().bottomRight();
		painter->drawRect(QRectF(topleft, bottomright).translated(pos));
		painter->setPen(orgpen);
	}
}

std::vector<unsigned int> Table::fittingTableRows(QPaintDevice* paintDevice, qreal hfirst, qreal hrest) const {
	// makes no sense to draw an empty table
	if( m_rows <= 0 || m_cols <= 0)
		return std::vector<unsigned int>(1, 0);

	QSizeF size = tableSize();

	if(size.height() <= hfirst)
		return std::vector<unsigned int>(1, m_rows);

	qreal headerHeight = 0;
	for(unsigned int i=0; i<m_headerRows; ++i) {
		headerHeight += m_cells[0][i].cellRect().height();
	}
	headerHeight += m_outerFrameWidth;

	std::vector<unsigned int> res;
	qreal dividedTableHeight = headerHeight;
	for(unsigned int i = m_headerRows; i<m_rows; ++i) {
		qreal currentHeight = m_cells[0][i].cellRect().height();
		dividedTableHeight += currentHeight;
		if(res.size() == 0 && (dividedTableHeight + m_outerFrameWidth) >= hfirst) {
			res.push_back(i-1);
			dividedTableHeight = headerHeight + currentHeight;
		}
		else if(res.size() > 0 && (dividedTableHeight + m_outerFrameWidth) >= hrest) {
			res.push_back(i-1);
			dividedTableHeight = headerHeight + currentHeight;
		}
	}
	res.push_back(m_rows-1);
	return res;
}

std::vector<Table*> Table::fittingTables(QPaintDevice* paintDevice, qreal hfirst, qreal hrest) {
	std::vector<unsigned int> tableRows = fittingTableRows(paintDevice, hfirst, hrest);
	std::vector<Table*> res;
	if(tableRows.size() == 1) {
		res.push_back(this);
		return res;
	}

	int startRow = 0;
	for(int i=0; i<tableRows.size(); ++i) {
		int endRow = tableRows[i];
		res.push_back(createSubTable(startRow, endRow));
		Q_ASSERT(res.back() != nullptr);
		res.back()->adjust(paintDevice);
		startRow = endRow + 1;
	}

	return res;
}


void Table::frameRect(Table& table, int cLeft, int cRight, int rTop, int rBottom, int lineWidth) {
	for( int i=rTop; i<=rBottom; ++i) {
		table.cell(cLeft, i).setBorderWidth(QtExt::TableCell::LeftBorder, lineWidth);
		table.cell(cRight, i).setBorderWidth(QtExt::TableCell::RightBorder, lineWidth);
	}
	for( int i=cLeft; i<=cRight; ++i) {
		table.cell(i, rTop).setBorderWidth(QtExt::TableCell::TopBorder, lineWidth);
		table.cell(i, rBottom).setBorderWidth(QtExt::TableCell::BottomBorder, lineWidth);
	}
}

Table* Table::createSubTable(unsigned int startRow, unsigned int endRow) {
	if( startRow > endRow || startRow >= m_rows || endRow >= m_rows)
		return nullptr;

	Table* table = new Table(m_textDocument, m_adaptive, m_size, parent());
	unsigned int rowCount = endRow-startRow+1;
	int usedHeaderRows = 0;
	if(startRow <= m_headerRows-1) {
		usedHeaderRows = (startRow - m_headerRows) * -1;
		rowCount += m_headerRows - usedHeaderRows;
	}
	rowCount += m_headerRows - usedHeaderRows;
	table->setColumnsRows(m_cols, rowCount, m_headerRows);

	table->m_textDocumentOwner = false;
	table->m_spacing = m_spacing;
	table->m_margin = m_margin;
	table->m_outerFrameWidth = m_outerFrameWidth;
	table->m_innerFrameWidth = m_innerFrameWidth;
	table->m_background = m_background;
	table->m_scale = m_scale;
	table->m_adjusted = false;
	table->m_columnWidths = m_columnWidths;

	for(unsigned int ci=0; ci<table->m_cols; ++ci) {
//		if(m_headerRows > 0)
//			table->m_HLines[ci][0] = m_HLines[ci][0];
		for(unsigned int ri=0; ri<m_headerRows; ++ri) {
			table->m_rowHeights[ri] = m_rowHeights[ri];
			table->m_cells[ci][ri] = m_cells[ci][ri];
//			table->m_VLines[ci][ri] = m_VLines[ci][ri];
//			table->m_HLines[ci][ri+1] = m_HLines[ci][ri+1];
		}
		unsigned int realStartRow = startRow;
		if(startRow <= m_headerRows -1) {
			realStartRow += usedHeaderRows;
		}
		unsigned int newTableRow = m_headerRows;
		for(unsigned int ri=realStartRow; ri<=endRow; ++ri) {
			table->m_rowHeights[newTableRow] = m_rowHeights[ri];
			table->m_cells[ci][newTableRow] = m_cells[ci][ri];
//			table->m_VLines[ci][ri] = m_VLines[ci][ri];
			++newTableRow;
		}
	}
	return table;
}

} // namespace QtExt
