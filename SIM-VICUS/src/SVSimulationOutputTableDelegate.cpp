#include "SVSimulationOutputTableDelegate.h"

#include <QComboBox>

#include <VICUS_Outputs.h>
#include <VICUS_OutputDefinition.h>
#include <VICUS_KeywordListQt.h>


QWidget * SVSimulationOutputTableDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const {
	// special handling for grid column and time-type column
	switch (index.column()) {
		case 4 : {
			QComboBox * combo = new QComboBox(parent);
			for (int t = 0; t<VICUS::OutputDefinition::NUM_OTT; ++t) {
				QString ttypekw = VICUS::KeywordListQt::Keyword("OutputDefinition::timeType_t", t);
				combo->addItem(ttypekw);
			}
			return combo;
		}

		case 5 : {
			QComboBox * combo = new QComboBox(parent);
			if (m_outputs != nullptr) { // outputs can be nullptr if empty
				for (unsigned int i = 0; i<m_outputs->m_grids.size(); ++i)
					combo->addItem( QString::fromStdString(m_outputs->m_grids[i].m_name) );
			}
			return combo;
		}
	}
	return QAbstractItemDelegate::createEditor(parent, option, index);
}


void SVSimulationOutputTableDelegate::setEditorData(QWidget * editor, const QModelIndex & index) const {
	switch (index.column()) {
		case 4 :
		case 5 : {
			QComboBox * combo = qobject_cast<QComboBox *>(editor);
			Q_ASSERT(combo);
			combo->setCurrentText(index.data(Qt::DisplayRole).toString());
			return;
		}
	}
	QAbstractItemDelegate::setEditorData(editor, index);
}

