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
#include "checklist_item.h"

LapsusCheckListItem::LapsusCheckListItem(KListView *parent, LapsusFeature *feature):
	QCheckListItem(parent, "", QCheckListItem::CheckBox),
	_feature(feature)
{
	if (feature && feature->dbusValid())
	{
		feature->dbusConnect();
		
		LapsusFeature::connect(feature, SIGNAL(featureUpdate(const QString &)),
			parent, SLOT(triggerUpdate()));
	}
}

LapsusCheckListItem::~LapsusCheckListItem()
{
	if (_feature) delete _feature;
}

QString LapsusCheckListItem::text(int column) const
{
	if (!_feature) return QString();
	
	switch (column)
	{
		case 0: return _feature->getFeatureDBusID();
		break;
		
		case 1: return _feature->getFeatureName();
		break;
		
		case 2: return _feature->getFeatureValue();
		break;
	}
	
	return QString();
}

LapsusFeature* LapsusCheckListItem::getFeature()
{
	return _feature;
}
