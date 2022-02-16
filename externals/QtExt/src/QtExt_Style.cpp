/*	QtExt - Qt-based utility classes and functions (extends Qt library)

	Copyright (c) 2014-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Heiko Fechner
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 3 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

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

