/***************************************************************************
 *   Copyright (C) 2007, 2008 by Jakub Schmidtke                           *
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

#ifndef LAPSUS_CHECK_LIST_ITEM_H
#define LAPSUS_CHECK_LIST_ITEM_H

#include <qlistview.h>
#include <klistview.h>

#include "lapsus_feature.h"

class LapsusCheckListItem: public QCheckListItem
{
	public:
		LapsusCheckListItem(KListView *parent, LapsusFeature *feature);
		virtual ~LapsusCheckListItem();
		
		virtual LapsusFeature* getFeature();
		virtual QString text(int column) const;
		
	protected:
		LapsusFeature* _feature;
};

#endif
