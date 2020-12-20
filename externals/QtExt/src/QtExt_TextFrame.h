/*	Authors: H. Fechner, A. Nicolai

	This file is part of the QtExt Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

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
