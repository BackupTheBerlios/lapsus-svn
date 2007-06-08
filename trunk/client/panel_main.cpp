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

#include <qcursor.h>

#include <klocale.h>
#include <kiconloader.h>

#include "lapsus.h"

#include "panel_main.h"
#include "panel_widget.h"
#include "panel_default.h"
#include "panel_slider.h"
#include "panel_vol_slider.h"
#include "panel_button.h"
#include "action_button.h"

// 1.5 sec
#define OSD_TIMEOUT_MS			2000

LapsusPanelMain::LapsusPanelMain(QWidget *parent, LapsusDBus *dbus,
			Qt::Orientation orientation):
	QWidget( parent ), _dbus( dbus ), _cfg("lapsusrc"),
	_orientation( orientation ), _osd(0), _osdTimer(0)
{
	_layout = new FlowLayout(this, _orientation);
	_layout->setSpacing(4);

	setBackgroundMode(X11ParentRelative);

	_actions = new KActionCollection( this );

	loadConfig();

	int added = 0;

	if (_dbus)
	{
		// TODO - cleanup following mess ;)

		for ( QStringList::ConstIterator it = _panelEntries.begin();
			it != _panelEntries.end(); ++it )
		{
			LapsusPanelWidget *widget =
				LapsusPanelWidget::newAppletwidget(
				*it, _orientation, this,
				_dbus, &_cfg);

			if (widget)
			{
				++added;

				_layout->add(widget);

				connect(widget, SIGNAL(rightButtonPressed()),
					this, SLOT(showContextMenu()));

				widget->show();
			}
		}

		for ( QStringList::ConstIterator it = _menuEntries.begin();
			it != _menuEntries.end(); ++it )
		{
			QString str = *it;

			_cfg.setGroup(str);

			if (!_cfg.hasKey("feature_id")) continue;

			QString fId = _cfg.readEntry("feature_id");

			if (fId.length() < 1
				|| dbus->getFeatureName(fId).length() < 1
				|| dbus->getFeatureArgs(fId).size() < 1)
			{
				continue;
			}

			if (!( KToggleAction* )_actions->action(str))
			{
				new LapsusActionButton(str, _dbus, &_cfg, _actions);
			}
		}

		connect(_dbus,
			SIGNAL(featureNotif(const QString &, const QString &)),
			this,
			SLOT(featureNotif(const QString &, const QString &)));

	}

	if (added < 1)
	{
		LapsusPanelWidget *widget = new LapsusPanelDefault(
			"lapsus_panel_default", _orientation, this, _dbus, &_cfg);

		connect(widget, SIGNAL(rightButtonPressed()),
			this, SLOT(showContextMenu()));

		_layout->add(widget);

		widget->show();
	}
}

LapsusPanelMain::~LapsusPanelMain()
{
}

void LapsusPanelMain::saveConfig()
{
	_cfg.setGroup("applet");
	_cfg.sync();
}

void LapsusPanelMain::loadConfig()
{
	_cfg.setGroup("applet");

	bool doAuto = true;

	if (_cfg.hasKey("autodetect"))
	{
		QString str = _cfg.readEntry("autodetect");

		if (str.length() > 0 && str != "true") doAuto = false;
	}

	if (_dbus->isValid() && doAuto )
	{
		QStringList fL = _dbus->listFeatures();
		QStringList pEntries;
		QStringList mEntries;

		// Remove old settings
		_cfg.deleteGroup("applet");

		_cfg.setGroup("applet");

		// So it's easier to change if user wants to set it to false :)
		_cfg.writeEntry("autodetect", "true");

		// It would be nice to have 'applet' group at the beginning
		_cfg.writeEntry("panel_entries", QStringList());
		_cfg.writeEntry("menu_entries", QStringList());

		QString sInit = QString("%1.").arg(LAPSUS_FEAT_INIT_PREFIX);
		QString sConf = QString("%1.").arg(LAPSUS_FEAT_CONFIG_PREFIX);
		
		for (QStringList::Iterator it = fL.begin(); it != fL.end(); ++it)
		{
			QString id = (*it).lower();
			
			// We don't add 'init.' or 'config.' entries to panel applet/menu
			if (id.startsWith(sInit) || id.startsWith(sConf))
			{
				continue;
			}
			
			QString grp;
			QStringList args = _dbus->getFeatureArgs(id);

			if (LapsusPanelVolSlider::supportsArgs(args))
			{
				grp = QString("panel_%1").arg(id);
				pEntries.push_front(grp);

				_cfg.deleteGroup(grp);
				_cfg.setGroup(grp);
				_cfg.writeEntry("widget_type", "vol_slider");
				_cfg.writeEntry("feature_id", id);
			}
			else if (LapsusPanelSlider::supportsArgs(args))
			{
				grp = QString("panel_%1").arg(id);
				pEntries.push_front(grp);

				_cfg.deleteGroup(grp);
				_cfg.setGroup(grp);
				_cfg.writeEntry("widget_type", "slider");
				_cfg.writeEntry("feature_id", id);
			}
			else if (LapsusPanelButton::supportsArgs(args))
			{
				int idx = id.findRev('.');
				QString fType;
				
				if (idx < 1)
				{
					continue;
				}
				
				fType = id.mid(idx+1);
				
				if (fType == LAPSUS_FEAT_BLUETOOTH_ID
					|| fType == LAPSUS_FEAT_WIRELESS_ID)
				{
					grp = QString("panel_%1").arg(id);
					pEntries.push_back(grp);
				}
				else if (fType.startsWith(LAPSUS_FEAT_LED_ID_PREFIX))
				{
					grp = QString("menu_%1").arg(id);
					mEntries.push_back(grp);
				}
				else if (fType == "thinklight")
				{
					grp = QString("menu_%1").arg(id);
					mEntries.push_back(grp);
				}
				else if (fType == LAPSUS_FEAT_TOUCHPAD_ID)
				{
					grp = QString("menu_%1").arg(id);
					mEntries.push_back(grp);
				}
				else
				{
					continue;
				}

				_cfg.deleteGroup(grp);
				_cfg.setGroup(grp);
				_cfg.writeEntry("widget_type", "button");
				_cfg.writeEntry("feature_id", id);
			}
		}

		_cfg.setGroup("applet");
		_cfg.writeEntry("panel_entries", pEntries);
		_cfg.writeEntry("menu_entries", mEntries);

		_cfg.sync();
	}

	_cfg.setGroup("applet");
	_panelEntries = _cfg.readListEntry("panel_entries");
	_menuEntries = _cfg.readListEntry("menu_entries");
}

int LapsusPanelMain::widthForHeight(int h) const
{
	return _layout->widthForHeight(h);
}

int LapsusPanelMain::heightForWidth(int w) const
{
	return _layout->heightForWidth(w);
}

QSize LapsusPanelMain::sizeHint() const
{
	return _layout->sizeHint();
}

QSize LapsusPanelMain::minimumSize() const
{
	return _layout->minimumSize();
}

void LapsusPanelMain::showContextMenu()
{
	_popMenu = new KPopupMenu( this );
	_popMenu->insertTitle( SmallIcon( "laptop" ), i18n("Switches") );

	KActionPtrList list = _actions->actions();
	qHeapSort( list );

	if (list.size() > 0)
	{
		for(KActionPtrList::iterator it = list.begin(); it != list.end(); ++it)
		{
			(*it)->plug(_popMenu);
		}
	}

	QPoint pos = QCursor::pos();
	_popMenu->popup( pos );
}

QSizePolicy LapsusPanelMain::sizePolicy() const
{
	return QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
}

void LapsusPanelMain::mousePressEvent( QMouseEvent *e )
{
	if (e->button() == RightButton)
	{
		e->accept();
		showContextMenu();
	}
}

void LapsusPanelMain::featureNotif(const QString &id, const QString &val)
{
	if (!_osd) _osd = new LapsusOSD(this);

	if (!_dbus) return;

	QString name = _dbus->getFeatureName(id);

	if (name.length() < 1) return;

	if (_osdTimer)
	{
		killTimer(_osdTimer);
		_osdTimer = 0;
	}

	_osd->setText(QString("%1: %2").arg(name).arg(val.upper()));
	_osd->show();

	// TODO This should be kept in configuration
	_osdTimer = startTimer(OSD_TIMEOUT_MS);
}

void LapsusPanelMain::timerEvent( QTimerEvent * e)
{
	if (_osdTimer && e->timerId() == _osdTimer)
	{
		killTimer(_osdTimer);
		_osdTimer = 0;

		if (_osd)
		{
			_osd->hide();
		}
	}
}

void LapsusPanelMain::appletPreferences()
{
//TODO
}

/*
void LapsusPanelMain::preferencesDone()
{
	_pref->delayedDestruct();
	_pref = 0;
}

void LapsusPanelMain::applyPreferences()
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
*/
