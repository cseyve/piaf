/***************************************************************************
 *            emamainwindow.h
 *
 *  Wed Jun 11 08:47:41 2009
 *  Copyright  2009  Christophe Seyve
 *  Email cseyve@free.fr
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef EMAMAINWINDOW_H
#define EMAMAINWINDOW_H

#include <QtCore/QTimer>

#include <QtGui/QMainWindow>
#include <QtGui/QTreeWidgetItem>
#include <QWorkspace>
#include <QSettings>

#include "workshopimagetool.h"


typedef struct _t_file {
	QString name;
	QString fullPath;
	QTreeWidgetItem * treeViewItem;
} t_file;

class EmaMainWindow;

/** \brief Directory tree */
class DirectoryTreeWidgetItem : QTreeWidgetItem
{
//	Q_OBJECT
	friend class EmaMainWindow;
public:
	DirectoryTreeWidgetItem(QTreeWidgetItem * treeWidgetItemParent, QString path);
	DirectoryTreeWidgetItem(QTreeWidget * treeWidgetParent, QString path);
	~DirectoryTreeWidgetItem();

	/** \brief Expand contents: read directory contents and display it */
	void expand();
	/** \brief collapse contents: collapse directory and clear data */
	void collapse();

	bool isFile() { return mIsFile; }
	QStringList getFileList();
protected:
	void init(); ///< init internal data and fill nb files
	QString name;
	QString fullPath;
	int nb_files;
	bool mIsFile;///< true if the item is a file
	QTreeWidgetItem * subItem;///< fake item to see the content
private:

	QList<DirectoryTreeWidgetItem *> subDirsList;
	QList<t_file *> filesList;


};


namespace Ui
{
	class EmaMainWindow;
}

/** @brief File known in collection */
typedef struct {
	QString fullpath;	///< Full path to file
	QString filename;	///< basename
	int type;			///< Type: movie, image
	struct _t_collection * pCollection; ///< Reverse pointer to its collection
} t_collection_file;

/** @brief Collection storage */
typedef struct _t_collection {
	QString title;
	QString comment;
	QList<t_collection_file *> filesList;	///< List of full path to their images
	QList<struct _t_collection *> subCollectionsList;
} t_collection;

typedef struct {
	QString node;
} t_device;

/** @brief Workflow interface settings storage */
typedef struct {
	/// List of input directories
	QStringList directoryList;

	/// list of collections
	QList<t_collection> collectionList;

	/// List of devices
	QList<t_device> devicesList;

} t_workflow_settings ;


/** \brief Main window for workflow app


*/
class EmaMainWindow : public QMainWindow
{
	Q_OBJECT


public:
	EmaMainWindow(QWidget *parent = 0);
	~EmaMainWindow();

private:

	Ui::EmaMainWindow *ui;

	void saveSettings();
	void loadSettings();
#define PIAFWKFL_SETTINGS	"piaf-workflow"
	QSettings mSettings;	///< Qt Settings object for piaf workflow

	t_workflow_settings m_workflow_settings;
	/// List of input directories
	QList<DirectoryTreeWidgetItem *> mDirectoryList;

	/// Opened image paths
	QStringList m_imageList;
	QStringList m_appendFileList;
	QStringList m_removeFileList;

	// Thumbnails sorter
	int m_sorter_nbitems_per_row;

	/** @brief append a directory to managed directories */
	DirectoryTreeWidgetItem * appendDirectoryToLibrary(QString path, QTreeWidgetItem * itemParent = NULL);

	/** @brief append a list of files to displayed files */
	void appendFileList(QStringList list);

	/** @brief remove a list of files to displayed files */
	void removeFileList(QStringList list);

	void appendThumbImage(QString fileName);
	void removeThumbImage(QString fileName);

private:
	QTimer m_timer;
	QWorkspace * pWorkspace;
	QImage mWorkImage;

	WorkshopImage * pWorkshopImage;
	WorkshopImageTool * pWorkshopImageTool;
public slots:
	void on_thumbImage_clicked(QString fileName);
	void on_thumbImage_selected(QString);
private slots:
	void on_actionView_right_column_toggled(bool );
 void on_actionView_left_column_toggled(bool );
 void on_batchPlayerButton_clicked();
	void on_filesTreeWidget_itemExpanded(QTreeWidgetItem* item);
	void on_workspaceButton_clicked();
	void on_filesShowCheckBox_stateChanged(int);
	void on_filesLoadButton_clicked();
	void on_filesClearButton_clicked();
	void on_zoomx2Button_clicked();
	void on_zoomx1Button_clicked();
	void on_groupBox_7_clicked();

	void on_gridWidget_signal_resizeEvent(QResizeEvent *);

	// Collections
	void on_collecShowCheckBox_stateChanged(int);

	void on_timer_timeout();

	void on_gridButton_clicked();
	void on_imgButton_clicked();

	void on_mainGroupBox_resizeEvent(QResizeEvent *);
	//
	void on_globalNavImageWidget_signalZoomOn(int, int, int);

	//void on_mainDisplayWidget_signalGreyscaleToggled(bool);
	void on_mainDisplayWidget_signalZoomRect(QRect cropRect);

	// MAIN MENU ACTION
	void on_actionQuit_activated();
	void on_actionEdit_plugins_activated();
	void on_actionBatch_processor_activated();
	void on_actionConvert_images_to_AVI_activated();
	void on_actionAbout_activated();

	void on_appendNewPictureThumb( QString  );

	void on_filesTreeWidget_itemClicked ( QTreeWidgetItem * item, int column );
	void on_filesTreeWidget_itemDoubleClicked ( QTreeWidgetItem * item, int column );
};

#endif // EmaMAINWINDOW_H