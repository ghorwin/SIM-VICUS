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

#include <QDebug>

#include "QtExt_MaterialTableProxyModel.h"
#include "QtExt_Style.h"

#include "QtExt_MaterialBase.h"

namespace QtExt {

MaterialTableProxyModel::MaterialTableProxyModel(QObject *parent) :
	QSortFilterProxyModel(parent),
	m_categoryFilter(-1),
	m_showDeprecatedFilter(true),
	m_capabilityFilter(0)
{
}

bool MaterialTableProxyModel::lessThan ( const QModelIndex & left, const QModelIndex & right ) const {
	QVariant leftData = sourceModel()->data(left);
	QVariant rightData = sourceModel()->data(right);

	// set demostration materials with all parameters equal 0 at end of the material table regardless other settings
	MaterialBase* leftMaterial = reinterpret_cast<MaterialBase*>(left.internalPointer());
	MaterialBase* rightMaterial = reinterpret_cast<MaterialBase*>(right.internalPointer());
	Q_ASSERT(leftMaterial != nullptr);
	Q_ASSERT(rightMaterial != nullptr);
	bool demoLeft = leftMaterial->rho() <= 0.1;
	bool demoRight = rightMaterial->rho() <= 0.1;
	if( demoLeft && !demoRight)
		return false;
	if( !demoLeft && demoRight)
		return true;

	int coll = left.column(), colr = right.column();
	MaterialBase::parameter_t para = static_cast<MaterialBase::parameter_t>(sourceModel()->data(left, ParaRole).toInt());
	if( coll == colr && (para > MaterialBase::MC_PRODUCER || para == MaterialBase::MC_ID)) {
		double dl = left.data(Qt::UserRole).toDouble();
		double dr = right.data(Qt::UserRole).toDouble();
		return dl < dr;
	}
	else {
		return QString::compare(leftData.toString(), rightData.toString(), Qt::CaseInsensitive) < 0;
	}
}

QVariant MaterialTableProxyModel::data(const QModelIndex &proxyIndex, int role) const {
//	if (role == Qt::BackgroundRole) {
//		if (proxyIndex.row() % 2 == 0)
//			return QColor(Style::AlternativeBackgroundBright);
//		else
//			return QColor(Style::AlternativeBackgroundDark);
//	}
	return QSortFilterProxyModel::data(proxyIndex, role);
}

static bool searchForString(const QString& str, const QString& searchPattern) {
	if( !searchPattern.isEmpty()) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
		QStringList tokens = searchPattern.split(' ', Qt::SkipEmptyParts);
#else
		QStringList tokens = searchPattern.split(' ', QString::SkipEmptyParts);
#endif
		bool found = false;
		foreach (QString t, tokens) {
			QRegExp regEx(t);
			regEx.setPatternSyntax(QRegExp::Wildcard);
			regEx.setCaseSensitivity(Qt::CaseInsensitive);

			if (regEx.indexIn(str) != -1) {
				found = true;
				break;
			}
		}
		if(!found)
			return false;
	}
	return true;
}

bool MaterialTableProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const {
	bool accepted = true;
	QModelIndex sindex = sourceModel()->index(sourceRow, 0, sourceParent);
	MaterialBase* mat = static_cast<MaterialBase*>(sindex.internalPointer());
	if( mat == nullptr || (m_categoryFilter == -1 && m_nameFilter.isEmpty() && m_producerFilter.isEmpty() &&
						   m_sourceFilter.isEmpty() && m_commentFilter.isEmpty()))
		return true;

	if( m_categoryFilter > -1 && m_categoryFilter < MaterialCategory::NUM_CATEGORIES) {
		if(m_categoryFilter != mat->category().idx)
			accepted = false;
	}

	if( !m_nameFilter.isEmpty()) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
		QStringList tokens = m_nameFilter.split(' ', Qt::SkipEmptyParts);
#else
		QStringList tokens = m_nameFilter.split(' ', QString::SkipEmptyParts);
#endif
		bool found = false;
		foreach (QString t, tokens) {
			QRegExp regEx(t);
			regEx.setPatternSyntax(QRegExp::Wildcard);
			regEx.setCaseSensitivity(Qt::CaseInsensitive);

			if (regEx.indexIn(mat->name()) != -1 || regEx.indexIn(mat->productName()) != -1) {
				found = true;
				break;
			}
		}
		if(!found)
			accepted = false;
	}

	if(!searchForString(mat->producer(), m_producerFilter))
		accepted = false;

	if(!searchForString(mat->dataSource(), m_sourceFilter))
		accepted = false;

	if(!searchForString(mat->notes(), m_commentFilter))
		accepted = false;

	// capabilitis
	if(m_capabilityFilter > 0) {
		int cap = mat->capabilities();
		if((cap & m_capabilityFilter) != m_capabilityFilter)
			accepted = false;
	}

	// show deprecated
	if(!m_showDeprecatedFilter) {
		if(mat->deprecatedState() == -1 || mat->deprecatedState() > 0) {
			accepted = false; // filtered out
		}
	}

	return accepted;
}

void MaterialTableProxyModel::setCategoryFilter(int val) {
	m_categoryFilter = val;
	invalidate();
	emit layoutChanged();
}

void MaterialTableProxyModel::setNameFilter(QString str) {
	m_nameFilter = str;
	invalidate();
	emit layoutChanged();
}

void MaterialTableProxyModel::setProducerFilter(QString str) {
	m_producerFilter = str;
	invalidate();
	emit layoutChanged();
}

void MaterialTableProxyModel::setSourceFilter(QString str) {
	m_sourceFilter = str;
	invalidate();
	emit layoutChanged();
}

void MaterialTableProxyModel::setCommentFilter(QString str) {
	m_commentFilter = str;
	invalidate();
	emit layoutChanged();
}

void MaterialTableProxyModel::setShowDeprecatedFilter(bool showDeprecated) {
	m_showDeprecatedFilter = showDeprecated;
	invalidate();
	emit layoutChanged();
}

void MaterialTableProxyModel::setCapabilityFilter(int capabilities) {
	m_capabilityFilter = capabilities;
	invalidate();
	emit layoutChanged();
}

QVariant MaterialTableProxyModel::headerData( int section, Qt::Orientation orientation, int role) const
{
	if (orientation == Qt::Vertical) {
		if (role == Qt::DisplayRole)
			return QString("%1").arg(section + 1);
	}
	return sourceModel()->headerData(section, orientation, role);
}

int MaterialTableProxyModel::getRow(int id) const {
	int count = sourceModel()->rowCount();
	if( id == -1)
		return -1;
	for( int i=0; i<count; ++i) {
		QModelIndex index = sourceModel()->index(i, 0, QModelIndex());
		MaterialBase* mat = static_cast<MaterialBase*>(index.internalPointer());
		if( mat != nullptr ) {
			if( mat->materialId() == id ) {
				QModelIndex lindex = this->mapFromSource(index);
				int row = lindex.row();
				return row;
			}
		}
	}
	return -1;
}

}
