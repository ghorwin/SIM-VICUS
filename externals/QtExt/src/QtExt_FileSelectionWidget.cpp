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

#include "QtExt_FileSelectionWidget.h"
#include "ui_QtExt_FileSelectionWidget.h"

#include <QResizeEvent>

namespace QtExt {

FileSelectionWidget::FileSelectionWidget(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::FileSelectionWidget),
	m_dirModel(new QFileSystemModel(this)),
	m_fileModel(new FileListModel(this))
{
	ui->setupUi(this);

	// Set filter
	m_dirModel->setFilter(QDir::NoDotAndDotDot | QDir::AllDirs);

	// QFileSystemModel requires root path
	m_dirModel->setRootPath(QDir::root().absolutePath());

	// Attach the model to the view
	ui->treeViewDirectories->setModel(m_dirModel);

	// Set filter
	m_fileModel->setFilters(QStringList() << "*.tsv" << "*.csv");

	// QFileSystemModel requires root path
	m_fileModel->update(QDir::root().absolutePath());

	// Attach the model to the view
	ui->listViewFiles->setModel(m_fileModel);

	// this will update the focusProxy() and when the editor is created, it will focus the line edit
	setFocusProxy(ui->lineEditFilename);
}

FileSelectionWidget::~FileSelectionWidget() {
	delete ui;
}

QString FileSelectionWidget::filename() const {
	return ui->lineEditFilename->text();
}


void FileSelectionWidget::setRootDir(const QDir& root) {
	QModelIndex index = m_dirModel->setRootPath(root.absolutePath());
	ui->treeViewDirectories->setRootIndex(index);
	m_fileModel->update(root.absolutePath());
}

void FileSelectionWidget::setCurrentDir(const QDir& base) {
	QModelIndex index = m_dirModel->index(base.path());
	if(index.isValid())
		ui->treeViewDirectories->setCurrentIndex(index);
}

void FileSelectionWidget::setFilename(const QFileInfo& file) {
	QModelIndex index = m_dirModel->index(file.absolutePath());
	if (index.isValid()) {
		ui->treeViewDirectories->setCurrentIndex(index);
		QString mPath = m_dirModel->fileInfo(index).absoluteFilePath();
		m_fileModel->update(mPath);
	}

	index = m_fileModel->indexForFile(file);
	if (index.isValid())
		ui->listViewFiles->setCurrentIndex(index);
	ui->lineEditFilename->setText(file.absoluteFilePath());
}

void FileSelectionWidget::setNameFilters(const QStringList& filters) {
	m_fileModel->setFilters(filters);
}


// *** protected

void FileSelectionWidget::resizeEvent(QResizeEvent *event) {
	QWidget::resizeEvent(event);

	int treeWidth = event->size().width();
	ui->treeViewDirectories->setColumnWidth(0, treeWidth*2/3);
}


// *** private slots

void FileSelectionWidget::on_treeViewDirectories_clicked(const QModelIndex &index) {
	QString mPath = m_dirModel->fileInfo(index).absoluteFilePath();
	m_fileModel->update(mPath);
}

void FileSelectionWidget::on_listViewFiles_doubleClicked(const QModelIndex &index) {
	ui->lineEditFilename->setText(ui->listViewFiles->model()->data(index, Qt::UserRole).toString());
	emit editingFinished();
	close();
}

void QtExt::FileSelectionWidget::on_listViewFiles_clicked(const QModelIndex &index) {
	ui->lineEditFilename->setText(ui->listViewFiles->model()->data(index, Qt::UserRole).toString());
}



// *** File list model ****

FileListModel::FileListModel(QObject* parent) :
	QAbstractListModel(parent)
{
}

int FileListModel::rowCount(const QModelIndex &/*parent*/) const {
	return m_files.size();
}

QVariant FileListModel::data(const QModelIndex &index, int role) const {
	int row = index.row();
	if(row >= rowCount())
		return QVariant();
	switch(role) {
		case Qt::DisplayRole: {
			return m_files[row].fileName();
		}
		case Qt::UserRole: {
			return m_files[row].absoluteFilePath();
		}
		case Qt::DecorationRole: {
			return m_iconProvider.icon(m_files[row]);
		}
	}
	return QVariant();
}

Qt::ItemFlags FileListModel::flags(const QModelIndex &index) const  {
	if (!index.isValid())
		return Qt::NoItemFlags;
	return Qt::ItemIsEnabled;

}

void FileListModel::update(const QDir& dir) {
	m_rootDir = dir;
	beginResetModel();
	m_files = m_rootDir.entryInfoList(m_filters, QDir::Files);
	endResetModel();
}

void FileListModel::setFilters(const QStringList& filters) {
	m_filters = filters;
	update(m_rootDir);
}

QModelIndex FileListModel::indexForFile(const QFileInfo& file) const {
	int row = m_files.indexOf(file);
	if(row == -1)
		return QModelIndex();

	return index(row, 0, QModelIndex());
}

} // namespace QtExt
