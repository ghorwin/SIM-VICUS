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

#ifndef QtExt_FileTreeSelectorH
#define QtExt_FileTreeSelectorH

#include <QWidget>
#include <QTreeWidget>
#include <QDir>


namespace QtExt {

/*! \brief Class contains filename, path and splitted path.
*/
struct FileWrapper {
	/*! Standard constructor.*/
	FileWrapper() :
		m_temporary(false)
	{}
	/*! Filename without path and extension.*/
	QString		m_name;
	/*! Filename without path.*/
	QString		m_filename;
	/*! List of category strings which will be created from directory structure.*/
	QStringList	m_categories;
	/*! Complete description of file.*/
	QFileInfo	m_file;
	/*! If true its a temporary file.*/
	bool m_temporary;
};

/*! \brief Tree view for selecting files based on a given root path.
	A filter for specific filenames and/or file extensions can be set.
*/
class FileTreeSelector : public QWidget
{
	Q_OBJECT
public:

	enum Option {
		ShowExtensions,			///< Show filename with extension.
		CanSelectDirectories,	///< Selection of directories is allowed
		MultiSelection			///< Selection of multiple files allowed
	};

	/*! Standard constructor.*/
	explicit FileTreeSelector(QWidget *parent = nullptr);

	/*! Return the currently selected files.*/
	QStringList selectedFiles() const;

	/*! Set root path as start for file search.*/
	void setRootPath(const QString& path, bool forceUpdate = true);

	/*! Set extensions as filter.
		\param filterExtensions List of extension (with or without point).
		\param forceUpdate If true treeview will be updated after set of new filters.
	*/
	void setExtensionsFilter(const QStringList& filterExtensions, bool forceUpdate = true);

	/*! Set filenames as filter.
		\param filterFiles List of file names.
		\param forceUpdate If true treeview will be updated after set of new filters.
	*/
	void setFileFilter(const QStringList& filterFiles, bool forceUpdate = true);

	/*! Set the option \a opt to the value \a isSet.*/
	void setOption(Option opt, bool isSet);

	/*! Clear all current selections.*/
	void clearSelections();

	/*! Add a file to the list of files in order to add these temperary to the list.*/
	void addFileToAdd(const QString& filename, const QStringList& categories);

	/*! Clear list of temporary files.*/
	void clearFilesToAdd();

	/*! Return the current file list. update must be called in order to create a valid list.*/
	const QList<FileWrapper> files() const;

signals:
	void selectionChanged(QStringList files);

public slots:

	/*! Triggerd from tree vie if selection is changed.
		Check if selection is valid and emit signal climateSelected with corresponding climatWrapper object.*/
	void onSelectionChanged();

	/*! Updates the tree view content.*/
	void update();

private:

	/*! Split the directories into a category list.*/
	bool categoriesFromDir(const QDir& filePath, QStringList& res);

	/*! Helper function for finding tree items.*/
	QTreeWidgetItem* findChildByText(QTreeWidgetItem* parent, const QString& text) const;

	QTreeWidget*		m_treeWidget;
	QString				m_rootPath;
	QStringList			m_filter;
	QStringList			m_fileFilter;
	QStringList			m_pathList;
	QList<FileWrapper>	m_files;
	QList<FileWrapper>	m_filesToAdd;
	QStringList			m_selectedFiles;
	QSet<Option>		m_options;
};

} // namespace QtExt

#endif // QtExt_FileTreeSelectorH
