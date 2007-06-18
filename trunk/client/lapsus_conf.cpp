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
#include <qtable.h>
#include <qtoolbutton.h>
#include <kiconloader.h>

#include "lapsus.h"
#include "lapsus_dbus.h"
#include "lapsus_conf.h"
#include "feature_manager.h"
#include "checklist_item.h"

LapsusConf::LapsusConf(QWidget *parent, KConfig *cfg):
	LapsusConfBase(parent), _osd(0), _cfg(cfg)
{
	connect( tabsConf, SIGNAL( currentChanged(QWidget *) ), this, SLOT( tabChanged(QWidget *) ) );
	
	if (!cfg || !LapsusDBus::get()->isActive()) return;
	
	btPanelUp->setIconSet(SmallIconSet("up", IconSize(KIcon::Small)));
	btPanelDown->setIconSet(SmallIconSet("down", IconSize(KIcon::Small)));
	btMenuUp->setIconSet(SmallIconSet("up", IconSize(KIcon::Small)));
	btMenuDown->setIconSet(SmallIconSet("down", IconSize(KIcon::Small)));
	
	addAllListEntries(LapsusFeature::PlacePanel);
	addAllListEntries(LapsusFeature::PlaceMenu);
	
	connect (listPanel, SIGNAL(selectionChanged()),
		this, SLOT(panelSelectionChanged()));
	
	connect (btPanelUp, SIGNAL(clicked()),
		this, SLOT(panelUp()));
	
	connect (btPanelDown, SIGNAL(clicked()),
		this, SLOT(panelDown()));
	
	connect (btMenuUp, SIGNAL(clicked()),
		this, SLOT(menuUp()));
	
	connect (btMenuDown, SIGNAL(clicked()),
		this, SLOT(menuDown()));
	
	connect (listMenu, SIGNAL(selectionChanged()),
		this, SLOT(menuSelectionChanged()));
	
	connect (btOK, SIGNAL(clicked()),
		this, SLOT(confOKClicked()));
		
	connect (btCancel, SIGNAL(clicked()),
		this, SLOT(confCancelClicked()));
}

LapsusConf::~LapsusConf()
{
}

void LapsusConf::addListEntries(KListView* itemList,
		QStringList *listFrom, QStringList *listPresent,
		QStringList *listDBus, LapsusFeature::Place where)
{
	for (QStringList::Iterator it = listFrom->begin(); it != listFrom->end(); ++it)
	{
		QString id = (*it);
		
		if (id.startsWith(LAPSUS_FEAT_INIT_PREFIX ".")
			|| id.startsWith(LAPSUS_FEAT_CONFIG_PREFIX "."))
		{
			continue;
		}
		
		LapsusFeature* feat = LapsusFeatureManager::newLapsusFeature(_cfg, id, where);
		
		if (feat != 0)
		{
			LapsusCheckListItem* item = new LapsusCheckListItem(itemList, feat);
			
			QListViewItem* last = itemList->lastChild();
			
			if (last != item)
				item->moveItem(last);
			
			if (listPresent->contains(id))
			{
				item->setOn(true);
			}
			
			if (listDBus) listDBus->remove(id);
		}
	}
}

void LapsusConf::addAllListEntries(LapsusFeature::Place where)
{
	_cfg->setGroup(LAPSUS_CONF_MAIN_GROUP);
	
	QStringList listAll, listPresent;
	QStringList listDBus = LapsusDBus::get()->listFeatures();
	KListView* list = 0;
	
	if (where == LapsusFeature::PlacePanel)
	{
		listAll = _cfg->readListEntry(LAPSUS_CONF_PANEL_LIST_ALL);
		listPresent = _cfg->readListEntry(LAPSUS_CONF_PANEL_LIST_SELECTED);
		list = listPanel;
	}
	else if (where == LapsusFeature::PlaceMenu)
	{
		listAll = _cfg->readListEntry(LAPSUS_CONF_MENU_LIST_ALL);
		listPresent = _cfg->readListEntry(LAPSUS_CONF_MENU_LIST_SELECTED);
		list = listMenu;
	}
	else
	{
		return;
	}
	
	if (!list) return;
	
	list->setSelectionMode(QListView::Single);
	list->setSorting(-1);
	list->addColumn(i18n("ID"));
	list->addColumn(i18n("Name"));
	list->addColumn(i18n("Value"));
	list->setAllColumnsShowFocus(true);
	
	addListEntries(list, &listAll, &listPresent, &listDBus, where);
	addListEntries(list, &listDBus, &listPresent, 0, where);
}

void LapsusConf::panelSelectionChanged()
{
	QListViewItem* item = listPanel->selectedItem();
	
	btPanelUp->setEnabled(false);
	btPanelDown->setEnabled(false);
	
	if (item)
	{
		if (item != listPanel->firstChild()) btPanelUp->setEnabled(true);
		if (item != listPanel->lastChild()) btPanelDown->setEnabled(true);
		
		listPanel->ensureItemVisible(item);
	}
}

void LapsusConf::menuSelectionChanged()
{
	QListViewItem* item = listMenu->selectedItem();
	
	btMenuUp->setEnabled(false);
	btMenuDown->setEnabled(false);
	
	if (item)
	{
		if (item != listMenu->firstChild()) btMenuUp->setEnabled(true);
		if (item != listMenu->lastChild()) btMenuDown->setEnabled(true);
		
		listMenu->ensureItemVisible(item);
	}
}

void LapsusConf::panelUp()
{
	QListViewItem* item = listPanel->selectedItem();
	
	if (item)
	{
		QListViewItem* nItem = item->itemAbove();
	
		if (nItem)
		{
			nItem->moveItem(item);
			
			panelSelectionChanged();
		}
	}
}

void LapsusConf::panelDown()
{
	QListViewItem* item = listPanel->selectedItem();
	
	if (item)
	{
		QListViewItem* nItem = item->itemBelow();
	
		if (nItem)
		{
			item->moveItem(nItem);
			
			panelSelectionChanged();
		}
	}
}

void LapsusConf::menuUp()
{
	QListViewItem* item = listMenu->selectedItem();
	
	if (item)
	{
		QListViewItem* nItem = item->itemAbove();
	
		if (nItem)
		{
			nItem->moveItem(item);
			
			menuSelectionChanged();
		}
	}
}

void LapsusConf::menuDown()
{
	QListViewItem* item = listMenu->selectedItem();
	
	if (item)
	{
		QListViewItem* nItem = item->itemBelow();
	
		if (nItem)
		{
			item->moveItem(nItem);
			
			menuSelectionChanged();
		}
	}
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
	
	QStringList all;
	QStringList selected;
	
	// So it is at the beginning
	_cfg->setGroup(LAPSUS_CONF_MAIN_GROUP);
	_cfg->writeEntry(LAPSUS_CONF_AUTODETECT, LAPSUS_CONF_FALSE);
	
	for( QListViewItem* item = listPanel->firstChild();
		item; item = item->itemBelow())
	{
		LapsusCheckListItem* lItem = (LapsusCheckListItem*) item;
		LapsusFeature *feat = 0;
		
		if (lItem) feat = lItem->getFeature();
		
		if (feat)
		{
			all.append(feat->getFeatureDBusID());
			
			if (lItem->isOn())
			{
				selected.append(feat->getFeatureDBusID());
				feat->saveFeature();
			}
		}
	}
	
	_cfg->setGroup(LAPSUS_CONF_MAIN_GROUP);
	_cfg->writeEntry(LAPSUS_CONF_PANEL_LIST_ALL, all);
	_cfg->writeEntry(LAPSUS_CONF_PANEL_LIST_SELECTED, selected);
	
	all.clear();
	selected.clear();
	
	for( QListViewItem* item = listMenu->firstChild();
		item; item = item->itemBelow())
	{
		LapsusCheckListItem* lItem = (LapsusCheckListItem*) item;
		LapsusFeature *feat = 0;
		
		if (lItem) feat = lItem->getFeature();
		
		if (feat)
		{
			all.append(feat->getFeatureDBusID());
			
			if (lItem->isOn())
			{
				selected.append(feat->getFeatureDBusID());
				feat->saveFeature();
			}
		}
	}
	
	_cfg->setGroup(LAPSUS_CONF_MAIN_GROUP);
	_cfg->writeEntry(LAPSUS_CONF_MENU_LIST_ALL, all);
	_cfg->writeEntry(LAPSUS_CONF_MENU_LIST_SELECTED, selected);
	
	_cfg->sync();
	
	emit finished(true);
}

void LapsusConf::confCancelClicked()
{
	emit finished(false);
}
