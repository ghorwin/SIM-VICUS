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

#include "QtExt_FileTreeSelector.h"

#include <QVBoxLayout>
#include <QHeaderView>

namespace QtExt {


static void recursiveSearch(QDir baseDir, QStringList & files, const QStringList & extensions, const QStringList& fileFilter) {
	QStringList	fileList = baseDir.entryList(QStringList(), QDir::AllEntries | QDir::NoDotAndDotDot, QDir::Name);

	foreach (QString f, fileList) {
		QString fullPath = baseDir.absoluteFilePath(f);
		QFileInfo finfo(fullPath);
		if (finfo.isDir()) {
			recursiveSearch(QDir(fullPath), files, extensions, fileFilter);
		}
		else {
			bool found = false;
			foreach (QString ext, extensions) {
				if (ext == "*" || ext == "*.*" || finfo.suffix() == ext) {
					found = true;
					break;
				}
			}
			foreach (QString file, fileFilter) {
				if (file == "*.*" || file == finfo.fileName()) {
					found = true;
					break;
				}
			}
			if (found)
				files.append(fullPath);
		}
	}
}

FileTreeSelector::FileTreeSelector(QWidget *parent) :
	QWidget(parent),
	m_treeWidget(new QTreeWidget(this))
{
	QVBoxLayout * lay  = new QVBoxLayout(this);
	lay->addWidget(m_treeWidget);
	setLayout(lay);
	m_filter << "*.*";

	m_treeWidget->header()->hide();

	connect(m_treeWidget, SIGNAL(itemSelectionChanged()),	this,SLOT(onSelectionChanged()));

}

QStringList FileTreeSelector::selectedFiles() const {
	return m_selectedFiles;
}

void FileTreeSelector::setRootPath(const QString& path, bool forceUpdate) {
	m_rootPath = path;
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
	m_pathList = QDir::toNativeSeparators(m_rootPath).split(QDir::separator(), Qt::SkipEmptyParts);
#else
	m_pathList = QDir::toNativeSeparators(m_rootPath).split(QDir::separator(), QString::SkipEmptyParts);
#endif
	if(forceUpdate)
		update();
}

void FileTreeSelector::setExtensionsFilter(const QStringList& filter, bool forceUpdate) {
	m_filter = filter;
	// remove . and parts before in order to have the bare extension
	for(QString& str: m_filter) {
		int pos = str.indexOf('.');
		if(pos != -1) {
			str = str.right(str.size() - pos - 1);
		}
	}
	if(forceUpdate)
		update();
}

void FileTreeSelector::setFileFilter(const QStringList& filter, bool forceUpdate) {
	m_fileFilter = filter;
	if(forceUpdate)
		update();
}

void FileTreeSelector::setOption(Option opt, bool isSet) {
	if(isSet)
		m_options.insert(opt);
	else
		m_options.remove(opt);
	if(opt == MultiSelection) {
		if(isSet)
			m_treeWidget->setSelectionMode(QAbstractItemView::MultiSelection);
		else
			m_treeWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	}
}

void FileTreeSelector::clearSelections() {
	m_treeWidget->clearSelection();
}

void FileTreeSelector::addFileToAdd(const QString& filename, const QStringList& categories) {
	FileWrapper tw;
	tw.m_filename = filename;
	int pos = filename.lastIndexOf('.');
	tw.m_name =  filename.left(pos);
	tw.m_categories = categories;
	tw.m_temporary = true;
	m_filesToAdd.push_back(tw);
	update();
}

void FileTreeSelector::clearFilesToAdd() {
	m_filesToAdd.clear();
	update();
}

const QList<FileWrapper> FileTreeSelector::files() const {
	return m_files;
}

void FileTreeSelector::onSelectionChanged() {
	QList<QTreeWidgetItem *> selected = m_treeWidget->selectedItems();
	m_selectedFiles.clear();
	for(int i=0; i<selected.size(); ++i) {
		QTreeWidgetItem* widget =selected[i];
		Q_ASSERT(widget != nullptr);
		QString file = widget->data(0, Qt::UserRole).toString();
		if(!file.isEmpty())
			m_selectedFiles << file;
	}
	emit selectionChanged(m_selectedFiles);
}

void FileTreeSelector::update() {
	m_treeWidget->clear();
	m_files.clear();
	QStringList files;
	QDir rootDir(m_rootPath);
	if(rootDir.exists()) {
		recursiveSearch(rootDir, files, m_filter, m_fileFilter);
	}
	else {
		return;
	}

	foreach(const QString& file, files) {
		QFileInfo fileInfo(file);
		FileWrapper item;
		item.m_filename = fileInfo.fileName();
		item.m_name = fileInfo.baseName();
		item.m_file = fileInfo;
		if(categoriesFromDir(fileInfo.path(), item.m_categories))
			m_files.push_back(item);
	}
	if(!m_filesToAdd.isEmpty()) {
		m_files << m_filesToAdd;
	}

	for(int i=0; i<m_files.size(); ++i) {
		int catlevels = m_files[i].m_categories.size();
		QTreeWidgetItem * currentItem = 0;
		for(int j=0; j<catlevels; ++j) {
			QString name = m_files[i].m_categories[j];
			QTreeWidgetItem * item = findChildByText(currentItem, name);
			if(item == nullptr) {
				if(j==0) {
					currentItem = new QTreeWidgetItem(m_treeWidget, QStringList(name));
					m_treeWidget->addTopLevelItem(currentItem);
				}
				else {
					QString lastItem = m_files[i].m_categories[j-1];
					Q_ASSERT(currentItem != nullptr);
					Q_ASSERT(currentItem->text(0) == lastItem);
					QTreeWidgetItem * parentItem = currentItem;
					currentItem = new QTreeWidgetItem(parentItem, QStringList(name));
					parentItem->addChild(currentItem);
				}
				if(m_options.contains(CanSelectDirectories))
					currentItem->setFlags(currentItem->flags() | Qt::ItemIsSelectable);
				else
					currentItem->setFlags(currentItem->flags() & ~Qt::ItemIsSelectable);
			}
			else {
				currentItem = item;
			}
		}
		QStringList text;
		if(m_options.contains(ShowExtensions))
			text << m_files[i].m_filename;
		else {
			text << m_files[i].m_name;
		}
		if(currentItem != nullptr) {
			QTreeWidgetItem* newItem = new QTreeWidgetItem(currentItem, text);
			newItem->setData(0, Qt::UserRole, m_files[i].m_file.absoluteFilePath());
			if(m_files[i].m_temporary)
				newItem->setForeground(0, Qt::red);
			newItem->setFlags(newItem->flags() | Qt::ItemIsSelectable);
			currentItem->addChild(newItem);
		}
		else {
			QTreeWidgetItem* newItem = new QTreeWidgetItem(m_treeWidget, text);
			newItem->setData(0, Qt::UserRole, m_files[i].m_file.absoluteFilePath());
			if(m_files[i].m_temporary)
				newItem->setForeground(0, Qt::red);
			newItem->setFlags(newItem->flags() | Qt::ItemIsSelectable);
			m_treeWidget->addTopLevelItem(newItem);
		}
	}
	m_treeWidget->expandAll();
}

bool FileTreeSelector::categoriesFromDir(const QDir& filePath, QStringList& res) {
	res.clear();
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
	QStringList filePathList = QDir::toNativeSeparators(filePath.absolutePath()).split(QDir::separator(), Qt::SkipEmptyParts);
#else
	QStringList filePathList = QDir::toNativeSeparators(filePath.absolutePath()).split(QDir::separator(), QString::SkipEmptyParts);
#endif
	if(m_pathList.size() > filePathList.size())
		return false;

	for(int i=0; i<filePathList.size(); ++i) {
		if(i<m_pathList.size()) {
			if(m_pathList[i] != filePathList[i])
				return false;
		}
		else {
			res << filePathList[i];
		}
	}
	return true;
}

QTreeWidgetItem* FileTreeSelector::findChildByText(QTreeWidgetItem* parent, const QString& text) const {
	if(parent == nullptr) {
		QList<QTreeWidgetItem *> items = m_treeWidget->findItems(text, Qt::MatchExactly, 0);
		if(items.isEmpty())
			return nullptr;

		return items.front();
	}

	int count = parent->childCount();
	for(int i=0; i<count; ++i) {
		if(parent->child(i)->text(0) == text)
			return parent->child(i);
	}
	return nullptr;
}

} // namespace QtExt
