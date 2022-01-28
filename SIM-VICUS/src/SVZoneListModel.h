#ifndef SVZoneListModelH
#define SVZoneListModelH

#include <QAbstractListModel>

namespace VICUS {
	class Project;
}

/*! A model that provides a list of zones and their IDs via UserRole.
	\note Implementation of member functions is in SVZoneSelectionDialog.cpp
*/
class SVZoneListModel : public QAbstractListModel {
	Q_OBJECT
public:

	SVZoneListModel(QObject * parent, const VICUS::Project &project);

	int rowCount(const QModelIndex & parent) const override;
	QVariant data(const QModelIndex & index, int role) const override;

private:
	QStringList					m_roomNames;
	std::vector<unsigned int>	m_roomIds;
};

#endif // SVZoneListModelH
