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

#ifndef QtExt_FileSelectionWidgetH
#define QtExt_FileSelectionWidgetH

#include <QFileSystemModel>
#include <QAbstractListModel>

#include <QWidget>
#include <QFileIconProvider>

namespace QtExt {

namespace Ui {
class FileSelectionWidget;
}

class FileListModel;

/*! \brief Simple widget for selecting files from the file system.
	Contrary to the Qt internal function it doesn't create a new dialog widget.
	This makes it possible to use in delegates.*/
class FileSelectionWidget : public QWidget {
	Q_OBJECT

public:
	/*! Standard constructor.*/
	explicit FileSelectionWidget(QWidget *parent = nullptr);

	/*! Standard destructor.*/
	~FileSelectionWidget() override;

	/*! Return currently selected file or an empty string.*/
	QString filename() const;

	/*! Set the root directory for the directory tree view. This is the top level entry.
	   That means no directory above can be selected.*/
	void setRootDir(const QDir& root);

	/*! Set the currently selected directory in the tree.*/
	void setCurrentDir(const QDir& root);

	/*! Try to select directory and file.*/
	void setFilename(const QFileInfo& file);

	/*! Set filter list for files.
		e.g. "*.csv"
	*/
	void setNameFilters(const QStringList& filters);

protected:
	/*! Updates the column size of the directory tree view.*/
	void resizeEvent(QResizeEvent *event) override;

signals:
	/*! Signal will be emitted then the dialog should be closed.
		This is after double click at a file entry.*/
	void editingFinished();

private slots:
	/*! Updates the file list based on the selected directory.*/
	void on_treeViewDirectories_clicked(const QModelIndex &index);

	/*! Double click on a file updates the current file and close the widget.*/
	void on_listViewFiles_doubleClicked(const QModelIndex &index);

	/*! Updates the current file and the file editor.*/
	void on_listViewFiles_clicked(const QModelIndex &index);

private:
	Ui::FileSelectionWidget *ui;
	QFileSystemModel *		m_dirModel;		///< Tree model for file system
	FileListModel *			m_fileModel;	///< File list model
};

/*! List model for files.*/
class FileListModel : public QAbstractListModel {
	Q_OBJECT
public:
	/*! Standard constructor.*/
	explicit FileListModel(QObject* parent);

	/*! Number of files in the current directory.*/
	int rowCount(const QModelIndex &parent = QModelIndex()) const override;

	/*! Show data:
		 - Display - filename
		 - User    - file path
		 - Icon
	*/
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

	/*! All items are read only.*/
	Qt::ItemFlags flags(const QModelIndex &index) const override;

	/*! Set a new current dir and updates the internal file list.*/
	void update(const QDir& dir);

	/*! Set the file filter (e.g. "*.csv") and updates the file list.*/
	void setFilters(const QStringList& filters);

	/*! Return the index for the given file or an invalid one if the file is not in the list.*/
	QModelIndex indexForFile(const QFileInfo& file) const;

private:
	QFileInfoList		m_files;		///< Internal file list
	QStringList			m_filters;		///< List of file filters.
	QDir				m_rootDir;		///< Base directory
	QFileIconProvider	m_iconProvider;	///< provide icons for all files
};


} // namespace QtExt
#endif // QtExt_FileSelectionWidgetH
