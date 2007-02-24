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
 ***************************************************************************/

// QT
#include <qcursor.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwmatrix.h>

// KDE
#include <kglobal.h>
#include <kiconloader.h>
#include <kstandarddirs.h>

#include "applet.h"
#include "lapsus.h"
#include "switch.h"

extern "C"
{
	KDE_EXPORT KPanelApplet* init(QWidget *parent, const QString& configFile)
	{
		KGlobal::locale()->insertCatalogue("lapsus");
		return new LapsusApplet(configFile, KPanelApplet::Normal,
					parent, "lapsusapplet");
	}
}

static const QColor highColor = "#FFFF00";
static const QColor lowColor =  "#707000";
static const QColor backColor = "#000000";

LapsusApplet::LapsusApplet( const QString& configFile, Type t, QWidget *parent, const char *name )
	: KPanelApplet( configFile, t, KPanelApplet::Preferences , parent, name ),
	_pref(0), _slider(0), _iconLabel(0)
{

	if ( orientation() == Qt::Vertical )
	{
		_layout = new QHBoxLayout( this );
		_layout->setAlignment(Qt::AlignCenter);
	}
	else
	{
		_layout = new QVBoxLayout( this );
		_layout->setAlignment(Qt::AlignCenter);
	}

	_colors.high = highColor;
	_colors.low = lowColor;
	_colors.back = backColor;

	KGlobal::dirs()->addResourceType( "appicon", KStandardDirs::kde_default("data") + "lapsus/pics" );
	loadConfig();

	_actions = new KActionCollection( this );

	if (_dbus.isValid() && _dbus.hasSwitches())
	{
		QStringList list = _dbus.listSwitches();

		for (QStringList::iterator it = list.begin(); it != list.end(); ++it)
		{
			QString name = (*it);

			if (!( KToggleAction* )_actions->action( name ))
			{
				LapsusSwitch *sw = new LapsusSwitch(name, 0, _actions);

				sw->setChecked(_dbus.getSwitch(name));

				connect(sw, SIGNAL(toggle(const QString &, bool)),
					this, SLOT(toggleSwitch(const QString &, bool)));
			}
		}
	}

	connect (&_dbus, SIGNAL(switchChanged(const QString &, bool)),
		this, SLOT(dbusSwitchChanged(const QString &, bool)));

	connect (&_dbus, SIGNAL(backlightChanged(uint)),
		this, SLOT(dbusBacklightChanged(uint)));

	positionChange(position());

	installEventFilter( this );
}

LapsusApplet::~LapsusApplet()
{
}

void LapsusApplet::saveConfig()
{
	KConfig cfg( "lapsusrc" );

	cfg.setGroup("applet");

	cfg.writeEntry( "ColorHigh", _colors.high.name() );
	cfg.writeEntry( "ColorLow", _colors.low.name() );
	cfg.writeEntry( "ColorBack", _colors.back.name() );

	cfg.sync();
}


void LapsusApplet::loadConfig()
{
	KConfig cfg( "lapsusrc", true );

	cfg.setGroup("applet");

	_colors.high = cfg.readColorEntry("ColorHigh", &highColor);
	_colors.low = cfg.readColorEntry("ColorLow", &lowColor);
	_colors.back = cfg.readColorEntry("ColorBack", &backColor);
}

void LapsusApplet::about()
{
//TODO
}

void LapsusApplet::help()
{
//TODO
}

void LapsusApplet::reportBug()
{
//TODO
}

void LapsusApplet::positionChange(Position)
{
	orientationChange( orientation() );
	QResizeEvent e( size(), size() ); // from KPanelApplet::positionChange
	resizeEvent( &e ); // from KPanelApplet::positionChange

	if (_slider || _iconLabel)
	{
		if (_slider)
		{
			_layout->remove(_slider);
			delete _slider;
		}

		if (_iconLabel)
		{
			_layout->remove(_iconLabel);
			delete _iconLabel;
		}
	}

	QPixmap pic = UserIcon("light_bulb");

	if (!pic.isNull() )
	{
		_iconLabel = new QLabel(this);

		// scale icon
		QWMatrix t;
		t = t.scale( 10.0/pic.width(), 10.0/pic.height() );
		_iconLabel->setPixmap( pic.xForm( t ) );
		_iconLabel->resize( 10, 10 );

		_layout->add(_iconLabel);
		_layout->addSpacing( 5 );

		_iconLabel->installEventFilter(this);
	}

	if (orientation() == Qt::Vertical)
	{
		_slider = new KSmallSlider( 0, _dbus.maxBacklight(), 1,
			_dbus.getBacklight(), Qt::Horizontal, this, "Brightness" );
	}
	else
	{
		_slider = new KSmallSlider( 0, _dbus.maxBacklight(), 1,
			_dbus.getBacklight(), Qt::Vertical, this, "Brightness" );
	}

	_slider->installEventFilter(this);
	QToolTip::add( _slider, "LCD Backlight" );

	connect ( _slider, SIGNAL(valueChanged(int)), this, SLOT(sliderValueChanged(int)) );

	_layout->add(_slider);
	_layout->activate();

	loadConfig();
	setColors();

	const QSize panelAppletConstrainedSize = sizeHint();

	_slider->setGeometry( 0, 0, panelAppletConstrainedSize.width(), panelAppletConstrainedSize.height() );
	resize( panelAppletConstrainedSize.width(), panelAppletConstrainedSize.height() );
	_slider->show();
}

void LapsusApplet::sliderValueChanged(int nValue)
{
	if (_dbus.isValid() && _dbus.hasBacklight())
	{
		_dbus.setBacklight(nValue);
	}
}

void LapsusApplet::toggleSwitch(const QString &name, bool nValue)
{
	KToggleAction *ta = ( KToggleAction* )_actions->action( name );

	if (!ta)
		return;

	if (_dbus.isValid() && _dbus.hasSwitches())
	{
		_dbus.setSwitch(name, nValue);
	}
}

void LapsusApplet::dbusSwitchChanged(const QString &name, bool nValue)
{
	KToggleAction *ta = ( KToggleAction* )_actions->action( name );

	if (!ta)
	{
		return;
	}

	if (ta->isChecked() != nValue)
	{
		ta->setChecked(nValue);
	}
}

void LapsusApplet::dbusBacklightChanged(uint nValue)
{
	if (_slider->value() == (int) nValue || _slider->maxValue() < (int) nValue)
		return;

	_slider->setValue( nValue);
}

void LapsusApplet::showContextMenu()
{
	_popMenu = new KPopupMenu( this );

	KActionPtrList list = _actions->actions();

	if (list.size() > 0)
	{
		// TODO:i18n _popMenu->insertTitle( SmallIcon( "laptop" ), i18n("Switches") );
		_popMenu->insertTitle( SmallIcon( "laptop" ), "Switches" );

		for(KActionPtrList::iterator it = list.begin(); it != list.end(); ++it)
		{
			(*it)->plug( _popMenu);
		}
	}

	QPoint pos = QCursor::pos();
	_popMenu->popup( pos );
}

void LapsusApplet::resizeEvent(QResizeEvent *e)
{
	if ( position() == KPanelApplet::pLeft || position() == KPanelApplet::pRight )
	{
		if (_iconLabel)
		{
			if (e->size().width()<32)
			{
				_iconLabel->hide();
			}
			else
			{
				_iconLabel->show();
			}
		}
		if ( _slider )
			_slider->resize(e->size().width(),_slider->height());
	}
	else
	{
		if (_iconLabel)
		{
			if (e->size().height()<32)
			{
				_iconLabel->hide();
			}
			else
			{
				_iconLabel->show();
			}
		}
		if ( _slider )
			_slider->resize(_slider->width(), e->size().height());
	}

	updateGeometry();

	emit updateLayout();
}

void LapsusApplet::updateGeometrySlot()
{
	updateGeometry();
}


QSize LapsusApplet::sizeHint() const
{
	QSize qsz;
	if ( _slider != 0)
	{
		qsz = _slider->sizeHint();
	}
	else
	{
		// During construction of m_mixerWidget or if something goes wrong:
		// Return something that should resemble our former sizeHint().
		qsz = size();
	}
	return qsz;
}

/**
   We need widthForHeight() and heigthForWidth() only because KPanelApplet::updateLayout does relayouts
   using this method. Actually we ignore the passed paramater and just return our preferred size.
*/
int LapsusApplet::widthForHeight(int) const
{
    return sizeHint().width();
}
int LapsusApplet::heightForWidth(int) const
{
    return sizeHint().height();
}


QSizePolicy LapsusApplet::sizePolicy() const
{
	if ( orientation() == Qt::Vertical )
	{
		return QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	}
	else
	{
		return QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
	}
}

void LapsusApplet::preferences()
{
	if ( !_pref )
	{
		_pref = new AppletConfigDialog( this );
		connect(_pref, SIGNAL(finished()), SLOT(preferencesDone()));
		connect(_pref, SIGNAL(applied()), SLOT(applyPreferences()) );

		_pref->setActiveColors(_colors.high     , _colors.low     , _colors.back);
	}

	_pref->show();
	_pref->raise();
}


void LapsusApplet::preferencesDone()
{
	_pref->delayedDestruct();
	_pref = 0;
}

void LapsusApplet::applyPreferences()
{
	if (!_pref)
		return;

	// copy the colors from the prefs dialog
	_pref->activeColors(_colors.high     , _colors.low     , _colors.back);

	if (!_slider)
		return;

	setColors();
	saveConfig();
}

void LapsusApplet::setColors()
{
	setColors( _colors );
}

void LapsusApplet::setColors( const Colors &color )
{
	if ( _slider == 0 )
	{
		return;
	}

	_slider->setColors( color.high, color.low, color.back );
}

/**
 * An event filter for the various QWidgets. We watch for Mouse press Events, so
 * that we can popup the context menu.
 */
bool LapsusApplet::eventFilter( QObject* obj, QEvent* e )
{
	if (e->type() == QEvent::MouseButtonPress)
	{
		QMouseEvent *qme = static_cast<QMouseEvent*>(e);
		if (qme->button() == Qt::RightButton)
		{
			showContextMenu();
			return true;
		}
	}
	// Attention: We don't filter WheelEvents for KSmallSlider, because it handles WheelEvents itself
	else if ( _slider && (e->type() == QEvent::Wheel) && !obj->isA("KSmallSlider") )
	{
		QWheelEvent *qwe = static_cast<QWheelEvent*>(e);
		if (qwe->delta() > 0)
		{
			_slider->moveUp();
		}
		else
		{
			_slider->moveDown();
		}
		return true;
	}

	return QWidget::eventFilter(obj,e);
}
