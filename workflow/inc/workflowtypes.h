/***************************************************************************
 *  workflowtypes.h
 *
 *  Sun Oct 30 21:32:11 2011
 *  Copyright  2011  Christophe Seyve
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

#ifndef WORKFLOWTYPES_H
#define WORKFLOWTYPES_H

#include <QTreeWidgetItem>
#include <QString>

#include "imageinfo.h"

typedef struct {
	QAction * pAction;
	int index;
	t_movie_pos movie_pos;	/*!< Position in file */
	int percent;			/*!< Percentage of the movie size */
} video_bookmark_t;

/** @brief Structure to define a mapping between a file and a tree item

 Beware: containing objects

  */
typedef struct _t_file {
	QString name;
	QString fullPath;
	QTreeWidgetItem * treeViewItem;
} t_file;

/** @brief Folder listed */
typedef struct {
	QString fullpath;	///< Full path to directory
	QString filename;	///< basename
	QString extension;	///< File extension
	bool expanded;		///< Expanded state on explorer

	QTreeWidgetItem * treeViewItem;
} t_folder;

class EmaCollection;

#define EMAFILE_TYPE_UNKNOWN	0
#define EMAFILE_TYPE_IMAGE		1
#define EMAFILE_TYPE_MOVIE		2
#define EMAFILE_TYPE_OPENNI		3



/** @brief File known in collection */
typedef struct {
	QString fullpath;	///< Full path to file
	QString filename;	///< basename
	int type;			///< Type: movie, image
	EmaCollection * pCollection; ///< Reverse pointer to its collection

	QTreeWidgetItem * treeViewItem;	///< pointer on item, which is of type DirectoryTreeWidgetItem
} t_collection_file;

/** @brief Collection storage */
class EmaCollection
{
public:
	EmaCollection();
	~EmaCollection();

	void clearFiles();
	void appendFiles(QStringList listOfFiles);
	void removeFiles(QStringList listOfFiles);
	void appendSubCollection(EmaCollection * sub);

	QString title;
	QString comment;
	QList<t_collection_file *> filesList;	///< List of full path to their images
	QList<EmaCollection *> subCollectionsList;

	QTreeWidgetItem * treeViewItem;
};

// a few functions for managing collections

/** @brief Remove a collection from a list */
void removeCollection(QList<EmaCollection *> * pCollList, EmaCollection * delCol);


typedef struct {
	QString node;
} t_device;

#endif // WORKFLOWTYPES_H
