#ifndef SVDBConstructionTreeModelH
#define SVDBConstructionTreeModelH

#include <QAbstractItemModel>

#include <VICUS_Construction.h>

/*! A tree model that provides data from construction database.
*/
class SVDBConstructionTreeModel : public QAbstractItemModel {
	Q_OBJECT
public:

	/*! Columns, provided for leaf nodes. */
	enum Columns {
		C_Name,
		C_UValue,
		NUM_C
	};

	explicit SVDBConstructionTreeModel(QObject *parent = nullptr);

	// Header:
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

	// Basic functionality:
	QModelIndex index(int row, int column,
					  const QModelIndex &parent = QModelIndex()) const override;
	QModelIndex parent(const QModelIndex &index) const override;

	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	int columnCount(const QModelIndex &parent = QModelIndex()) const override;

	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

	void setDataStore(std::map<unsigned int, VICUS::Construction> & dbConstructions);

private:
	/*! Data structure, maps to data structure. */
	struct CategoryItem {
		VICUS::Construction::UsageType	m_usageType;
		QString							m_categoryName;
		std::vector<unsigned int>		m_constructions;
	};

	/*! Categories are the top-level nodes.
		Child nodes are construction types, here identified by their ID.
	*/
	std::vector<CategoryItem>			m_categoryItems;

	/*! Pointer to data storager for this model (owned by SVSettings). */
	std::map<unsigned int, VICUS::Construction> * m_dbConstructions;
};

#endif // SVDBConstructionTreeModelH
