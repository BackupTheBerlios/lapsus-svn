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

#ifndef LAPSUS_LISTBOX_SLIDER_H
#define LAPSUS_LISTBOX_SLIDER_H

#include "listbox_feature.h"
#include "lapsus_slider.h"

class LapsusListBoxSlider : public LapsusListBoxFeature
{
	public:
		LapsusListBoxSlider(QListBox* listbox, LapsusSlider* feat);
		virtual ~LapsusListBoxSlider();
		
		virtual bool isConfigurable();
	
		static LapsusListBoxSlider* createListBoxItem(QListBox* listbox, const QString &confID, KConfig *cfg);
		static LapsusListBoxSlider* createListBoxItem(QListBox* listbox, const QString &confID, KConfig *cfg,
			const QString &dbusID, const QStringList &args);
	
	private:
		LapsusSlider* _slider;
};

#endif
