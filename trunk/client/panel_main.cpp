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

#include "feature_manager.h"

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
		LapsusPanelWidget *widget = LapsusFeatureManager::newPanelWidget(
			&_cfg, *it, _orientation, this, LapsusFeature::ValidDBus);
		
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
			addedOK = LapsusFeatureManager::newActionButton(&_cfg, str, _actions,
				LapsusFeature::ValidDBus);
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
	_cfg.setGroup(LAPSUS_CONF_MAIN_GROUP);
	_cfg.sync();
}

void LapsusPanelMain::loadConfig()
{
	_cfg.setGroup(LAPSUS_CONF_MAIN_GROUP);

	bool doAuto = true;

	if (_cfg.hasKey(LAPSUS_CONF_AUTODETECT))
	{
		QString str = _cfg.readEntry(LAPSUS_CONF_AUTODETECT);

		if (str.length() > 0 && str != LAPSUS_CONF_TRUE) doAuto = false;
	}

	if (LapsusDBus::get()->isActive() && doAuto )
	{
		LapsusFeatureManager::writeAutoConfig(&_cfg);
	}

	_cfg.setGroup(LAPSUS_CONF_MAIN_GROUP);
	_panelEntries = _cfg.readListEntry(LAPSUS_CONF_PANEL_LIST);
	_menuEntries = _cfg.readListEntry(LAPSUS_CONF_MENU_LIST);
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

bool LapsusPanelMain::appletPreferences()
{
	if (_confDlg) delete _confDlg;
	
	_confDlg = new LapsusConfDialog(0, &_cfg);
	_confDlg->exec();
	
	int res = _confDlg->result();
	
	delete _confDlg;
	_confDlg = 0;
	
	return (res != QDialog::Rejected);
}
