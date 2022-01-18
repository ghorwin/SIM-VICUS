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

#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QColorDialog>

#include "QtExt_ColorEditWidget.h"

namespace QtExt {


ColorEditWidget::ColorEditWidget(QWidget * parent) :
	QWidget(parent)
{
	setupUi();
}

ColorEditWidget::ColorEditWidget(const QString & label, QWidget * parent) :
	QWidget(parent)
{
	setupUi();
	m_label->setText(label);
}

void ColorEditWidget::setupUi() {
	m_label = new QLabel(this);

	m_colorWidget = new QFrame(this);
	m_colorWidget->setFrameShape(QFrame::Box);
	m_colorWidget->setMinimumSize(48,24);
	m_colorWidget->setMaximumSize(48,24);
	m_colorWidget->setAutoFillBackground(true);

	m_pushButton = new QPushButton(this);
	m_pushButton->setText(tr("..."));
	m_pushButton->setMaximumWidth(48);

	connect(m_pushButton, SIGNAL(clicked()), this, SLOT(onSelectColor()));

	setMaximumHeight(m_pushButton->height());
	QHBoxLayout * lay = new QHBoxLayout;
	lay->addWidget(m_label);
	lay->addWidget(m_colorWidget);
	lay->addWidget(m_pushButton);
	lay->setContentsMargins(0,0,0,0);
	setLayout(lay);
}

void ColorEditWidget::setLabel(const QString & label) {
	m_label->setText(label);
}

void ColorEditWidget::setColor(const QColor & color) {
	QPalette pal = m_colorWidget->palette();
	pal.setColor(QPalette::Background, color);
	m_colorWidget->setPalette(pal);
}

QColor ColorEditWidget::color() const {
	return m_colorWidget->palette().color(QPalette::Background);
}

void ColorEditWidget::onSelectColor() {
	QColorDialog dlg(color(), this);
	if (dlg.exec() == QDialog::Accepted) {
		setColor(dlg.selectedColor());
		emit colorSelected(dlg.selectedColor());
	}
}

} // namespace QtExt
