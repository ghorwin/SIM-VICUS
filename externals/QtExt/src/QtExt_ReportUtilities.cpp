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

#include "QtExt_ReportUtilities.h"

namespace QtExt {

QString htmlCaptionFormat(const QString& text, FormatTags level) {
	switch(level) {
		case FT_Small: return QString("<small>%1</small>").arg(text);
		case FT_Normal: return text;
		case FT_Header1: return QString("<h1>%1</h1>").arg(text);
		case FT_Header2: return QString("<h2>%1</h2>").arg(text);
		case FT_Header3: return QString("<h3>%1</h3>").arg(text);
		case FT_Header4: return QString("<h4>%1</h4>").arg(text);
		default: return text;
	}
}

FormatTags smallerFormatTag(FormatTags org) {
	switch(org) {
		case FT_Header1: return FT_Header2;
		case FT_Header2: return FT_Header3;
		case FT_Header3: return FT_Header4;
		case FT_Header4: return FT_Normal;
		case FT_Normal: return FT_Small;
		case FT_Small: return FT_Small;
		default: return FT_Normal;
	}
}

FormatTags biggerFormatTag(FormatTags org) {
	switch(org) {
		case FT_Header1: return FT_Header1;
		case FT_Header2: return FT_Header1;
		case FT_Header3: return FT_Header2;
		case FT_Header4: return FT_Header3;
		case FT_Normal: return FT_Header4;
		case FT_Small: return FT_Normal;
		default: return FT_Normal;
	}
}


} // namespace QtExt
