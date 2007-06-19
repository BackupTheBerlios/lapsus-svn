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

#include "osd.h"
#include "lapsus_dbus.h"

#include <kwin.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <kcursor.h>
#include <kconfig.h>
#include <klocale.h>
#include <kpopupmenu.h>

#include <qpixmap.h>
#include <qpainter.h>
#include <qapplication.h>

#include <X11/Xlib.h>

#include <kglobalsettings.h>

#define LAPSUS_CONF_OSD_GROUP		"osd"
#define LAPSUS_CONF_OSD_TIMEOUT		"timeout"
#define LAPSUS_CONF_OSD_USE_CUSTOM	"use_custom_colors"
#define LAPSUS_CONF_OSD_FOREGROUND	"foreground"
#define LAPSUS_CONF_OSD_BACKGROUND	"background"
#define LAPSUS_CONF_OSD_POSITION	"position"

LapsusOSD::LapsusOSD(KConfig *cfg, QWidget* parent, const char* name )
	: QWidget( parent, name, WType_TopLevel | WNoAutoErase |
				WStyle_Customize | WX11BypassWM |
				WStyle_StaysOnTop ),
	_cfg(cfg),
	_dirty(true), _dragging(false), _screen(0),
	_draggingEnabled(false),
	_progressVisible(false), _progressProc(0),
	_osdTimer(0)
{
	loadConfig();
	
	setFocusPolicy( NoFocus );
	setBackgroundMode( NoBackground );

	// dummy size
	resize( 20, 20 );

	// make sure we are always visible
	KWin::setOnAllDesktops( winId(), true );

	//connect( kapp, SIGNAL(appearanceChanged()),
	//	this, SLOT(refresh()) );
}

LapsusOSD::~LapsusOSD()
{
}

void LapsusOSD::enableDBus()
{
	connect(LapsusDBus::get(),
		SIGNAL(dbusFeatureUpdate(const QString &, const QString &, bool)),
		this,
		SLOT(dbusFeatureUpdate(const QString &, const QString &, bool)));
}

void LapsusOSD::show()
{
	if( _dirty ) renderOSD();

	QWidget::show();
}

void LapsusOSD::setText( const QString& text )
{
	if( _text != text )
	{
		_text = text;
		_progressVisible = false;
		refresh();
	}
}

void LapsusOSD::setProgressText(uint progressProc, const QString& text)
{
	if( _text != text )
	{
		_text = text;
		_progressVisible = true;
		
		if (progressProc < 100)
			_progressProc = progressProc;
		else
			_progressProc = 100;
		
		refresh();
	}
}

void LapsusOSD::setPosition( const QPoint& p )
{
	_position = p;
	reposition();
}

void LapsusOSD::refresh()
{
	if ( isVisible() ) renderOSD();
	else _dirty = true;
}

void LapsusOSD::renderOSD()
{
	// ----------------------------------------
	// |                                      |
	// |  Icon   Message                      |
	// |                                      |
	// ----------------------------------------

	// calculate needed size
	//if( K3bTheme* theme = k3bappcore->themeManager()->currentTheme() ) {
	if( true )
	{
		QColor cF;
		QColor cB;
		
		if (_useCustomColors)
		{
			cF = _foreground;
			cB = _background;
		}
		else
		{
			cF = getDefaultForeground();
			cB = getDefaultBackground();
		}	

		QPixmap icon = KGlobal::iconLoader()->loadIcon( "laptop", KIcon::NoGroup, 32 );
		int margin = 10;
		int textWidth = fontMetrics().width( _text );

		// do not change the size every time the text changes, just in case we are too small
		//QSize newSize( QMAX( QMAX( 2*margin + icon.width() + margin + textWidth, 100 ), width() ),
		//		QMAX( 2*margin + icon.height(), 2*margin + fontMetrics().height()*2 ) );

		// change every time
		QSize newSize( QMAX( 2*margin + icon.width() + margin + textWidth, 150 ),
				QMAX( 2*margin + icon.height(), 2*margin + fontMetrics().height()*2 ) );

		_osdBuffer.resize( newSize );
		QPainter p( &_osdBuffer );

		//p.setPen( theme->foregroundColor() );
		p.setPen( cF );

		// draw the background and the frame
		QRect thisRect( 0, 0, newSize.width(), newSize.height() );
		//p.fillRect( thisRect, theme->backgroundColor() );
		p.fillRect( thisRect, cB );
		p.drawRect( thisRect );

		// draw the icon
		p.drawPixmap( margin, (newSize.height()-icon.height())/2, icon );

		// draw the text
		QSize textSize = fontMetrics().size( 0, _text );
		int textX = 2*margin + icon.width();
		int textY = (newSize.height() + (fontMetrics().ascent()*5)/6)/2;

		p.drawText(textX, textY, _text);
		
		if (_progressVisible)
		{
			// draw the progress
			textY += fontMetrics().descent() + 4;
			QRect progressRect( textX, textY, newSize.width()-textX-margin, newSize.height()-textY-margin );
			p.drawRect( progressRect );
			progressRect.setWidth( _progressProc > 0 ? _progressProc*progressRect.width()/100 : 0 );
			//p.fillRect( progressRect, theme->foregroundColor() );
			p.fillRect( progressRect, cF );
		}

		// reposition the osd
		reposition( newSize );

		_dirty = false;

		update();
	}
}


void LapsusOSD::setScreen( int screen )
{
	const int n = QApplication::desktop()->numScreens();
	_screen = (screen >= n) ? n-1 : screen;
	reposition();
}


void LapsusOSD::reposition( QSize newSize )
{
	if( !newSize.isValid() ) newSize = size();

	QPoint newPos = _position;
	const QRect& screen = QApplication::desktop()->screenGeometry( _screen );

	// now to properly resize if put into one of the corners we interpret the position
	// depending on the quadrant
	int midH = screen.width()/2;
	int midV = screen.height()/2;

	if( newPos.x() > midH ) newPos.rx() -= newSize.width();
	if( newPos.y() > midV ) newPos.ry() -= newSize.height();

	newPos = fixupPosition( newPos );

	// correct for screen position
	newPos += screen.topLeft();

	// ensure we are painted before we move
	if( isVisible() ) paintEvent( 0 );

	// fancy X11 move+resize, reduces visual artifacts
	XMoveResizeWindow( x11Display(), winId(), newPos.x(), newPos.y(), newSize.width(), newSize.height() );
}


void LapsusOSD::paintEvent( QPaintEvent* )
{
	bitBlt( this, 0, 0, &_osdBuffer );
}


void LapsusOSD::mousePressEvent( QMouseEvent* e )
{
	if (_draggingEnabled)
	{
		_dragOffset = e->pos();
		
		if( e->button() == LeftButton && !_dragging )
		{
			grabMouse( KCursor::sizeAllCursor() );
			_dragging = true;
		}
	}
}

void LapsusOSD::setDraggingEnabled(bool drag)
{
	_draggingEnabled = drag;
}

void LapsusOSD::mouseReleaseEvent( QMouseEvent* )
{
	if( _dragging )
	{
		_dragging = false;
		releaseMouse();
	}
}

void LapsusOSD::mouseMoveEvent( QMouseEvent* e )
{
	if( _dragging && this == mouseGrabber() )
	{
		// check if the OSD has been dragged out of the current screen
		int currentScreen = QApplication::desktop()->screenNumber( e->globalPos() );

		if( currentScreen != -1 ) _screen = currentScreen;

		const QRect& screen = QApplication::desktop()->screenGeometry( _screen );

		// make sure the position is valid
		_position = fixupPosition( e->globalPos() - _dragOffset - screen.topLeft() );

		// move us to the new position
		move( _position );

		// fix the position
		int midH = screen.width()/2;
		int midV = screen.height()/2;

		if( _position.x() + width() > midH ) _position.rx() += width();
		if( _position.y() + height() > midV ) _position.ry() += height();
	}
}

QPoint LapsusOSD::fixupPosition( const QPoint& pp )
{
	QPoint p(pp);
	const QRect& screen = QApplication::desktop()->screenGeometry( _screen );
	int maxY = screen.height() - height() - s_outerMargin;
	int maxX = screen.width() - width() - s_outerMargin;

	if( p.y() < s_outerMargin ) p.ry() = s_outerMargin;
	else if( p.y() > maxY ) p.ry() = maxY;

	if( p.x() < s_outerMargin )
		p.rx() = s_outerMargin;
	else if( p.x() > maxX )
		p.rx() = screen.width() - s_outerMargin - width();

	p += screen.topLeft();

	return p;
}

void LapsusOSD::dbusFeatureUpdate(const QString &id, const QString &val, bool isNotif)
{
	if (!isNotif) return;
	
	QString name = LapsusDBus::get()->getFeatureName(id);

	if (name.length() < 1) return;

	if (_osdTimer)
	{
		killTimer(_osdTimer);
		_osdTimer = 0;
	}

	uint proc;
	
	if (featureProcRange(val, LapsusDBus::get()->getFeatureArgs(id), &proc))
	{
		setProgressText(proc, QString("%1: %2").arg(name).arg(val.upper()));
	}
	else
	{
		setText(QString("%1: %2").arg(name).arg(val.upper()));
	}
	
	show();

	// TODO This should be kept in configuration
	_osdTimer = startTimer((uint) (_timeout*1000.0));
}

bool LapsusOSD::featureProcRange(const QString &val, const QStringList &args, uint *proc)
{
	int vMin, vMax;
	int cur = 0;
	
	QStringList vals = QStringList::split(',', val);
	
	bool ok = false;
	
	for (QStringList::ConstIterator it = vals.begin(); it != vals.end(); ++it)
	{
		cur = (*it).toInt(&ok);
		
		if (ok) break;
	}
	
	if (!ok) return false;
	
	for (QStringList::ConstIterator it = args.begin(); it != args.end(); ++it)
	{
		QStringList list = QStringList::split(':', *it);
		
		if (list.size() == 2)
		{
			vMin = list[0].toInt(&ok);
			
			if (ok) vMax = list[1].toInt(&ok);
			
			if (ok && vMin < vMax && vMin <= cur && cur <= vMax )
			{
				cur -= vMin;
				vMax -= vMin;
				
				*proc = (uint) (100*cur)/vMax;
				
				return true;
			}
		}
	}
	
	return false;
}

void LapsusOSD::timerEvent(QTimerEvent * e)
{
	if (_osdTimer && e->timerId() == _osdTimer)
	{
		killTimer(_osdTimer);
		_osdTimer = 0;
		hide();
	}
}

void LapsusOSD::loadConfig()
{
	resetConfig();
	
	if (!_cfg) return;
	
	_cfg->setGroup(LAPSUS_CONF_OSD_GROUP);
	_timeout = _cfg->readDoubleNumEntry(LAPSUS_CONF_OSD_TIMEOUT, _timeout);
	_useCustomColors = _cfg->readBoolEntry(LAPSUS_CONF_OSD_USE_CUSTOM, _useCustomColors);
	_foreground = _cfg->readColorEntry(LAPSUS_CONF_OSD_FOREGROUND, &_foreground);
	_background = _cfg->readColorEntry(LAPSUS_CONF_OSD_BACKGROUND, &_background);
	_position = _cfg->readPointEntry(LAPSUS_CONF_OSD_POSITION, &_position);
}

void LapsusOSD::setTimeout(double nVal)
{
	_timeout = nVal;
}

void LapsusOSD::setUseCustom(bool nVal)
{
	_useCustomColors = nVal;
	refresh();
}

void LapsusOSD::setForeground(const QColor &nVal)
{
	_foreground = nVal;
	refresh();
}

void LapsusOSD::setBackground(const QColor &nVal)
{
	_background = nVal;
	refresh();
}

void LapsusOSD::saveConfig()
{
	if (!_cfg) return;
	
	_cfg->setGroup(LAPSUS_CONF_OSD_GROUP);
	_cfg->writeEntry(LAPSUS_CONF_OSD_TIMEOUT, _timeout);
	_cfg->writeEntry(LAPSUS_CONF_OSD_USE_CUSTOM, _useCustomColors);
	_cfg->writeEntry(LAPSUS_CONF_OSD_FOREGROUND, _foreground);
	_cfg->writeEntry(LAPSUS_CONF_OSD_BACKGROUND, _background);
	_cfg->writeEntry(LAPSUS_CONF_OSD_POSITION, _position);
}

void LapsusOSD::resetConfig()
{
	// Default values:
	_position = QPoint(s_outerMargin, s_outerMargin);
	_timeout = 1.5;
	_useCustomColors = false;
	_foreground = getDefaultForeground();
	_background = getDefaultBackground();
}

QColor LapsusOSD::getForeground()
{
	return _foreground;
}

QColor LapsusOSD::getBackground()
{
	return _background;
}

QColor LapsusOSD::getDefaultForeground()
{
	return KGlobalSettings::activeTextColor();
}

QColor LapsusOSD::getDefaultBackground()
{
	return KGlobalSettings::activeTitleColor();
}

double LapsusOSD::getTimeout()
{
	return _timeout;
}

bool LapsusOSD::getUseCustomColors()
{
	return _useCustomColors;
}
