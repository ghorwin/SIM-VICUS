/*	Authors: H. Fechner, A. Nicolai

	This file is part of the QtExt Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#include "QtExt_ColorButton.h"

#include <QPainter>
#include <QStyleOptionButton>
#include <QColorDialog>

#if QT_VERSION >= 0x050000
#include <qdrawutil.h>
#endif

namespace QtExt {

ColorButton::ColorButton(QWidget *parent) :
	QPushButton(parent),
	m_color(Qt::red)
{
	connect(this, SIGNAL(clicked()), this, SLOT(onClicked()));
}


void ColorButton::setColor(const QColor & c) {
	m_color = c;
	update();
}


void ColorButton::paintEvent( QPaintEvent* e) {
	// draw default button
	QPushButton::paintEvent(e);

	// now draw the color
	QPainter painter(this);
	QStyle *style = QWidget::style();
	Q_ASSERT(style != nullptr);
	QStyleOptionButton opt;
	opt.initFrom(this);
	opt.state |= isDown() ? QStyle::State_Sunken : QStyle::State_Raised;
	opt.features = QStyleOptionButton::None;
	if (isDefault())
		opt.features |= QStyleOptionButton::DefaultButton;
	opt.text.clear();
	opt.icon = QIcon();


	QRect labelRect = style->subElementRect( QStyle::SE_PushButtonContents, &opt, this );
	int shift = style->pixelMetric( QStyle::PM_ButtonMargin, &opt, this ) / 2;
	labelRect.adjust(shift, shift, -shift, -shift);
	int x, y, w, h;
	labelRect.getRect(&x, &y, &w, &h);

	if (isChecked() || isDown()) {
		x += style->pixelMetric( QStyle::PM_ButtonShiftHorizontal, &opt, this );
		y += style->pixelMetric( QStyle::PM_ButtonShiftVertical, &opt, this );
	}

	QColor fillCol = isEnabled() ? m_color : palette().color(backgroundRole());
	qDrawShadePanel( &painter, x, y, w, h, palette(), true, 1, NULL);
	if ( fillCol.isValid() ) {
		painter.fillRect(QRect( x+1, y+1, w-2, h-2), QBrush(fillCol));
	}
}


void ColorButton::onClicked() {
	QColorDialog dlg(this);

#ifdef Q_OS_WIN
	#define NO_NATIVE_COLOR_DLG
	#ifdef NO_NATIVE_COLOR_DLG
		dlg.setOption(QColorDialog::DontUseNativeDialog, true);
	#endif
#endif

	dlg.setCurrentColor(m_color);
	if (dlg.exec() == QDialog::Accepted) {
		QColor newColor = dlg.currentColor();
		if (newColor != m_color) {
			setColor(dlg.currentColor());
			emit colorChanged();
		}
	}
}

} // namespace QtExt
