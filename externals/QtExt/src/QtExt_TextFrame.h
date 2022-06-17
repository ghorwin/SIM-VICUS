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

#ifndef QtExt_TextFrameH
#define QtExt_TextFrameH

#include <vector>

#include <QObject>
#include <QRectF>
#include <QFont>
#include <QTextDocument>
#include <QColor>

class QPaintDevice;

namespace QtExt {

/*! \todo Heiko: Document this class!!! */
class TextFrame : public QObject {
Q_OBJECT
public:

	/*! Constructor for use of shared textDocuments.
		\param textDocument QTextDocument pointer.
		\param parent Parent object is responsible for delete.
		If textDocument is 0 a new textDocument will be created and the instance of this class has ownership.
	*/
	TextFrame(QTextDocument* textDocument, QObject *parent = 0);
	/*! Destructor.
		Deletes m_textDocument if necessary.
	*/
	~TextFrame();
	/*! Returns the table rectangle.
		\param Current painter.
		\param width Preferred width of the table. If 0 width of paintDevice will be used.
	*/
	QRectF frameRect(QPaintDevice* paintDevice, qreal width = 0);
	/*! Sets the text. It can be a normal tect or HTML-formated text.
		\param text Simple text or HTML sequence.
	*/
	void setText(const QString& text);
	/*! Returns the internal text.
		This includes all HTML format sentences if exist.
	*/
	QString text() const;
	/*! Sets a default font.*/
	void setDefaultFont(const QFont& font);
	/*! Sets the default style sheet (css).*/
	void setDefaultStyleSheet ( const QString & sheet );
	/*! Sets the alignment of the text in the cell.
		\param alignment One or a valid combination of alignments.
	*/
	void setAlignment(Qt::Alignment alignment);
	/*! Returns the text alignment.*/
	Qt::Alignment alignment() const;
	/*! Returns the left margin.*/
	qreal leftMargin() const;
	/*! Sets the left margin.*/
	void setLeftMargin(qreal margin);
	/*! Returns the right margin.*/
	qreal rightMargin() const;
	/*! Sets the right margin.*/
	void setRightMargin(qreal margin);

	/*! Draw the whole frame with the given painter.
		\param painter Painter used for drawing.
		\param pos Left/Top corner.
	*/
	void drawFrame(QPainter* painter, const QPointF& pos, qreal width);
	/*! Returns the background color of the frame.*/
	QColor	backgroundColor() const;
	/*! sets the background color of the frame.*/
	void	setBackgroundColor(const QColor& col);
	/*! Returns the font color of the text in the frame.*/
	QColor	fontColor() const;
	/*! Sets the font color of the text in the frame.*/
	void	setFontColor(const QColor& col);

	/*! Clone the current text frame.*/
	TextFrame* clone();

signals:
	/*! \todo check usage of signal -> when and where should the signal be emitted. */
	void changed();

public slots:

private:
	QTextDocument*	m_textDocument;			///< Internal document for formating text.
	bool			m_textDocumentOwner;	///< True if the class instance is owner of the text document.
	qreal			m_leftMargin;			///< Width of the left border.
	qreal			m_rightMargin;			///< Width of the right border.
	Qt::Alignment	m_alignment;			///< Text alignment.
	QString			m_text;					///< Cell text.
	QColor			m_backgroundColor;		///< Color for frame background (default is transparent).
	QColor			m_fontColor;			///< Color for text/font (default is black).

	/*! Returns the surrounding text rectangle for the given text.
		\param text Text string.
		\param width Maximum text width.
	*/
	QRectF textRect(const QString& html, qreal width) const;
};

} // namespace QtExt

#endif // QtExt_TextFrameH
