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

#include <kiconloader.h>

#include "panel_main.h"
#include "panel_widget.h"
#include "panel_default.h"
#include "panel_slider.h"
#include "panel_button.h"
#include "action_button.h"

LapsusPanelMain::LapsusPanelMain(QWidget *parent, LapsusDBus *dbus,
			Qt::Orientation orientation):
	QWidget( parent ), _dbus( dbus ), _cfg("lapsusrc"), _orientation( orientation )
{
	_layout = new FlowLayout(this, _orientation);
	_layout->setSpacing(4);

	setBackgroundMode(X11ParentRelative);

	_actions = new KActionCollection( this );

	loadConfig();

	int added = 0;

	for (uint i = 0; i < _panelEntries.size(); ++i)
	{
		LapsusPanelWidget *widget = LapsusPanelWidget::newAppletwidget(
			_panelEntries[i], _orientation, this, _dbus, &_cfg);

		if (widget)
		{
			++added;

			_layout->add(widget);

			connect(widget, SIGNAL(rightButtonPressed()),
				this, SLOT(showContextMenu()));

			widget->show();
		}
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

	for (uint i = 0; i < _menuEntries.size(); ++i)
	{
		if (!( KToggleAction* )_actions->action( _menuEntries[i] ))
		{
			new LapsusActionButton(_menuEntries[i], _dbus, &_cfg, _actions);
		}
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

	if (_dbus->isValid()
		& (!_cfg.hasKey("panel_entries")
			|| !_cfg.hasKey("menu_entries")) )
	{
		QStringList fL = _dbus->listFeatures();
		QStringList pEntries;
		QStringList mEntries;

		// Remove old settings
		_cfg.deleteGroup("applet");

		_cfg.setGroup("applet");

		// It would be nice to have 'applet' group at the beginning
		_cfg.writeEntry("panel_entries", QStringList());
		_cfg.writeEntry("menu_entries", QStringList());

		for (QStringList::Iterator it = fL.begin(); it != fL.end(); ++it)
		{
			QString id = (*it).lower();
			QString grp;
			QStringList args = _dbus->getFeatureArgs(id);

			if (LapsusPanelSlider::supportsArgs(args))
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
				if (id.find("bluetooth") >= 0)
				{
					grp = QString("panel_%1").arg(id);
					pEntries.push_back(grp);
				}
				else if (id.find("wireless") >= 0)
				{
					grp = QString("panel_%1").arg(id);
					pEntries.push_back(grp);
				}
				else if (id.find("_led_") >= 0)
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
	_popMenu->insertTitle( SmallIcon( "laptop" ), "Switches" );

	KActionPtrList list = _actions->actions();

	if (list.size() > 0)
	{
		for(KActionPtrList::iterator it = list.begin(); it != list.end(); ++it)
		{
			(*it)->plug( _popMenu);
		}
	}

	QPoint pos = QCursor::pos();
	_popMenu->popup( pos );
}

void LapsusPanelMain::resizeEvent(QResizeEvent *e)
{
	resize(e->size());
}

void LapsusPanelMain::mousePressEvent( QMouseEvent *e )
{
	if (e->button() == RightButton)
	{
		e->accept();
		showContextMenu();
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
