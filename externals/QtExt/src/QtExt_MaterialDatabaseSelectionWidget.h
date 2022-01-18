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

#ifndef QtExt_MaterialDatabaseSelectionWidgetH
#define QtExt_MaterialDatabaseSelectionWidgetH

#include <QWidget>
#include <QList>
#include <QItemSelection>

#include <memory>

#include "QtExt_MaterialBase.h"

namespace QtExt {

class MaterialTableProxyModel;
class MaterialTableModel;
class MaterialDBItemDelegate;

namespace Ui {
	class MaterialDatabaseSelectionWidget;
}

/*! Basic widget for selecting materials from a database.
	It only contains viewing and filtering functions. Adding or removing materials from database has to be done outside.

	The table view inside the widget can be configured to only show a subset of all available columns.
	Once the widget has been instantiated, materials must be set in the widget with setMaterials().

	The widget includes functionality for adding/copying and removing materials from the list. These operations
	modify the internally stored list of materials, and emits corresponding signals which allows
	classes using the widget to keep their data store in sync.

	The widget remembers the column widths adjusted by users in a settings object.
*/
class MaterialDatabaseSelectionWidget : public QWidget {
	Q_OBJECT

public:
	/*! Standard constructor.
		Function setup must be called shortly after creating a object of this widget.
	*/
	explicit MaterialDatabaseSelectionWidget(QWidget *parent = 0);

	/*! Extended constructor.*/
	MaterialDatabaseSelectionWidget(const QSet<MaterialBase::parameter_t>& visibleParams,
									const QString& orgName, const QString& programName, const QString& widgetName,
									QWidget *parent = 0);

	/*! Standard destructor, only deletes ui.*/
	~MaterialDatabaseSelectionWidget() override;

	/*! Setup of view. Must be called in case of use of standard constructor. The extended constructor calls setup itself.
		\param visibleParams Set of parameters for view in database table.
		\param orgName Organisation name for use in settings for store view parameters
		\param programName Name of the calling program for use in settings for store view parameters
		\param widgetName Name of the widget for use in settings for store view parameters
	*/
	void setup(const QSet<MaterialBase::parameter_t>& visibleParams,
			   const QString& orgName, const QString& programName, const QString& widgetName);

	/*! Fill the material list with pointer to MaterialBase.
		The model takes ownership of this pointer.
		MaterialBase is abstract. Therefore each caller has to create own derivated classes.
	*/
	void setMaterials(const std::vector<MaterialBase*>&	materials);

	/*! Set selection to material with given id.*/
	void selectMaterial(int id);

	/*! Set view to use default color setting (\sa QtExt::Style).*/
	void setDefaultColors();

	/*! Set colors for user defined color setting.*/
	void setUserDefinedColors(const QColor& bright, const QColor& dark);

	/*! Set color for not built-in materials.*/
	void setUserColor(const QColor& col);

	/*! Set use of alternating colors for rows in view.
		Default setting is on.
		Colors can be set by using setUserColors or setDefaultColors.
		\param alternate If true alternating colors are used.
	*/
	void setAlternatingColors(bool alternate);

	/*! Sets a new filter text. */
	void setMaterialNameFilter(const QString & filterText);

	/*! Sets a new filter text. */
	void setProducerFilter(const QString & filterText);

	static QString parameterDescription(MaterialBase::parameter_t type);

public slots:
	/*! remove material with given id from internal list.
		Nothing will happen if id doesn't exist.
		Must be called if this material is also removed from parent database.
	*/
	void removeMaterial(int id);

	/*! Add given material to internal list.
		Id must be unique. In case of already existing id function return false.
		Must be called if this material is also added to parent database.
	*/
	bool addMaterial(MaterialBase*);

	/*! Set given material to position of existing material with same id.
		Return false if id doesn't exist.
		Must be called if this material is also changed in parent database.
	*/
	bool changeMaterial(MaterialBase*);

signals:
	void materialSelected(int id);
	void doubleClickMaterials(int id);

protected:
	/*! Store the current column sizes.*/
	void hideEvent(QHideEvent *event) override;

	/*! Re-implemented to resize columns based on content, which is only done on first view,
		afterwards user can adjust column sizes at will.
	*/
	void showEvent(QShowEvent *event) override;

private slots:

	/*! Connected to selction signal of proxy model.*/
	void onMaterialSelectionChanged(const QItemSelection &selected, const QItemSelection &);

	/*! Connected to selction signal of proxy model.*/
	void onMaterialDoubleClicked(QModelIndex index);

	/*! Triggered by category combo.*/
	void on_comboBoxCategories_currentIndexChanged(int index);

	/*! Triggered by name filter edit field.*/
	void on_lineEditMaterialNameFilter_textChanged(const QString &arg1);

	/*! Triggered by producer filter edit field.*/
	void on_lineEditProducerFilter_textChanged(const QString &arg1);

	/*! Is triggerd when a column size is changed.*/
	void columnSizeChanged(int logicalIndex, int oldSize, int newSize);

	/*! Is triggerd when the user clicks at a column header.*/
	void columnClicked(int logicalIndex);

	void updateColumnSizes(bool hasSettings);

	void on_lineEditSourceFilter_textChanged(const QString &arg1);

	void on_lineEditCommentFilter_textChanged(const QString &arg1);

private:

	Ui::MaterialDatabaseSelectionWidget *	ui;
	/*! Model for material table view.*/
	MaterialTableModel*						m_model;
	MaterialTableProxyModel*				m_proxyModel;
	MaterialDBItemDelegate *				m_delegate;
	std::map<MaterialBase::parameter_t,int>	m_parameterSizeMap;
	std::vector<MaterialBase::parameter_t>	m_columnToParameter;
	QSet<MaterialBase::parameter_t>			m_visibleParams;
	QString									m_orgName;			///< Organization name. Used for handling settings
	QString									m_programName;		///< program name. Used for handling settings
	QString									m_widgetName;		///< program name. Used for handling settings
	int										m_nameColumn;
};


} // namespace QtExt
#endif // QtExt_MaterialDatabaseSelectionWidgetH
