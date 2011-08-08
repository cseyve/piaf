/***************************************************************************
	vidacqsettingswindow.h  -  V4L2/OpenCV camera acquisition control window
							 -------------------
	begin                : Thu Aug 04 2011
	copyright            : (C) 2011 by Christophe Seyve (CSE)
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

#ifndef VIDACQSETTINGSWINDOW_H
#define VIDACQSETTINGSWINDOW_H

#include <QMainWindow>
#include "virtualdeviceacquisition.h"
#include "videocapture.h"

namespace Ui {
    class VidAcqSettingsWindow;
}

class VidAcqSettingsWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit VidAcqSettingsWindow(QWidget *parent = 0);
    ~VidAcqSettingsWindow();

	/** @brief Set the pointer to video capture doc */
	void setVideoCaptureDoc(VideoCaptureDoc * doc);

protected:
    void changeEvent(QEvent *e);


private slots:

	/// Disconnection of device to prevent from continuing to use it
	void on_mpVideoCaptureDoc_documentClosed(VideoCaptureDoc * doc);

	void on_exposureModeComboBox_currentIndexChanged(int index);

	/** @brief Get video props from device and update display */
	void updateVideoProperties();

	void on_brightnessSlider_valueChanged(int value);
	void on_contrastSlider_valueChanged(int value);
	void on_hueSlider_valueChanged(int value);
	void on_saturationSlider_valueChanged(int value);
	void on_wbSlider_valueChanged(int value);

	void on_exposureSlider_valueChanged(int value);
	void on_gainSlider_valueChanged(int value);
	void on_brightnessModeComboBox_currentIndexChanged(int index);

	void on_wbModeComboBox_currentIndexChanged(int index);

	void on_wbDoButton_clicked();

	void on_resetButton_clicked();	/*!< Reset device to piaf default */

private:

	/** @brief Send current video props to device */
	void sendVideoProperties();

	Ui::VidAcqSettingsWindow *ui;

	VideoCaptureDoc * mpVideoCaptureDoc;

	t_video_properties m_video_properties;
};

#endif // VIDACQSETTINGSWINDOW_H