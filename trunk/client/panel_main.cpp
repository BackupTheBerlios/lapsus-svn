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

#include "lapsus_dbus.h"

// 1.5 sec
#define OSD_TIMEOUT_MS			2000

LapsusPanelMain::LapsusPanelMain(QWidget *parent,
			Qt::Orientation orientation):
	QWidget( parent ), _cfg("lapsusrc"),
	_orientation( orientation ), _osd(0), _osdTimer(0),
	_confDlg(0)
{
	_layout = new FlowLayout(this, _orientation);
	_layout->setSpacing(4);

	setBackgroundMode(X11ParentRelative);

	_actions = new KActionCollection( this );

	loadConfig();

	int added = 0;

	// TODO - cleanup following mess ;)

	for ( QStringList::ConstIterator it = _panelEntries.begin();
		it != _panelEntries.end(); ++it )
	{
		LapsusPanelWidget *widget;
		
		widget = LapsusPanelVolSlider::newPanelWidget(
				*it, _orientation, this, &_cfg);
		
		if (!widget)
			widget = LapsusPanelSlider::newPanelWidget(
				*it, _orientation, this, &_cfg);

		if (!widget)
			widget = LapsusPanelButton::newPanelWidget(
				*it, _orientation, this, &_cfg);

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
		bool addedOK = false;
		
		if (!( KToggleAction* )_actions->action(str))
		{
			addedOK = LapsusActionButton::addNewActionButton(str, &_cfg, _actions);
		}
	}

	connect(LapsusDBus::get(),
		SIGNAL(dbusFeatureUpdate(const QString &, const QString &, bool)),
		this,
		SLOT(dbusFeatureUpdate(const QString &, const QString &, bool)));

	if (added < 1)
	{
		LapsusPanelWidget *widget = new LapsusPanelDefault(
						_orientation, this);

		connect(widget, SIGNAL(rightButtonPressed()),
			this, SLOT(showContextMenu()));

		_layout->add(widget);

		widget->show();
	}
}

LapsusPanelMain::~LapsusPanelMain()
{
	if (_confDlg) delete _confDlg;
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

	if (LapsusDBus::get()->isValid() && doAuto )
	{
		QStringList fL = LapsusDBus::get()->listFeatures();
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
			
			QString panelID = QString("panel_%1").arg(id);
			QString menuID = QString("menu_%1").arg(id);
			QStringList args = LapsusDBus::get()->getFeatureArgs(id);
			
			if (LapsusVolSlider::supportsArgs(args)
				&& LapsusVolSlider::addConfigEntry(panelID, id, &_cfg))
			{
				pEntries.push_front(panelID);
			}
			else if (LapsusSlider::supportsArgs(args)
				&& LapsusSlider::addConfigEntry(panelID, id, &_cfg))
			{
				pEntries.push_front(panelID);
			}
			else if (LapsusSwitch::supportsArgs(args))
			{
				int idx = id.findRev('.');
				QString fType;
				
				if (idx > 0)
				{
					fType = id.mid(idx+1);
					
					if (fType == LAPSUS_FEAT_BLUETOOTH_ID
						|| fType == LAPSUS_FEAT_WIRELESS_ID)
					{
						if (LapsusSwitch::addConfigEntry(panelID, id, &_cfg))
							pEntries.push_back(panelID);
					}
					else if (fType.startsWith(LAPSUS_FEAT_LED_ID_PREFIX)
						|| fType == LAPSUS_FEAT_TOUCHPAD_ID
						|| fType == "thinklight")
					{
						if (LapsusSwitch::addConfigEntry(menuID, id, &_cfg))
							mEntries.push_back(menuID);
					}
				}
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

void LapsusPanelMain::dbusFeatureUpdate(const QString &id, const QString &val, bool isNotif)
{
	if (!isNotif) return;
	
	if (!_osd) _osd = new LapsusOSD(this);

	QString name = LapsusDBus::get()->getFeatureName(id);

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
	if (_confDlg) delete _confDlg;
	
	_confDlg = new LapsusConfDialog(0, &_cfg);
	_confDlg->exec();
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
