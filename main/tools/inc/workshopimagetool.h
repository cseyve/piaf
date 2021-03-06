/***************************************************************************
	workshopimagetool.h  -  still image display/processing tool for Piaf
							 -------------------
	begin                : Wed Nov 13 10:07:22 CET 2002
	copyright            : (C) 2002 by Christophe Seyve
	email                : cseyve@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef WORKSHOPIMAGETOOL_H
#define WORKSHOPIMAGETOOL_H

#include "piaf-common.h"
#include "swopencv.h"

// include files for Qt
#include <qwidget.h>
#include <qpushbutton.h>

#include <qimage.h>
#include <qpainter.h>

#include <q3vbox.h>
#include <qslider.h>
#include <qlabel.h>
#include <qtimer.h>
#include <qpushbutton.h>
#include <q3buttongroup.h>
#include <QAction>
#include <QMenu>
#include <q3toolbar.h>
#include <q3ptrlist.h>
#include <qworkspace.h>


#include <q3listview.h>
//Added by qt3to4:
#include <QCloseEvent>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QMouseEvent>

// Sisell Workshop include files
#include "workshoptool.h"
#include "previewimage.h"
#include "SwFilters.h"

#if _USE_FFMPEG_ENCODER
#define MovieEncoder FFMpegEncoder
#include "FFMpegEncoder.h"
#else
#define MovieEncoder OpenCVEncoder
#include "OpenCVEncoder.h"
#endif


/** \brief Workshop image display and processing tool

	Enables to display (zoom/move) image, plus advanced options such as snapshot
	and plugin manager handling.

	\author Christophe SEYVE \mail cseyve@free.fr
*/
class WorkshopImageTool : public WorkshopTool
{
  Q_OBJECT

  public:

	WorkshopImageTool(WorkshopImage *iv,
					  QWidget *p_parent, const char* p_name, Qt::WidgetAttribute wflags);
	WorkshopImageTool(WorkshopImage *iv,
						QWidget *p_parent, bool showSnap, bool showRec,
						const char* p_name, Qt::WidgetAttribute wflags);
	/** Destructor for the main view */
	~WorkshopImageTool();

	void setWorkspace(QWorkspace * wsp);
	//void update(WorkshopVideoCaptureView* pSender);

	/** contains the implementation for printing functionality and gets called by MultiVideoApp::slotFilePrint() */
	void print(QPrinter *pPrinter);

	/** @brief Set title of window */
	void setTitle(QString ptitle) {
		m_title = ptitle;
		pWin->setCaption(m_title);
	};

	/** @brief Change the zoom factor */
	void setZoom(int zoomscale, int xcenter, int ycenter);

	/** @brief sets the base image */
	void setWorkshopImage(WorkshopImage * iv);

	/** @brief Set overlay rect */
	void setOverlayRect(QRect overlayRect, QColor col);

	ImageWidget * imageView() { return mpViewWidget; };

	/** Set default directories for images and movies */
	void setDefaultDirectories(const QString & image_dir, const QString & movie_dir) {
		imageDir = image_dir;
		movieDir = movie_dir;
	};

protected:
	virtual void closeEvent(QCloseEvent*);


// -------------------- VIDEO SPECIFIC SECTION ---------------------
public:
	tBoxSize getImageSize();
	//virtual void paintEvent(QPaintEvent *);
	int loadFilterManager();

private:
	void setViewSize();
	void setToolbar();
	void changeViewSize();

private:
	/// Title
	QString m_title;

	/// Plugin/filter manager
	SwFilterManager * filterManager;

	/// Input image
	WorkshopImage * m_pWorkshopImage;

	// default image directory
	QString imageDir;
	// default movie directory
	QString movieDir;

	QWidget * pParent;
	QWorkspace * pWorkspace;

	bool ShowSnapbutton;
	bool ShowRecbutton;
	QPushButton * bRecord;

	/// initializes tool properties
	void initTool();
	void init(); // RAZ buffers
	int m_colorMode;
	unsigned char typeOfView;
	tBoxSize viewSize;
	tBoxSize imageSize;

	int viewPixel;
	int imagePixel;

	QImage OrigImgRGB;	///< Original image (without processing)
	QImage ProcImgRGB;	///< Processed image (after processing)


	/// Ready-to-use QImage object for RGB image
	QImage ImgRGB;

	Q3VBox *vBox;
	Q3HBox *hBox;
	ImageWidget * mpViewWidget;


	// View selection buttons
	QPushButton * aMenuView;
	QMenu * viewMenu;
	QMenu * colorMenu;

	// actions for view mode selection
	QAction * actZoomInView;
	QAction * actZoomFitView;
	QAction * actZoomOutView;
	QAction * actMoveView;
	void updateToolbar();

	// Color menu widgets
	QPushButton * aMenuColor;
	QAction * actColorGrey;
	QAction * actColorIndexed;
	QAction * actColorGreyInverted;
	QAction * actColorThermicBlack2Red;
	QAction * actColorThermicBlue2Red;

	void updateColorMenu();

	// export button
	QPushButton * bExport;

	// image parameters
	int ZoomScale;
	int xZoomOrigine; // always in original image coordinates !!!
	int yZoomOrigine;
	int xZoomCenter;
	int yZoomCenter;
	int moveDx;
	int moveDy;
	void CalculateZoomWindow();
	void WindowView2Absolute(int Vx, int Vy, int * Ax, int * Ay);
	void Absolute2View(int Ax, int Ay, int * Vx, int * Vy);

	Q3ToolBar * toolBar;
	QMenuBar * menuBar;

	void refreshDisplay();

  protected slots:
	void slotFilters();
	// change size
	//void slotSetSize( const QString & str );
	/////////////////////
	void slotSaveImage();
	//////////////////////

	// view slots
	void slotMenuView();
	void slotZoomInMode();
	void slotZoomOutMode();
	void slotZoomInOnce();
	void slotZoomOutOnce();
	void slotZoomFit();
	void slotMoveMode();

	// color picker
	void slotColorPickerMode();

	// Color mode slotc
	void slotColorGreyMode();
	void slotColorGreyInvertMode();
	void slotColorThermicBlackToRedMode();
	void slotColorThermicBlueToRedMode();
	void slotColorIndexedMode();

	void slot_mpViewWidget_signalPicker(QRgb colorRGB, int colorGrey, QPoint pt);

	// mask slots
	void slotMaskTools(int id);
	void slotSelMask();
	void slotAddMask();
	void slotClearMask();
	void slotDelSelectedMask();
	void slotMenuMask();

	/// process again image with filters
	void slotUpdateImage();
	/// update image view, eg with zooming
	void slotUpdateView();

	void mousePressEvent( QMouseEvent *e);
	void mouseReleaseEvent( QMouseEvent *e);
	void mouseMoveEvent( QMouseEvent *e);
	void resizeEvent ( QResizeEvent * e);

	// record
	void slotRecord();


  private:
	void onLButtonDown(Qt::ButtonState nFlags, const QPoint point);
	void onLButtonUp(Qt::ButtonState nFlags, const QPoint point);
	void onRButtonDown(Qt::ButtonState nFlags, const QPoint point);
	void onRButtonUp(Qt::ButtonState nFlags, const QPoint point);
	// drawing rectangle function
	void onMouseMove(Qt::ButtonState nFlags, const QPoint point);
	bool selectPressed;

	int myStartX;
	int myStartY;
	int myStopX;
	int myStopY;

	unsigned char mToolMode;	/// Display tool mode
	QRect * getMaskFromPos(int x, int y);

	// MASK MANAGEMENT
	/// mask for image
	unsigned char * mask;
	// Area selection buttons
	Q3HButtonGroup * toolGroup;
	QPushButton * aMenuMask;
	QMenu * maskMenu;

	QAction * actSelMask;
	QAction * actAddMask;
	QAction * actDelSelectedMask;
	QAction * actAllMask;
	QRect * selectedMask;

private:
	bool showMask;



private:

	QImage savedImage;

	// ---- VIDEO ENCODER SECTION -----

private:
	/** Encoder for exporting images */
	MovieEncoder * mpegEncoder;

	bool record;
	int recordNum;
	/** start a new record */
	void startRecording();
	/** stop current record */
	void stopRecording();

	/** returns true if recording */
	bool isRecording();

	char movieFile[512];

signals:
	void ImageSaved(QImage *);
	void VideoSaved(char *);

	/// A pixel has been picked
	void signalColorPicker(QPoint, QRgb);
};

#endif
