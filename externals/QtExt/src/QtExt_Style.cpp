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

#include "QtExt_Style.h"

#include <QVBoxLayout>
//#include <QWidgetItemV2>

#include <QtExt_FeatureWidget.h>

namespace QtExt {

QString Style::EditFieldBackground						= "#f9f6c8";
QString Style::AlternativeEditFieldBackground			= "#f9ffd8";
QString Style::ErrorEditFieldBackground					= "#ff7777";
QString Style::ReadOnlyEditFieldBackground				= "#d6e9ff";

QString Style::AlternativeBackgroundBright				= "#fff4b8";
QString Style::AlternativeBackgroundDark				= "#ffe49d";
QString Style::AlternativeBackgroundText				= "#760000";

QString Style::ToolBoxPageBackground					= "#212124";
QString Style::ToolBoxPageEdge							= "#3a3b3f";

void Style::adjustContentWidget(QWidget * widget) {
	QLayout * lay = dynamic_cast<QLayout *>(widget->layout());
	if (lay != NULL) {
		lay->setSpacing(4);
		lay->setContentsMargins(6,6,6,6);
	}
}

void Style::adjustContentPropertyWidget(QWidget * widget) {
	QLayout * lay = dynamic_cast<QLayout *>(widget->layout());
	if (lay != NULL) {
		lay->setSpacing(4);
		lay->setContentsMargins(0,0,0,0);
	}
}


void Style::adjustFeatureWidget(FeatureWidget * widget) {
	// A feature widget holds a single layout and a content widget
	QVBoxLayout * vlay = dynamic_cast<QVBoxLayout *>(widget->layout());
	Q_ASSERT(vlay != NULL);
	vlay->setSpacing(0);
	vlay->setContentsMargins(0,0,0,0);
	if (vlay->count() > 1) {
		QWidgetItem * witem = dynamic_cast<QWidgetItem *>(vlay->itemAt(1));
		if (witem != NULL && witem->widget() != NULL)
			adjustContentWidget(witem->widget());
	}
}

void Style::adjustFeaturePropertyWidget(FeatureWidget * widget) {
	// A feature widget holds a single layout and a content widget
	QVBoxLayout * vlay = dynamic_cast<QVBoxLayout *>(widget->layout());
	Q_ASSERT(vlay != NULL);
	vlay->setSpacing(0);
	vlay->setContentsMargins(0,0,0,0);
	if (vlay->count() > 1) {
		QWidgetItem * witem = dynamic_cast<QWidgetItem *>(vlay->itemAt(1));
		if (witem != NULL && witem->widget() != NULL)
			adjustContentPropertyWidget(witem->widget());
	}
}



void Style::adjustScrollAreaWithFeatureWidgets(QWidget * widget) {
	QVBoxLayout * vlay = dynamic_cast<QVBoxLayout *>(widget->layout());
	Q_ASSERT(vlay != NULL);
	vlay->setSpacing(1);
	vlay->setContentsMargins(0,0,0,0);
	// loop through all layout items in the layout and if they
	// are convertible to feature widgets, also call the feature widget
	// layout function
	for (int i=0; i<vlay->count(); ++i) {
		QLayoutItem * item = vlay->itemAt(i);
		QWidgetItem * witem = dynamic_cast<QWidgetItem *>(item);
		if (witem == NULL) continue;
		QtExt::FeatureWidget * fw = dynamic_cast<QtExt::FeatureWidget *>(witem->widget());
		if (fw != NULL)
			adjustFeatureWidget(fw);
	}

}

void Style::adjustScrollAreaWithFeaturePropertyWidgets(QWidget * widget) {

	QVBoxLayout * vlay = dynamic_cast<QVBoxLayout *>(widget->layout());
	Q_ASSERT(vlay != NULL);
	vlay->setSpacing(1);
	vlay->setContentsMargins(0,0,0,0);
	// loop through all layout items in the layout and if they
	// are convertible to feature widgets, also call the feature widget
	// layout function
	for (int i=0; i<vlay->count(); ++i) {
		QLayoutItem * item = vlay->itemAt(i);
		QWidgetItem * witem = dynamic_cast<QWidgetItem *>(item);
		if (witem == NULL) continue;
		QtExt::FeatureWidget * fw = dynamic_cast<QtExt::FeatureWidget *>(witem->widget());
		if (fw != NULL)
			adjustFeaturePropertyWidget(fw);
	}

}

} // namespace QtExt

