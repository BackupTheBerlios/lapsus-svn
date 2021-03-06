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
#include <knuminput.h>
#include <qcheckbox.h>
#include <kcolorbutton.h>

#include "lapsus.h"
#include "lapsus_dbus.h"
#include "lapsus_conf.h"
#include "feature_manager.h"
#include "checklist_item.h"

LapsusConf::LapsusConf(QWidget *parent, KConfig *cfg):
	LapsusConfBase(parent), _osd(cfg, this), _cfg(cfg)
{
	connect( tabsConf, SIGNAL( currentChanged(QWidget *) ), this, SLOT( tabChanged(QWidget *) ) );
	
	// Not needed yet.
	tabsConf->removePage(HotKeyPage);
	tabsConf->removePage(InitPage);
	tabsConf->removePage(DaemonPage);
	
	_osd.setText(i18n("Drag OSD to desired position"));
	_osd.setDraggingEnabled(true);
	
	if (!cfg || !LapsusDBus::get()->isActive()) return;
	
	btPanelUp->setIconSet(SmallIconSet("up", IconSize(KIcon::Small)));
	btPanelDown->setIconSet(SmallIconSet("down", IconSize(KIcon::Small)));
	btMenuUp->setIconSet(SmallIconSet("up", IconSize(KIcon::Small)));
	btMenuDown->setIconSet(SmallIconSet("down", IconSize(KIcon::Small)));
	
	addAllListEntries(LapsusFeature::PlacePanel);
	addAllListEntries(LapsusFeature::PlaceMenu);
	addSaveListEntries();
	
	setOSDValues();
	
	connect (listPanel, SIGNAL(selectionChanged()),
		this, SLOT(panelSelectionChanged()));
	
	connect (btPanelUp, SIGNAL(clicked()),
		this, SLOT(panelUp()));
	
	connect (btPanelDown, SIGNAL(clicked()),
		this, SLOT(panelDown()));
	
	connect (btPanelAuto, SIGNAL(clicked()),
		this, SLOT(panelAuto()));
	
	connect (listMenu, SIGNAL(selectionChanged()),
		this, SLOT(menuSelectionChanged()));
	
	connect (btMenuUp, SIGNAL(clicked()),
		this, SLOT(menuUp()));
	
	connect (btMenuDown, SIGNAL(clicked()),
		this, SLOT(menuDown()));
	
	connect (btMenuAuto, SIGNAL(clicked()),
		this, SLOT(menuAuto()));
	
	
	connect(btOSDReset, SIGNAL(clicked()),
		this, SLOT(resetOSD()));
		
	connect (osdTime, SIGNAL(valueChanged(double)),
		&_osd, SLOT(setTimeout(double)));
	
	connect (osdCustomColors, SIGNAL(toggled(bool)),
		&_osd, SLOT(setUseCustom(bool)));
	
	connect (osdForeground, SIGNAL(changed(const QColor &)),
		&_osd, SLOT(setForeground(const QColor &)));
	
	connect (osdBackground, SIGNAL(changed(const QColor &)),
		&_osd, SLOT(setBackground(const QColor &)));
		
		
	connect (btOK, SIGNAL(clicked()),
		this, SLOT(confOKClicked()));
		
	connect (btCancel, SIGNAL(clicked()),
		this, SLOT(confCancelClicked()));
}

LapsusConf::~LapsusConf()
{
	_osd.hide();
}

void LapsusConf::addListEntries(KListView* itemList,
		QStringList *listFrom, QStringList *listPresent,
		QStringList *listDBus, LapsusFeature::Place where)
{
	for (QStringList::Iterator it = listFrom->begin(); it != listFrom->end(); ++it)
	{
		QString id = (*it);
		
		if (id.startsWith(LAPSUS_FEAT_SAVE_PREFIX ".")
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
			
			if (listPresent && listPresent->contains(id))
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

void LapsusConf::addSaveListEntries()
{
	QStringList listDBus = LapsusDBus::get()->listFeatures();
	
	listSave->setSelectionMode(QListView::Single);
	listSave->setSorting(-1);
	listSave->addColumn(i18n("ID"));
	listSave->addColumn(i18n("Name"));
	listSave->setAllColumnsShowFocus(true);
	
	for (QStringList::Iterator it = listDBus.begin(); it != listDBus.end(); ++it)
	{
		QString id = (*it).lower();
		
		if (id.startsWith(LAPSUS_FEAT_SAVE_PREFIX ".")
			|| id.startsWith(LAPSUS_FEAT_CONFIG_PREFIX "."))
		{
			continue;
		}
		
		QString sId = QString(LAPSUS_FEAT_SAVE_PREFIX ".%1").arg(id);
		QCheckListItem* item = new QCheckListItem(listSave, "", QCheckListItem::CheckBox);
		
		QString val = LapsusDBus::get()->getFeatureValue(sId);
		
		if (val == LAPSUS_FEAT_ON)
		{
			item->setOn(true);
		}
		
		item->setText(0, id);
		item->setText(1, LapsusDBus::get()->getFeatureName(id));
	}
}

void LapsusConf::panelSelectionChanged()
{
	QListViewItem* item = listPanel->selectedItem();
	
	bool enableUp = false;
	bool enableDown = false;
	
	if (item)
	{
		if (item != listPanel->firstChild()) enableUp = true;
		if (item != listPanel->lastChild()) enableDown = true;
		
		listPanel->ensureItemVisible(item);
	}
	
	if (enableUp != btPanelUp->isEnabled())
		btPanelUp->setEnabled(enableUp);
	
	if (enableDown != btPanelDown->isEnabled())
		btPanelDown->setEnabled(enableDown);
}

void LapsusConf::menuSelectionChanged()
{
	QListViewItem* item = listMenu->selectedItem();
	
	bool enableUp = false;
	bool enableDown = false;
	
	if (item)
	{
		if (item != listMenu->firstChild()) enableUp = true;
		if (item != listMenu->lastChild()) enableDown = true;
		
		listMenu->ensureItemVisible(item);
	}
	
	if (enableUp != btMenuUp->isEnabled())
		btMenuUp->setEnabled(enableUp);
	
	if (enableDown != btMenuDown->isEnabled())
		btMenuDown->setEnabled(enableDown);
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

void LapsusConf::fillAuto(KListView* itemList, LapsusFeature::Place where)
{
	QPtrList<LapsusFeature> list = LapsusFeatureManager::autodetectFeatures(_cfg, where);
	LapsusFeature* feat;
	QStringList dbusList = LapsusDBus::get()->listFeatures();
	
	list.setAutoDelete(false);
	
	itemList->clear();
	
	for (feat = list.first(); feat; feat = list.next())
	{
		QString id = feat->getFeatureDBusID();
		LapsusCheckListItem* item = new LapsusCheckListItem(itemList, feat);
		QListViewItem* last = itemList->lastChild();
		
		if (last != item)
			item->moveItem(last);
		
		item->setOn(true);
		
		dbusList.remove(id);
	}
	
	// Add remaining items.
	addListEntries(itemList, &dbusList, 0, 0, where);
}

void LapsusConf::panelAuto()
{
	fillAuto(listPanel, LapsusFeature::PlacePanel);
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

void LapsusConf::menuAuto()
{
	fillAuto(listMenu, LapsusFeature::PlaceMenu);
}

void LapsusConf::resetOSD()
{
	_osd.resetConfig();
	setOSDValues();
}

void LapsusConf::setOSDValues()
{
	osdTime->setValue(_osd.getTimeout());
	osdCustomColors->setChecked(_osd.getUseCustomColors());
	osdForeground->setColor(_osd.getForeground());
	osdBackground->setColor(_osd.getBackground());
}

void LapsusConf::tabChanged(QWidget *tab)
{
	if (tab == OSDPage)
	{
		_osd.show();
	}
	else
	{
		_osd.hide();
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
	
	_osd.saveConfig();
	
	_cfg->setGroup(LAPSUS_CONF_MAIN_GROUP);
	_cfg->writeEntry(LAPSUS_CONF_MENU_LIST_ALL, all);
	_cfg->writeEntry(LAPSUS_CONF_MENU_LIST_SELECTED, selected);
	
	_cfg->sync();
	
	QStringList listDBus = LapsusDBus::get()->listFeatures();
	
	for (QStringList::Iterator it = listDBus.begin(); it != listDBus.end(); ++it)
	{
		QString id = (*it).lower();
		
		if (id.startsWith(LAPSUS_FEAT_SAVE_PREFIX "."))
		{
			LapsusDBus::get()->setFeature(id, LAPSUS_FEAT_OFF);
		}
	}
	
	for( QListViewItem* item = listSave->firstChild();
		item; item = item->itemBelow())
	{
		QCheckListItem* cItem = (QCheckListItem*) item;
		
		if (cItem && cItem->isOn())
		{
			QString id = cItem->text(0);
			
			if (id.length() > 0)
			{
				QString sId = QString(LAPSUS_FEAT_SAVE_PREFIX ".%1").arg(id);
				LapsusDBus::get()->setFeature(sId, LAPSUS_FEAT_ON);
			}
		}
	}
	
	
	emit finished(true);
}

void LapsusConf::confCancelClicked()
{
	emit finished(false);
}
