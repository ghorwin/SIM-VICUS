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

#ifndef QtExt_MaterialTableModelH
#define QtExt_MaterialTableModelH

#include <QAbstractTableModel>
#include <QList>
#include <QSet>
#include <QPixmap>

#include <memory>
#include <map>

#include "QtExt_MaterialBase.h"


namespace QtExt {

/*! Implements the model for the TableView of the MaterialDatabaseWidget.
*/
class MaterialTableModel : public QAbstractTableModel {
	Q_OBJECT
public:

	static const unsigned int ParametrizationColWidth = 36;

	/*! Constructor of the model.
		\param parent	Parent widget.
	*/
	explicit MaterialTableModel(const QSet<MaterialBase::parameter_t>& visibleParams, QObject *parent = 0);

	/*! returns row count (from matList.size()).*/
	int rowCount(const QModelIndex &parent = QModelIndex()) const;

	/*! Returns column count (fixed to 10).*/
	int columnCount(const QModelIndex &parent = QModelIndex()) const;

	/*! returns the data dependent on role.
		\arg DisplaRole and EditRole: value from material.
		\arg TextAlignmentRole: different alignments for strings and numbers.
		\arg BackgroundRole: different background color for user defined material
	*/
	QVariant data(const QModelIndex &index, int role) const;

	/*! returns the header strings*/
	QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;

	/*! returns flags for selectable but not editable items*/
	Qt::ItemFlags flags(const QModelIndex &index) const;

	/*! returns index of the given item and.
		sets internalPointer to a pointer of the corresponding material (class Material)
	*/
	QModelIndex index( int row, int column, const QModelIndex & parent ) const;

	/*! returns only QModelIndex() (no parent)*/
	QModelIndex parent( const QModelIndex & index ) const;

	/*! Fill the material list with pointer to MaterialBase.
		The class takes ownership of this pointer.
		MaterialBase is abstract. Therefore each caller has to create own derivated classes.
	*/
	void setMaterials(const std::vector<MaterialBase*>&	materials);

	/*! Set a new row height used for drawing capability pixmaps.*/
	void setDefaultRowHeight(unsigned int val) { m_defaultRowHeight = val; }

public slots:
	/*! remove material with given id from internal list.
		Nothing will happen if id doesn't exist.
		Must be called if this material is also removed from parent database.
	*/
	void removeMaterial(int id);

	/*! Add given material to internal list.
		Id must be unique. In case of already existing id function return false.
		Must be called if this material is also added to parent database.
		The function takes ownership of the pointer. Don't delete it outside this class.
	*/
	bool addMaterial(MaterialBase* material);

	/*! Set given material to position of existing material with same id.
		Return false if id doesn't exist.
		Must be called if this material is also changed in parent database.
		The function takes ownership of the pointer. Don't delete it outside this class.
		The old item will be deleted.
	*/
	bool changeMaterial(MaterialBase* material);

signals:
	/*! Set selection to the material given by id.*/
	void selectMaterial(unsigned int id);

private:

	/*! Updates cached set of capability/parametrization pixmaps. */
	void updateCapabilityPixmaps();

	/*! search for material with given id.
		Return nullptr if not found.*/
	const MaterialBase* materialById(int id);

	/*! Material list for one language.*/
	std::vector<std::unique_ptr<MaterialBase> >	m_materials;

	/*! Map a column to the corresponding parameter.*/
	std::vector<MaterialBase::parameter_t>		m_columnParameterMapping;

	/*! Default row height must be set from parent widget.*/
	unsigned int								m_defaultRowHeight;

	/*! Cached set of capability pixmaps.
		Updated in updateCapabilityPixmaps().
	*/
	QList<QPixmap>								m_capabilityPixmaps;


};

}

/*! @file QtExt_MaterialTableModel.h
	@brief Contains the class MaterialTableModel.
*/

#endif // QtExt_MaterialTableModelH
