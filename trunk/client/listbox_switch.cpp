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
#include "listbox_switch.h"

LapsusListBoxSwitch::LapsusListBoxSwitch(QListBox* listbox, LapsusSwitch* feat):
	LapsusListBoxFeature(listbox, feat)
{
}

LapsusListBoxSwitch::~LapsusListBoxSwitch()
{
}

bool LapsusListBoxSwitch::isConfigurable()
{
	return false;
}

LapsusListBoxSwitch* LapsusListBoxSwitch::createListBoxItem(
	QListBox* listbox, const QString &confID, KConfig *cfg)
{
	if (LapsusSwitch::readFeatureType(confID, cfg) != LapsusSwitch::featureType()) return 0;
	
	LapsusSwitch *feat = new LapsusSwitch(cfg, confID);
	
	if (feat->isValid())
	{
		return new LapsusListBoxSwitch(listbox, feat);
	}
	
	delete feat;
	return 0;
}

LapsusListBoxSwitch* LapsusListBoxSwitch::createListBoxItem(
	QListBox* listbox, const QString &confID, KConfig *cfg,
	const QString &dbusID, const QStringList &args)
{
	if (!LapsusSwitch::supportsArgs(args)) return 0;
	
	LapsusSwitch *feat = new LapsusSwitch(cfg, confID, dbusID);
	
	if (feat->isValid())
	{
		return new LapsusListBoxSwitch(listbox, feat);
	}
	
	delete feat;
	return 0;
}
