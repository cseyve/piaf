/***************************************************************************
 *      Main image display in middle of GUI
 *
 *  Sun Aug 16 19:32:41 2009
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
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef MAINIMAGEWIDGET_H
#define MAINIMAGEWIDGET_H

#include "imageinfo.h"
#include <QtGui/QWidget>

namespace Ui {
	class MainImageWidget;
}

class FilterSequencer;

/** @brief Main image display with toolbars for : image control */

class MainImageWidget : public QWidget {
	Q_OBJECT
	Q_DISABLE_COPY(MainImageWidget)
public:
	explicit MainImageWidget(QWidget *parent = 0);
	virtual ~MainImageWidget();

	/** @brief Set the background image */
	void setImageFile(QString imagePath, t_image_info_struct * pinfo = NULL);
	/** @brief Zoom on a part of input image at a specified scale */
	void zoomOn(int unscaled_x, int unscaled_y, int scale);
	/** @brief crop absolute part of image for display */
	void cropAbsolute(int crop_x, int crop_, int scale);

	/** @brief Set pointer to filter sequencer */
	void setFilterSequencer(FilterSequencer *);

protected:
	virtual void changeEvent(QEvent *e);

	QRect m_displayRect;
	QImage m_fullImage;
	QImage m_displayImage;

	FilterSequencer * mpFilterSequencer;
	int m_zoom_scale;
	QPoint m_lastClick;
	QRect m_cropRect;
	QRect m_lastCrop;

	bool m_mouse_has_moved;
private slots:
	// Zoom options
	void on_zoomButton_toggled(bool checked);

	void on_globalImageLabel_signalMousePressEvent(QMouseEvent * e);
	void on_globalImageLabel_signalMouseReleaseEvent(QMouseEvent * e);
	void on_globalImageLabel_signalMouseMoveEvent(QMouseEvent * e);
	void on_globalImageLabel_signalWheelEvent( QWheelEvent * e );

	void on_actionZoomIn_activated( );

	void slotUpdateImage();
private:
	Ui::MainImageWidget *m_ui;
signals:
	/// Signal to tell where the crop window is located
	void signalCropRect(QRect);
};

#endif // MAINIMAGEWIDGET_H
