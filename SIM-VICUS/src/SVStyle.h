/*	SIM-VICUS - Building and District Energy Simulation Tool.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Dirk Weiss  <dirk.weiss -[at]- tu-dresden.de>
	  Stephan Hirth  <stephan.hirth -[at]- tu-dresden.de>
	  Hauke Hirsch  <hauke.hirsch -[at]- tu-dresden.de>

	  ... all the others from the SIM-VICUS team ... :-)

	This program is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

#ifndef SVStyleH
#define SVStyleH

#include "SVSettings.h"

#include <QString>

class QPlainTextEdit;
class QWidget;
class QTableView;
class QTreeView;
class QListView;

/*! Central implementation and configuration of visual appearance of the software.
	This class provides several member functions that can be used to tune the appearance of the software.

	Instantiate exactly one instance of this class right *after* creating the application object but *before*
	creating the first widgets.
*/
class SVStyle {
public:
	SVStyle();

	/*! Returns the instance of the singleton. */
	static SVStyle & instance();

	void formatPlainTextEdit(QPlainTextEdit * textEdit) const;

	static void formatWidgetWithLayout(QWidget * w);

	static void formatDatabaseTableView(QTableView * v);
	static void formatDatabaseTreeView(QTreeView * v);
	static void formatListView(QListView * v);

	/*! Replaces all color text placeholders with colors based on the current style sheet. */
	static void formatWelcomePage(QString & htmlCode);

	/*! Resizes column to contents, yet honors the enlargeOnly flag.
		Useful if user can adjust column width manually, but data changes may require enlarging
		column.
	*/
	static void resizeTableColumnToContents(QTableView * v, int column, bool enlargeOnly);

	/*! Sets the application wide style sheet. */
	void setStyle(SVSettings::ThemeType dark);

	/*! Returns a randomized color.
		Subsequent calls to this function generate sufficiently different colors.
	*/
	static QColor randomColor();

	QString				m_styleSheet;

	QColor				m_alternativeBackgroundDark; // TODO
	QColor				m_alternativeBackgroundBright; // TODO
	QColor				m_alternativeBackgroundText; // TODO
	QColor				m_readOnlyEditFieldBackground;		// TODO : subtitute with QtExt::Style::ReadOnlyEditFieldBackground
	QColor				m_alternativeReadOnlyEditFieldBackground; // TODO
	QColor				m_errorEditFieldBackground; // TODO

	QColor				m_userDBBackgroundBright;
	QColor				m_userDBBackgroundDark;
	QColor				m_notReferencedText;

	QColor				m_logProgressText;
	QColor				m_logErrorText;
	QColor				m_logWarningText;
	QColor				m_logDebugText;

private:
	static SVStyle		*m_self;


	/*! The hue value of the last randomly generated color. */
	static double		m_randomColorHueValue;

	unsigned int		m_fontMonoSpacePointSize;
	QString				m_fontMonoSpace;
};

#endif // SVStyleH
