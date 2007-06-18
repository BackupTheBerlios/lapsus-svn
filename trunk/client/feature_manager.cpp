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

#include "lapsus.h"
#include "lapsus_dbus.h"
#include "feature_manager.h"

#include "lapsus_switch.h"
#include "lapsus_slider.h"
#include "lapsus_vol_slider.h"

#include "lapsus_conf.h"

QPtrList<LapsusFeature> LapsusFeatureManager::autodetectFeatures(KConfig *cfg, LapsusFeature::Place where)
{
	QPtrList<LapsusFeature> list;
	
	if (!cfg || !LapsusDBus::get()->isActive()) return list;
	if (where == LapsusFeature::PlaceUnknown) return list;
	
	QStringList featList = LapsusDBus::get()->listFeatures();
	
	for (QStringList::Iterator it = featList.begin(); it != featList.end(); ++it)
	{
		QString id = (*it).lower();
		
		// We don't 'detect' any 'init.' or 'config.' entries
		if (id.startsWith(LAPSUS_FEAT_INIT_PREFIX ".")
			|| id.startsWith(LAPSUS_FEAT_CONFIG_PREFIX "."))
		{
			continue;
		}
		
		LapsusFeature *feat;
		
		if (where == LapsusFeature::PlacePanel)
		{
			feat = new LapsusVolSlider(cfg, id, where);
			
			if (feat->dbusValid())
			{
				list.prepend(feat);
				continue;
			}
			else
			{
				delete feat;
			}
			
			feat = new LapsusSlider(cfg, id, where);
			
			if (feat->dbusValid())
			{
				list.prepend(feat);
				continue;
			}
			else
			{
				delete feat;
			}
		}
		
		int idx = id.findRev('.');
		
		if (idx > 0)
		{
			QString fType = id.mid(idx+1);
			LapsusFeature::Place whereShould = LapsusFeature::PlaceUnknown;
			
			if (fType == LAPSUS_FEAT_BLUETOOTH_ID
				|| fType == LAPSUS_FEAT_WIRELESS_ID)
			{
				whereShould = LapsusFeature::PlacePanel;
			}
			else if (fType.startsWith(LAPSUS_FEAT_LED_ID_PREFIX)
					|| fType == LAPSUS_FEAT_TOUCHPAD_ID
					|| fType == "thinklight")
			{
				whereShould = LapsusFeature::PlaceMenu;
			}
			
			if (where == whereShould)
			{
				feat = new LapsusSwitch(cfg, id, where);
		
				if (feat->dbusValid())
				{
					list.append(feat);
					continue;
				}
				else
				{
					delete feat;
				}
			}
		}
	}
	
	return list;
}

void LapsusFeatureManager::writeAutoConfig(KConfig *cfg)
{
	if (!cfg) return;
	
	cfg->setGroup(LAPSUS_CONF_MAIN_GROUP);
	
	// It would be nice to have 'applet' group at the beginning
	cfg->writeEntry(LAPSUS_CONF_AUTODETECT, LAPSUS_CONF_TRUE);

	QPtrList<LapsusFeature> list;
	QStringList entries;
	LapsusFeature *feat;
	
	list = autodetectFeatures(cfg, LapsusFeature::PlacePanel);
	
	for (feat = list.first(); feat; feat = list.next())
	{
		entries.append(feat->getFeatureDBusID());
		feat->saveFeature();
	}
	
	cfg->setGroup(LAPSUS_CONF_MAIN_GROUP);
	cfg->writeEntry(LAPSUS_CONF_PANEL_LIST_SELECTED, entries);
	entries.clear();
	
	list.setAutoDelete(true);
	list.clear();
	
	list = autodetectFeatures(cfg, LapsusFeature::PlaceMenu);
	
	for (feat = list.first(); feat; feat = list.next())
	{
		entries.append(feat->getFeatureDBusID());
		feat->saveFeature();
	}
	
	cfg->setGroup(LAPSUS_CONF_MAIN_GROUP);
	cfg->writeEntry(LAPSUS_CONF_MENU_LIST_SELECTED, entries);
	entries.clear();
	
	list.setAutoDelete(true);
	list.clear();
	
	cfg->sync();
}

LapsusFeature* LapsusFeatureManager::newLapsusFeature(KConfig *cfg,
		const QString &id, LapsusFeature::Place where)
{
	LapsusFeature *feat;
	
	if (where == LapsusFeature::PlacePanel)
	{
		feat = new LapsusVolSlider(cfg, id, where);
		
		if (feat->confValid() || feat->dbusValid()) return feat;
		
		delete feat;
		
		feat = new LapsusSlider(cfg, id, where);
		
		if (feat->confValid() || feat->dbusValid()) return feat;
		
		delete feat;
	}
	
	feat = new LapsusSwitch(cfg, id, where);
	
	if (feat->confValid() || feat->dbusValid()) return feat;
	
	delete feat;
	
	return 0;
}

LapsusPanelWidget* LapsusFeatureManager::newPanelWidget(KConfig *cfg, const QString &id,
			Qt::Orientation orientation, QWidget *parent)
{
	LapsusFeature *feat = newLapsusFeature(cfg, id, LapsusFeature::PlacePanel);
	
	if (!feat) return 0;
	
	if (!feat->dbusValid())
	{
		delete feat;
		return 0;
	}
	
	LapsusPanelWidget* widget = feat->createPanelWidget(orientation, parent);
	
	if (!widget)
	{
		delete feat;
		return 0;
	}
	
	return widget;
}

bool LapsusFeatureManager::newActionButton(KConfig *cfg, const QString &id,
		KActionCollection *parent)
{
	LapsusFeature *feat = newLapsusFeature(cfg, id, LapsusFeature::PlaceMenu);
	
	if (!feat) return 0;
	
	if (!feat->dbusValid())
	{
		delete feat;
		return 0;
	}
	
	bool ret = feat->createActionButton(parent);
	
	if (!ret) delete feat;
	
	return ret;
}
