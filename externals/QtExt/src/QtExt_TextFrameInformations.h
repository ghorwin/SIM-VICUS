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

#ifndef QtExt_TextFrameInformationsH
#define QtExt_TextFrameInformationsH

#include <vector>

#include <QtGlobal>
#include <QVector>
#include <QFont>
#include <QSizeF>
#include <QRectF>

class QPaintDevice;
class QTextDocument;

namespace QtExt {

/*! \brief Class contains a vector of possible text sizes for different line counts.
	The vector is only valid for the specified paintDevice.
*/
class TextFrameInformations
{
public:
	class TextFrameInfo {
	public:

		TextFrameInfo() :
			m_lineCount(0),
			m_height(0),
			m_minWidth(0),
			m_maxWidth(0),
			m_valid(false)
		{}

		TextFrameInfo(unsigned int lineCount, qreal height, qreal minWidth, qreal maxWidth) :
			m_lineCount(lineCount),
			m_height(height),
			m_minWidth(minWidth),
			m_maxWidth(maxWidth),
			m_valid(lineCount > 0)
		{}

		TextFrameInfo(unsigned int lineCount, const QSizeF& size, qreal maxWidth) :
			m_lineCount(lineCount),
			m_height(size.height()),
			m_minWidth(size.width()),
			m_maxWidth(maxWidth),
			m_valid(maxWidth > 0)
		{}

		bool isValid() const {
			return m_valid;
		}

		QSizeF size() const {
			return QSizeF(m_minWidth, m_height);
		}

		static TextFrameInfo nonValid(unsigned int lineCount) {
			TextFrameInfo ret(lineCount, 0, 0, 0);
			ret.m_valid = false;
			return ret;
		}

		unsigned int	m_lineCount;
		qreal			m_height;
		qreal			m_minWidth;
		qreal			m_maxWidth;

	private:
		bool			m_valid;
	};

	/*! Default constructor.
		This constructor creates a object without informations.
		Call set to fill the object.
	*/
	TextFrameInformations();

	/*! Constructor with necessary informations to create content.*/
	TextFrameInformations(QTextDocument* textDocument, const QString& text, bool adaptive);

	~TextFrameInformations();

	/*! Copy constructor.*/
	TextFrameInformations(const TextFrameInformations&);

	/*! Copy assignment operator.*/
	TextFrameInformations& operator=(TextFrameInformations);

	/*! Specialized swap function. Used in copy assignment operator.*/
	void Swap(TextFrameInformations&);

	/*! Set new content for the given text.
		\param textDocument Text document for rendering text.
		\param text Text for calculation. HTML format is possible.
		\param lineEnd up to how many lines should go the calculation. 0 or 1 means only optimal width.
		If the text and paintDevice isn't changed since the last call calculation will take place only if the lineEnd is bigger than the already existing.
	*/
	void set(QTextDocument* textDocument, const QString& text, bool adaptive);

	/*! Gets the text size for a given maximum text width.
		If the text size for the given width isn't already calculated a new calculation with the last parameters will occure.
	*/
	TextFrameInfo sizeForMaximumWidth(qreal maxWidth, bool adaptive);

	/*! Returns true if no information exists.*/
	bool empty() const { return !m_infoVect.empty(); }

	static int m_pixelStep;

private:
	QTextDocument*				m_textDocument;	///< Internal text document.
	std::vector<TextFrameInfo>	m_infoVect;		///< Vector for text sizes for different line counts.
	QString						m_text;			///< Last used text;

	/*! Has the same functionality like set but uses the internal variables.*/
	void setInternal(bool adaptive);
};

} // namespace QtExt

#endif // QtExt_TextFrameInformationsH
