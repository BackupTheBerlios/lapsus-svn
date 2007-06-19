/***************************************************************************
 *   Copyright (C) 2007 by Jakub Schmidtke                                 *
 *   sjakub@users.berlios.de                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.           *
 *                                                                         *
 *   Parts of this code were taken from:                                   *
 *   K3b by Sebastian Trueg <trueg@k3b.org>                                *
 *                                                                         *
 ***************************************************************************/

#ifndef LAPSUS_OSD_H_
#define LAPSUS_OSD_H_

#include <kconfig.h>
#include <qwidget.h>
#include <qpixmap.h>

class QPaintEvent;
class QMouseEvent;
class KConfigBase;

/**
 * An OSD displaying a text and a progress bar.
 *
 * Insprired by Amarok's OSD (I also took a bit of their code. :)
 */
class LapsusOSD : public QWidget
{
	Q_OBJECT

	public:
		LapsusOSD (KConfig *cfg, QWidget* parent = 0, const char* name = 0 );
		~LapsusOSD();

		int screen() const { return _screen; }
		const QPoint& position() const { return _position; }

		void enableDBus();
		
		QColor getForeground();
		QColor getBackground();
		QColor getDefaultForeground();
		QColor getDefaultBackground();
		double getTimeout();
		bool getUseCustomColors();
		
		void resetConfig();
		void saveConfig();
		
	public slots:
		void setScreen( int );
		void setText( const QString& );
		void setProgressText(uint, const QString&);

		/**
		* The position refers to one of the corners of the widget
		* regarding on the value of the x and y coordinate.
		* If for example the x coordinate is bigger than half the screen
		* width it refers to the left edge of the widget.
		*/
		void setPosition( const QPoint& );

		void setDraggingEnabled(bool drag);
		
		void show();
		
		void setTimeout(double nVal);
		void setUseCustom(bool nVal);
		void setForeground(const QColor &nVal);
		void setBackground(const QColor &nVal);
		
	protected:
		void paintEvent( QPaintEvent* );
		void mousePressEvent( QMouseEvent* );
		void mouseReleaseEvent( QMouseEvent* );
		void mouseMoveEvent( QMouseEvent* );
		void renderOSD();
		void reposition( QSize size = QSize() );
		void timerEvent( QTimerEvent * );

	protected slots:
		void refresh();
		virtual void dbusFeatureUpdate(const QString &id, const QString &val, bool isNotif);
		
	private:
		/**
		* Ensure that the position is inside m_screen
		*/
		QPoint fixupPosition( const QPoint& p );
		static const int s_outerMargin = 15;
		
		bool featureProcRange(const QString &val, const QStringList &args, uint *proc);
		void loadConfig();
		
		KConfig* _cfg;
		QPixmap _osdBuffer;
		bool _dirty;
		QString _text;
		bool _dragging;
		QPoint _dragOffset;
		int _screen;
		QPoint _position;
		bool _draggingEnabled;
		bool _progressVisible;
		uint _progressProc;
		int _osdTimer;
		double _timeout;
		bool _useCustomColors;
		QColor _foreground;
		QColor _background;
};

#endif
