/***************************************************************************
 *            qimagedisplay.cpp
 *
 *  Mon Jun 16 23:47:41 2008
 *  Copyright  2007  Christophe Seyve 
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
 
#include "qimagedisplay.h"
#include <qevent.h>
#include <stdio.h>
#include <QPainter>

unsigned char g_debug_QImageDisplay = 0;

QImageDisplay::QImageDisplay(QWidget * l_parent)
	: QLabel(l_parent) {
	setMouseTracking(true);

	setStyleSheet("background-color: rgb(0,0,255);");
}

/** @brief Overloaded paintEvent */
void QImageDisplay::paintEvent( QPaintEvent * )
{
	QPainter p(this);
	int xoff = 0, yoff = 0;
	if(pixmap() && !pixmap()->isNull())
	{
		xoff = ( width() - pixmap()->width() )/2;
		yoff = ( height() - pixmap()->height() )/2;
		QImage dispImage = pixmap()->toImage();
		p.drawImage(xoff, yoff, dispImage.copy());
//		fprintf(stderr, "QImageDisplay::%s:%d : pixmap()=%p => %dx%d\n",
//				__func__, __LINE__, pixmap(),
//				dispImage.width(), dispImage.height());
	}

	if(!m_overlayRect.isNull()) {
		//	QColor m_overlayColor;
		p.setPen(m_overlayColor);
		p.drawRect(QRect(m_overlayRect.x()+xoff, m_overlayRect.y()+yoff,
						 m_overlayRect.width(), m_overlayRect.height()));
	}

	p.end();
}

void QImageDisplay::mouseDoubleClickEvent ( QMouseEvent * e)
{
	if(g_debug_QImageDisplay) {
		fprintf(stderr, "QImageDisplay::%s:%d : e=%p\n", __func__, __LINE__, e);
	}
	emit signalMouseDoubleClickEvent(e);
}

void QImageDisplay::mousePressEvent(QMouseEvent * e)
{
	if(g_debug_QImageDisplay) {
		fprintf(stderr, "QImageDisplay::%s:%d : e=%p\n", __func__, __LINE__, e);
	}
	emit signalMousePressEvent(e);
}

void QImageDisplay::mouseReleaseEvent(QMouseEvent * e)
{
	if(g_debug_QImageDisplay) {
		fprintf(stderr, "QImageDisplay::%s:%d : e=%p\n", __func__, __LINE__, e);
	}
	emit signalMouseReleaseEvent(e);
}

void QImageDisplay::mouseMoveEvent(QMouseEvent * e)
{
	if(g_debug_QImageDisplay) {
		fprintf(stderr, "QImageDisplay::%s:%d : e=%p : %d,%d\n", __func__, __LINE__, e,
				e?e->pos().x():-1,
				e?e->pos().y():-1
				);
	}
	emit signalMouseMoveEvent(e);
}
void QImageDisplay::focusInEvent ( QFocusEvent * event )
{
	if(g_debug_QImageDisplay) {
		fprintf(stderr, "QImageDisplay::%s:%d : e=%p\n", __func__, __LINE__, event);
	}
}

void QImageDisplay::wheelEvent(QWheelEvent * e)
{
	if(e) {
		int numDegrees = e->delta() / 8;
		int numSteps = numDegrees / 15;

		if(g_debug_QImageDisplay) {
			fprintf(stderr, "QImageDisplay::%s:%d : Wheel event e=%p delta=%d\n",
					__func__, __LINE__, e, e->delta());
			fprintf(stderr, "\tDeplacement :\t%d steps = %d degrees\n",
					numSteps, numDegrees );
			fprintf(stderr, "\tOrientation :\t");
			switch(e->orientation()) {
			case Qt::Horizontal:
				fprintf(stderr, "Qt::Horizontal\n");
				break;
			case Qt::Vertical:
				fprintf(stderr, "Qt::Vertical\n");
				break;
			default:
				fprintf(stderr, "Unknown\n");
				break;
			}
		}
	}
	
	emit signalWheelEvent(e);
}
