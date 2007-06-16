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

#ifndef LAPSUS_LISTBOX_FEATURE_H
#define LAPSUS_LISTBOX_FEATURE_H

#include <qlistbox.h>

class LapsusFeature;

class LapsusListBoxFeature : public QListBoxText
{
	public:
		LapsusListBoxFeature(QListBox* listbox, LapsusFeature* feat);
		virtual ~LapsusListBoxFeature();
		
		LapsusFeature* getFeature();
		
		virtual bool isConfigurable();
	
	private:
		LapsusFeature* _feature;
};

#endif
