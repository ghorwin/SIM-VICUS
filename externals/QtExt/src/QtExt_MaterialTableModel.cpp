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

#include "QtExt_MaterialTableModel.h"

#include <QFont>
#include <QPainter>

#include "QtExt_Constants.h"

namespace QtExt {

MaterialTableModel::MaterialTableModel(const QSet<MaterialBase::parameter_t>& visibleParams, QObject *parent)
  : QAbstractTableModel(parent),
	m_defaultRowHeight(20)
{
	for(int i=0; i<MaterialBase::NUM_MC; ++i) {
		MaterialBase::parameter_t para = static_cast<MaterialBase::parameter_t>(i);
		if(visibleParams.contains(para))
			m_columnParameterMapping.push_back(para);
	}
}

int MaterialTableModel::rowCount(const QModelIndex &parent) const
{
	if( parent.isValid() )
		return 0;
	return (int)m_materials.size();
}

int MaterialTableModel::columnCount(const QModelIndex &parent) const
{
	if( parent.isValid() )
		return 0;
	return (int)m_columnParameterMapping.size();
}

QVariant MaterialTableModel::headerData( int section, Qt::Orientation orientation, int role) const {
	if (orientation == Qt::Vertical)
		return QVariant();

	if( role == Qt::DisplayRole) {
		Q_ASSERT(section < (int)m_columnParameterMapping.size());

		switch(m_columnParameterMapping[(size_t)section]) {
			case MaterialBase::MC_ID				: return tr("ID");
			case MaterialBase::MC_PARAMETRIZATION	: return QVariant();
			case MaterialBase::MC_NAME				: return tr("Name");
			case MaterialBase::MC_CATEGORY			: return tr("Category");
			case MaterialBase::MC_PRODUCTID			: return tr("ProductName");
			case MaterialBase::MC_PRODUCER			: return tr("Producer");
			case MaterialBase::MC_SOURCE			: return tr("DataSource");
			case MaterialBase::MC_RHO				: return tr("%1 [kg/m%2]").arg(QString::fromWCharArray(L"\u03C1"), QString::fromWCharArray(L"\u00B3"));
			case MaterialBase::MC_CET				: return tr("cp [J/kgK]");
			case MaterialBase::MC_MEW				: return tr("%1 [-]").arg(QString::fromWCharArray(L"\u03BC"));
			case MaterialBase::MC_LAMBDA			: return tr("%1 [W/mK]").arg(QString::fromWCharArray(L"\u03BB"));
			case MaterialBase::MC_W80				: return tr("W80 [kg/m%1]").arg(QString::fromWCharArray(L"\u00B3"));
			case MaterialBase::MC_WSAT				: return tr("WSat [kg/m%1]").arg(QString::fromWCharArray(L"\u00B3"));
			case MaterialBase::MC_AW				: return tr("Aw\n[kg/m%1s%2]").arg(QString::fromWCharArray(L"\u00B2"), QString::fromWCharArray(L"\u00BD"));
			case MaterialBase::MC_SD				: return tr("sd [m]");
			case MaterialBase::MC_KAir				: return tr("Kair [s/m]");
			default : ;
		}
	}
	return QVariant();
}

QVariant MaterialTableModel::data(const QModelIndex &index, int role) const {
	int col = index.column();
	int row = index.row();
//	unsigned int matCount = m_materials.size();

	if (!index.isValid())
		return QVariant();
	const std::unique_ptr<MaterialBase>& mat = m_materials[row];
	if( mat.get() == nullptr)
		return QVariant();

	Q_ASSERT((size_t)col < m_columnParameterMapping.size());
	MaterialBase::parameter_t param = m_columnParameterMapping[(size_t)col];

	switch(role) {
		case Qt::DisplayRole:
		case Qt::EditRole: {
			switch (param) {
				case MaterialBase::MC_ID: return QString("%1").arg(mat->materialId());
				case MaterialBase::MC_NAME: return mat->name();
				case MaterialBase::MC_CATEGORY: return QVariant(mat->category().toString());
				case MaterialBase::MC_PRODUCTID: return mat->productName();
				case MaterialBase::MC_PRODUCER: return mat->producer();
				case MaterialBase::MC_SOURCE: return mat->dataSource();
				case MaterialBase::MC_RHO:		return mat->materialValueFormatter(param, mat->rho());
				case MaterialBase::MC_CET:		return mat->materialValueFormatter(param, mat->cp());
				case MaterialBase::MC_LAMBDA:	return mat->materialValueFormatter(param, mat->lambda());
				case MaterialBase::MC_MEW:		return mat->materialValueFormatter(param, mat->mu());
				case MaterialBase::MC_W80:		return mat->materialValueFormatter(param, mat->w_80());
				case MaterialBase::MC_WSAT:		return mat->materialValueFormatter(param, mat->w_sat());
				case MaterialBase::MC_AW:		return mat->materialValueFormatter(param, mat->Aw());
				case MaterialBase::MC_SD:		return mat->materialValueFormatter(param, mat->sdValue());
				case MaterialBase::MC_KAir:		return mat->materialValueFormatter(param, mat->Kair());

				case MaterialBase::MC_PARAMETRIZATION :; /// \todo not needed?
				default: ;
			}
			break;
		}
		case Qt::TextAlignmentRole: {
			// text left aligned
			if (param < MaterialBase::MC_RHO )
				return int(Qt::AlignLeft | Qt::AlignVCenter);

			return int(Qt::AlignRight | Qt::AlignVCenter);
		}
		case Qt::BackgroundRole: {
			// background will be handled by delegate
			break;
		}
		// UserRole is used for sorting
		case Qt::UserRole: {

			switch (param) {
				case MaterialBase::MC_ID				: return mat->materialId();
				case MaterialBase::MC_PARAMETRIZATION	: return mat->capabilities();
				case MaterialBase::MC_NAME				: return mat->name();
				case MaterialBase::MC_CATEGORY			: return mat->category().idx;
				case MaterialBase::MC_PRODUCTID			: return mat->productName();
				case MaterialBase::MC_PRODUCER			: return mat->producer();
				case MaterialBase::MC_SOURCE			: return mat->dataSource();
				case MaterialBase::MC_RHO				: return mat->rho();
				case MaterialBase::MC_CET				: return mat->cp();
				case MaterialBase::MC_LAMBDA			: return mat->lambda();
				case MaterialBase::MC_MEW				: return mat->mu();
				case MaterialBase::MC_W80				: return mat->w_80();
				case MaterialBase::MC_WSAT				: return mat->w_sat();
				case MaterialBase::MC_AW				: return mat->Aw();
				default :;
			}
			break;
		}
		case Qt::DecorationRole: {
			if (param == MaterialBase::MC_PARAMETRIZATION) {
				Q_ASSERT(row < m_capabilityPixmaps.count());
				return m_capabilityPixmaps[row];
			}
			break;
		}
		case Qt::TextColorRole : {
			if(mat->deprecatedState() == -1 || mat->deprecatedState() > 0) {
				return QColor(Qt::gray);
			}
			return QColor(Qt::black);
		}
		case Qt::FontRole : {
			if(mat->deprecatedState() == -1) {
				QFont f;
				f.setStrikeOut(true);
				return f;
			}
			break;
		}
		case IdRole: {
			return mat->materialIndex();
		}
		case BuiltInRole: {
			return !mat->isUserMaterial();
		}
		case ParaRole: {
			return param;
		}
	}
	return QVariant();
}

Qt::ItemFlags MaterialTableModel::flags(const QModelIndex &index) const {
	if (!index.isValid())
		return QAbstractItemModel::flags(index);

	const std::unique_ptr<MaterialBase>& mat = m_materials[index.row()];
	if( mat.get() != nullptr)
		return QAbstractItemModel::flags(index);

	return QAbstractItemModel::flags(index) | Qt::ItemIsSelectable;
}

QModelIndex MaterialTableModel::index( int row, int column, const QModelIndex & ) const
{
	if (row < 0 || row >= static_cast<int>(m_materials.size()))
		return QModelIndex();
	return createIndex(row, column, (void*)(m_materials[row].get()));
}

QModelIndex MaterialTableModel::parent( const QModelIndex &) const {
	return QModelIndex();
}

void MaterialTableModel::setMaterials(const std::vector<MaterialBase*>&	materials) {
	beginResetModel();
	m_materials.clear();
	foreach (MaterialBase* mat, materials) {
		m_materials.push_back(std::unique_ptr<MaterialBase>(mat));
	}
	updateCapabilityPixmaps();
	endResetModel();
}

static void drawBar(QPainter & p, unsigned int barIdx, unsigned int h, QColor c) {
	/// \todo 4K design check
	const unsigned int BARWIDTH = 6;
	p.setPen(Qt::NoPen);
	p.setBrush( c );
	p.drawRect(barIdx*BARWIDTH, 0, BARWIDTH, h);
//	p.setPen(Qt::black);
//	p.drawLine(barIdx*BARWIDTH-1, 0, barIdx*BARWIDTH-1, h);
}


void MaterialTableModel::updateCapabilityPixmaps() {
	m_capabilityPixmaps.clear();

	for (unsigned int i=0; i<m_materials.size(); ++i) {


		// depending on parameters render image
		QPixmap pixmap(ParametrizationColWidth,m_defaultRowHeight);
		pixmap.fill(QColor(255,255,255,0)); // transparent background
		QPainter p(&pixmap);

		// draw bars based on capability
		if (m_materials[i]->capabilities() & MaterialBase::TT_Thermal)
			drawBar(p, 0, m_defaultRowHeight, ColorHeat);
		if (m_materials[i]->capabilities() & MaterialBase::TT_Liquid)
			drawBar(p, 1, m_defaultRowHeight, ColorWater);
		if (m_materials[i]->capabilities() & MaterialBase::TT_Vapour)
			drawBar(p, 2, m_defaultRowHeight, ColorVapor);
		if (m_materials[i]->capabilities() & MaterialBase::TT_Airflow)
			drawBar(p, 3, m_defaultRowHeight, ColorAir);

		m_capabilityPixmaps.append(pixmap);
	}

}

const MaterialBase* MaterialTableModel::materialById(int id) {
	for (unsigned int i=0; i<m_materials.size(); ++i) {
		if(m_materials[i]->materialId() == id)
			return m_materials[i].get();
	}
	return nullptr;
}

void MaterialTableModel::removeMaterial(int id) {
	int pos = -1;
	for (unsigned int i=0; i<m_materials.size(); ++i) {
		if(m_materials[i]->materialId() == id) {
			pos = i;
			break;
		}
	}
	if(pos == -1)
		return;

	beginRemoveRows(QModelIndex(), pos, pos);
	m_materials.erase(std::remove(m_materials.begin(), m_materials.end(), *(m_materials.begin()+pos)), m_materials.end());
	updateCapabilityPixmaps();
	endRemoveRows();
}

bool MaterialTableModel::addMaterial(MaterialBase* material) {
	const MaterialBase* mat = materialById(material->materialId());
	if(mat != nullptr)
		return false;

	int lastPos = (int)m_materials.size();
	beginInsertRows(QModelIndex(), lastPos, lastPos);
	m_materials.push_back(std::unique_ptr<MaterialBase>(material));
	updateCapabilityPixmaps();
	endInsertRows();
	return true;
}

bool MaterialTableModel::changeMaterial(MaterialBase* material) {
	int pos = -1;
	for (size_t i=0; i<m_materials.size(); ++i) {
		if(m_materials[i]->materialId() == material->materialId()) {
			pos = (int)i;
			break;
		}
	}
	if(pos == -1)
		return false;

	m_materials[pos].reset(material);

	updateCapabilityPixmaps();
	int lastColumn = (int)m_columnParameterMapping.size() - 1;
	emit dataChanged(index(pos, 0, QModelIndex()), index(pos, lastColumn, QModelIndex()) );
	return true;
}


}
