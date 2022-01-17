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

#include "QtExt_CoordinateIndexEdit.h"
#include "ui_QtExt_CoordinateIndexEdit.h"

#include <algorithm>

#include <IBK_Exception.h>
#include <IBK_FormatString.h>

namespace QtExt {

CoordinateIndexEdit::CoordinateIndexEdit(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::CoordinateIndexEdit),
	m_ascending(true),
	m_maxCoordinateIndex(-1),
	m_minCoordinateIndex(-1)
{
	ui->setupUi(this);
	connect(ui->lineEdit, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
	connect(ui->spinBox, SIGNAL(valueChanged(int)), this, SLOT(onIndexChanged(int)));
}

CoordinateIndexEdit::~CoordinateIndexEdit()
{
	delete ui;
}

void CoordinateIndexEdit::changeEvent(QEvent *e)
{
	QWidget::changeEvent(e);
	switch (e->type()) {
		case QEvent::LanguageChange:
			ui->retranslateUi(this);
			break;
		default:
			break;
	}
}

void CoordinateIndexEdit::set(const QVector<double>& coordinates) {
	m_coordinates = coordinates;
	m_maxCoordinateIndex = -1;
	m_minCoordinateIndex = -1;
	m_ascending = true;
	if(m_coordinates.empty()) {
		setDisabled(true);
		return;
	}
	ui->spinBox->setRange(0, m_coordinates.size()-1);
	if(m_coordinates.size() == 1) {
		m_maxCoordinateIndex = 0;
		m_minCoordinateIndex = 0;
	}
	else {
		m_ascending = m_coordinates[0] < m_coordinates[1];
		// check sorting
		for(QVector<double>::size_type i=2; i<m_coordinates.size(); ++i) {
			if(m_ascending) {
				if(m_coordinates[i-1] >= m_coordinates[i])
					throw IBK::Exception("Coordinate vector not sorted", "[CoordinateIndexEdit::set]");
			}
			else {
				if(m_coordinates[i-1] >= m_coordinates[i])
					throw IBK::Exception("Coordinate vector not sorted", "[CoordinateIndexEdit::set]");
			}
		}
		m_maxCoordinateIndex = m_ascending ? m_coordinates.size()-1 : 0;
		m_minCoordinateIndex = m_ascending ? 0 : m_coordinates.size()-1;
	}
}

bool CoordinateIndexEdit::validCoordinates() const {
	if(m_coordinates.empty()) {
		return false;
	}
	if(m_maxCoordinateIndex == -1 || m_minCoordinateIndex == -1)
		return false;

	return true;
}

bool CoordinateIndexEdit::isValid() const {
	if(!validCoordinates())
		return false;

	return ui->lineEdit->isValid();
}


void CoordinateIndexEdit::setValidator(ValidatorBase* validator) {
	ui->lineEdit->setValidator(validator);
}

bool CoordinateIndexEdit::isValidNumber(double & val) const {
	bool res = ui->lineEdit->isValidNumber(val);
	if(!res)
		return false;

	if(validCoordinates()) {
		if(val > m_coordinates[m_maxCoordinateIndex])
			return false;
		if(val < m_coordinates[m_minCoordinateIndex])
			return false;
	}

	return true;
}

void CoordinateIndexEdit::setEnabled(bool enabled) {
	ui->lineEdit->setEnabled(enabled);
	ui->spinBox->setEnabled(enabled);
}


void CoordinateIndexEdit::setReadOnly(bool readOnly) {
	ui->lineEdit->setReadOnly(readOnly);
	ui->spinBox->setReadOnly(readOnly);
}

double CoordinateIndexEdit::value() const {
	int index = ui->spinBox->value();
	return m_coordinates[index];
}

void CoordinateIndexEdit::setValue(double value) {
	int index = -1;
	// do nothing in case of empty vector
	if(m_coordinates.size() <= 1)
		return;

	// now find index of closest element
	if(m_coordinates.size() == 2) {
		double diff1 = std::abs(m_coordinates[0] - value);
		double diff2 = std::abs(value - m_coordinates[1]);
		index = (diff2 < diff1) ? 1 : 0;
	}
	else {
		if(m_ascending) {
			auto it = std::lower_bound(m_coordinates.begin(), m_coordinates.end(), value);
			// all values in vector are smaller
			if(it == m_coordinates.end())
				index = m_coordinates.size() - 1;
			else {
				index = it - m_coordinates.begin();
			}
			if(index > 0) {
				double diff1 = m_coordinates[index] - value;
				double diff2 = value - m_coordinates[index-1];
				if(diff2 < diff1)
					index = index - 1;
			}
		}
		else {
			auto it = std::lower_bound(m_coordinates.rbegin(), m_coordinates.rend(), value);
			// all values in vector are smaller
			if(it == m_coordinates.rend())
				index = 0;
			else {
				index = m_coordinates.size() - 1 - (it - m_coordinates.rbegin());
			}
			if(index < m_coordinates.size() - 1) {
				double diff1 = m_coordinates[index] - value;
				double diff2 = value - m_coordinates[index+1];
				if(diff2 < diff1)
					index = index + 1;
			}
		}
	}
	setIndex(index);
}

int CoordinateIndexEdit::index() const {
	return ui->spinBox->value();
}

void CoordinateIndexEdit::setIndex(unsigned int index) {
	if(m_coordinates.empty())
		return;

	if(index >= (unsigned int)m_coordinates.size())
		index = m_coordinates.size() - 1;

	blockSignals(true);
	ui->spinBox->setValue(index);
	ui->lineEdit->setValue(m_coordinates[index]);
	blockSignals(false);

	emit editingFinishedSuccessfully();
}


void CoordinateIndexEdit::onEditingFinished() {
	if (isValid()) {
		// use setValue function for checking
		setValue(ui->lineEdit->value());
	}
}

void CoordinateIndexEdit::onIndexChanged(int index) {
	blockSignals(true);
	ui->lineEdit->setValue(m_coordinates[index]);
	blockSignals(false);
	emit editingFinishedSuccessfully();
}


} // namespace QtExt
