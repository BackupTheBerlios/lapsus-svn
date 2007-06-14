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

#include <qtabwidget.h>
#include <qlistbox.h>
#include <klocale.h>
#include <kactionselector.h>
#include <kpushbutton.h>

#include "lapsus.h"
#include "lapsus_dbus.h"
#include "lapsus_conf.h"
#include "listbox_vol_slider.h"
#include "listbox_slider.h"
#include "listbox_switch.h"

LapsusConf::LapsusConf(QWidget *parent, KConfig *cfg):
	LapsusConfBase(parent), _osd(0), _cfg(cfg)
{
	connect( tabsConf, SIGNAL( currentChanged(QWidget *) ), this, SLOT( tabChanged(QWidget *) ) );
	
	if (!cfg || !LapsusDBus::get()->isValid()) return;
	
	QListBox *avail = selectPanel->availableListBox();
	QListBox *selected = selectPanel->selectedListBox();
	
	cfg->setGroup(LAPSUS_CONF_MAIN_GROUP);
	
	QStringList fL = LapsusDBus::get()->listFeatures();
	QStringList entries = cfg->readListEntry(LAPSUS_CONF_PANEL_LIST);
	
	for (QStringList::Iterator it = entries.begin(); it != entries.end(); ++it)
	{
		QString confID = (*it);
		LapsusListBoxFeature* feat;
		
		feat = LapsusListBoxVolSlider::createListBoxItem(selected, confID, cfg);
		
		if (!feat) feat = LapsusListBoxSlider::createListBoxItem(selected, confID, cfg);
		if (!feat) feat = LapsusListBoxSwitch::createListBoxItem(selected, confID, cfg);
		
		if (feat != 0 && feat->getFeature())
		{
			fL.remove(feat->getFeature()->getFeatureDBusID());
		}
	}
	
	for (QStringList::Iterator it = fL.begin(); it != fL.end(); ++it)
	{
		QString dbusID = (*it);
		
		if (dbusID.startsWith(LAPSUS_FEAT_INIT_PREFIX ".")
			|| dbusID.startsWith(LAPSUS_FEAT_CONFIG_PREFIX "."))
		{
			continue;
		}
		
		QString confID = QString(LAPSUS_CONF_PANEL_FEAT_PREFIX "%1").arg(dbusID);
		QStringList args = LapsusDBus::get()->getFeatureArgs(dbusID);
		LapsusListBoxFeature* feat;
		
		feat = LapsusListBoxVolSlider::createListBoxItem(avail, confID, cfg, dbusID, args);
		
		if (!feat) feat = LapsusListBoxSlider::createListBoxItem(avail, confID, cfg, dbusID, args);
		if (!feat) feat = LapsusListBoxSwitch::createListBoxItem(avail, confID, cfg, dbusID, args);
	}
	
	avail = selectMenu->availableListBox();
	selected = selectMenu->selectedListBox();
	
	cfg->setGroup(LAPSUS_CONF_MAIN_GROUP);
	
	fL = LapsusDBus::get()->listFeatures();
	entries = cfg->readListEntry(LAPSUS_CONF_MENU_LIST);
	
	for (QStringList::Iterator it = entries.begin(); it != entries.end(); ++it)
	{
		QString confID = (*it);
		LapsusListBoxFeature* feat = LapsusListBoxSwitch::createListBoxItem(selected, confID, cfg);
		
		if (feat != 0 && feat->getFeature())
		{
			fL.remove(feat->getFeature()->getFeatureDBusID());
		}
	}
	
	for (QStringList::Iterator it = fL.begin(); it != fL.end(); ++it)
	{
		QString dbusID = (*it);
		
		if (dbusID.startsWith(LAPSUS_FEAT_INIT_PREFIX ".")
			|| dbusID.startsWith(LAPSUS_FEAT_CONFIG_PREFIX "."))
		{
			continue;
		}
		
		QString confID = QString(LAPSUS_CONF_PANEL_FEAT_PREFIX "%1").arg(dbusID);
		QStringList args = LapsusDBus::get()->getFeatureArgs(dbusID);
		
		LapsusListBoxSwitch::createListBoxItem(avail, confID, cfg, dbusID, args);
	}
	
	connect (btOK, SIGNAL(clicked()),
		this, SLOT(confOKClicked()));
}

LapsusConf::~LapsusConf()
{
}

void LapsusConf::tabChanged(QWidget *tab)
{
	if (tab == OSDPage)
	{
		if (!_osd)
		{
			_osd = new LapsusOSD(this);
			_osd->setText(i18n("Drag OSD to desired position"));
			_osd->setDraggingEnabled(true);
		}
	
		_osd->show();
	}
	else
	{
		if (_osd) _osd->hide();
	}
}

void LapsusConf::confOKClicked()
{
	if (!_cfg) return;
	
	int i, c;
	QListBox* selected = selectPanel->selectedListBox();
	QStringList entries;
	
	c = selected->count();
	
	_cfg->deleteGroup(LAPSUS_CONF_MAIN_GROUP);
	_cfg->setGroup(LAPSUS_CONF_MAIN_GROUP);
	_cfg->writeEntry(LAPSUS_CONF_AUTODETECT, LAPSUS_CONF_FALSE);
	
	for (i = 0; i < c; ++i)
	{
		LapsusListBoxFeature* feat = (LapsusListBoxFeature*) selected->item(i);
		
		if (feat && feat->getFeature())
		{
			feat->getFeature()->saveFeature();
			entries.append(feat->getFeature()->getFeatureConfID());
		}
	}
	
	_cfg->setGroup(LAPSUS_CONF_MAIN_GROUP);
	_cfg->writeEntry(LAPSUS_CONF_PANEL_LIST, entries);
	
	selected = selectMenu->selectedListBox();
	entries.clear();
	
	c = selected->count();
	
	for (i = 0; i < c; ++i)
	{
		LapsusListBoxFeature* feat = (LapsusListBoxFeature*) selected->item(i);
		
		if (feat && feat->getFeature())
		{
			feat->getFeature()->saveFeature();
			entries.append(feat->getFeature()->getFeatureConfID());
		}
	}
	
	_cfg->setGroup(LAPSUS_CONF_MAIN_GROUP);
	_cfg->writeEntry(LAPSUS_CONF_MENU_LIST, entries);
	
	_cfg->sync();
	
	emit finished(true);
}

void LapsusConf::confCancelClicked()
{
	emit finished(false);
}
