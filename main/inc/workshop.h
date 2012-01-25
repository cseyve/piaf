/***************************************************************************
						  workshop.h  -  main IHM for Piaf
							 -------------------
	begin                : ven nov 29 15:53:48 UTC 2002
	copyright            : (C) 2002 by Olivier Viné & Christophe Seyve
	email                : olivier.vine@sisell.com
							cseyve@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef WORKSHOP_H
#define WORKSHOP_H

// include files for QT
#include <qapplication.h>
#include <q3mainwindow.h>
#include <qaction.h>
#include <qworkspace.h>
#include <qmenubar.h>
#include <q3toolbar.h>
#include <qstatusbar.h>
#include <q3whatsthis.h>
#include <q3popupmenu.h>
#include <q3accel.h>
#include <qtoolbutton.h>
#include <qmessagebox.h>
#include <q3filedialog.h>
#include <qprinter.h>
#include <qstring.h>
#include <qpixmap.h>
//Added by qt3to4:
#include <Q3PtrList>
#include <QEvent>
#include <Q3ActionGroup>

#include "workshopmeasure.h"
#include "workshopimage.h"
#include "workshopmovie.h"

// forward declaration of the Workshop classes
//class WorkshopDoc;
class MultiDocView;
class RandSignalForm;
class WorkshopList;
class ObjectsExplorer;
class PreviewPlot2D;
class PreviewImage;
class WorkshopPlot2D;
class WorkshopTool;
class WorkshopImage;
class SwVideoAcquisition;
class VideoCaptureDoc;
class WorkshopVideoCaptureView;


// Base components definition
// --------------------------
#define NB_COMPONENTS  3
#include "piaf-settings.h"

/**
  This Class is the base class for your application. It sets up the main
  window and providing a menubar, toolbar
  and statusbar. For the main view, an instance of class WorkshopView is
  created which creates your view.

	\brief Main GUI and component handling class.

  \author Source Framework Automatically Generated by KDevelop, (c) The KDevelop Team.

  */
class WorkshopApp : public Q3MainWindow
{
	Q_OBJECT

public:
	/** construtor */
	WorkshopApp();
	/** destructor */
	~WorkshopApp();

	// Fundamentals "create_viewers" functions
	void createPlot2D(WorkshopMeasure* doc);
	/// create a viewer for video capture
	void createVideoCaptureView(VideoCaptureDoc * va);
	void createImageView();


	// SISELL add functions
	void addNewMeasure(WorkshopMeasure *src, int type);
	void addNewMeasure(WorkshopMeasure *pWm);
	WorkshopMeasure *createNewMeasure(WorkshopMeasure *src, int type);

	void addNewImage(WorkshopImage *pWi);
	void deleteImage(WorkshopImage *pWi);

	void addNewMovie(WorkshopMovie *pWi);
	void deleteMovie(WorkshopMovie *pWi);

protected:
	/** overloaded for Message box on last window exit */
	bool queryExit();
	/** event filter to catch close events for MDI child windows and is installed in createClient() on every child window.
	  * Closing a window calls the eventFilter first which removes the view from the connected documents' view list. If the
	  * last view is going to be closed, the eventFilter() tests if the document is modified; if yes, it asks the user to
	  * save the document. If the document title contains "Untitled", slotFileSaveAs() gets called to get a save name and path.
	  */
	virtual bool eventFilter(QObject* object, QEvent* event);

	/** Load configuration on starting */
	void loadOnStart();

	// Last opened path for each type
	QString m_lastImageDirName;
	QString m_lastMovieDirName;
	QString m_lastMeasureDirName;

private slots:
	/** Save configuration on exit */
	void saveSettings();

	/// Clean everything before quit
	void purge();

	// General slots
	void slotNewVirtualMeasure(WorkshopMeasure *);

	// Object explorer slots
	void slotOnDoubleClickOnObject(Q3ListViewItem *);
	void slotOnRightClickOnObject(Q3ListViewItem *, const QPoint &, int);
	void slotOnNewObjectSelected(Q3ListViewItem *);
	// Tools slots
	void slotSignalGenerator();
	void slotOnNewVideoAcq();
	void slotSnapShot(QImage * image);
	void slotMovieCapture(char *movie);

	// Viewers slots
	void slotCreateTable();
	void slotCreatePlot2D();
	void slotCreateVideoCaptureView();
	void slotCreateImageView();

	void slotNewMeasureOnTool(int param);
	void slotDelMeasureOnTool(int param);

	/// Add a new file from converter
	void on_pconvertDialog_signalNewMovie(QString);

	// General documents slots
	void slotFileNew();
	void slotFileOpen();
	void slotFileSave();
	void slotFileSaveAs();
	void slotFileClose();
	void slotFilePrint();

	/** Clear components under a tree */
	void slotComponentClear();

	/** exits the application */
	void slotFileQuit();

	/** reverts the last editing action */
	void slotEditUndo();
	/** put the marked text/object into the clipboard and remove
	 * it from the document */
	void slotEditCut();
	/** put the marked text/object into the clipboard*/
	void slotEditCopy();
	/** paste the clipboard into the document*/
	void slotEditPaste();

	/** @brief launch the conversion from image files to MJPEG movie */
	void slotConvertor();

	/** @brief launch the batch processor */
	void slotBatch();

	/** launch the configuration dialog */
	void slotConfigurator();
	/** launch the plugin list editor */
	void slotPluginDialog();

	/** toggle the toolbar*/
	void slotViewToolBar(bool toggle);
	/** toggle the statusbar*/
	void slotViewStatusBar(bool toggle);
	/** shows an about dlg*/
	void slotHelpAbout();
	/** change the status message of the whole statusbar temporary */
	void slotStatusHelpMsg(const QString &text);
	/** gets called when the window menu is activated; recreates the window menu with all opened window titles. */
	void windowMenuAboutToShow();
	/** activates the MDI child widget when it gets selected from the window menu. */
	void windowMenuActivated( int id );

  private:
	// pointers to general forms
	RandSignalForm  *pRandSignalForm;
	WorkshopList    *pWorkshopList;
	ObjectsExplorer *pObjectsExplorer;

	// Previewers
	PreviewPlot2D *pPVPlot2D;
	PreviewImage  *pPImage;

	// list of data
	// ----------------------------------
	Q3PtrList<WorkshopComponent> *pWComponentList;

	// list of tools
	Q3PtrList<WorkshopTool> *pWToolList;

	// ------------------------------------------------
	//            Initialisation functions
	// ------------------------------------------------
	/// initializes all internal variables to default
	void initVars();
	// initializes all QActions of the application
	void initActions();
	// initMenuBar creates the menu_bar and inserts the menuitems
	void initMenuBar();
	// Creating the toolbars
	void initToolBar();
	// setup the statusbar
	void initStatusBar();
	/// setup the mainview
	void initView();

	/** @brief Discover connected devices belong supported APIs

		Discover connected devices: supported by now:
		V4L2 : Linux only
		OpenCV: depending on your configuration, may support GStreamer, ...
		freenect(Kinect) : if
		OpenNI
		 */
	void discoverDevices();
	// ------------------------------------------------
	//               Handling components list
	// ------------------------------------------------
	void filterComponentName(QString &);

	// ------------------------------------------------
	//               Handling tools list
	// ------------------------------------------------
	void filterToolName(QString &);
	void displayToolsList();

	// ------------------------------------------------
	//                   Utils...
	// ------------------------------------------------
	void createSubmenu(Q3ListViewItem *item, Q3PopupMenu *menu);

	/** file_menu contains all items of the menubar entry "File" */
	Q3PopupMenu *pFileMenu;
	/** edit_menu contains all items of the menubar entry "Edit" */
	Q3PopupMenu *pEditMenu;
	/** view_menu contains all items of the menubar entry "View" */
	Q3PopupMenu *pViewMenu;
	/** view_menu contains all items of the menubar entry "Help" */
	Q3PopupMenu *pHelpMenu;
	/** the window menu */
	Q3PopupMenu *pWindowMenu;

	// SISELL
	// the TOOLS menu contains all tools
	Q3PopupMenu *pToolsMenu;
	Q3PopupMenu *pImagesTools;
	Q3PopupMenu *pMeasuresTools;
	Q3PopupMenu *pVideosTools;

	Q3ToolBar *fileToolbar;

	/** pWorkspace is the MDI frame widget that handles MDI child widgets. Inititalized in
	  * initView()
	  */
	QWorkspace *pWorkspace;
	/** the printer instance */
	QPrinter *printer;
	/** a counter that gets increased each time the user creates a new document with "File"->"New" */
	int untitledSnapShotCount;

	// TOOLS actions
	QAction *measureGen;
	QAction *videoAcquire;
	QAction *snapshot;

	QAction *fileNew;
	QAction *fileOpen;
	QAction *fileSave;
	QAction *fileSaveAs;
	QAction *fileClose;
	QAction *filePrint;
	QAction *fileQuit;

	QAction *editUndo;
	QAction *editCut;
	QAction *editCopy;
	QAction *editPaste;

	QAction *viewToolBar;
	QAction *viewStatusBar;

	QAction *convertAction;
	QAction *batchAction;
	QAction *configAction;
	QAction *pluginAction;


	QActionGroup *windowAction;
	QAction *windowTile;
	QAction *windowCascade;

	QAction *helpAboutApp;

	// -----------------------------
	// 		internal variables
	// -----------------------------
	bool saveSettingsImmediatly;

	float defaultFPS;
	int untitledMovieCount;
};
#endif
