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

#ifndef LAPSUS_FEATURE_MANAGER_H
#define LAPSUS_FEATURE_MANAGER_H

#include <qptrlist.h>
#include <kconfig.h>

#include "lapsus_feature.h"

class LapsusFeatureManager
{
	public:
		static QPtrList<LapsusFeature> autodetectFeatures(KConfig *cfg, LapsusFeature::Place where);
		
		static void writeAutoConfig(KConfig *cfg);
		
		static LapsusFeature* newLapsusFeature(KConfig *cfg, const QString &id,
			LapsusFeature::Place where, LapsusFeature::ValidityMode vMode);
		
		static LapsusPanelWidget* newPanelWidget(KConfig *cfg, const QString &id,
			Qt::Orientation orientation, QWidget *parent, LapsusFeature::ValidityMode vMode);
		
		static LapsusListBoxFeature* newListBoxFeature(KConfig *cfg, const QString &id,
			LapsusFeature::Place where, QListBox* listbox, LapsusFeature::ValidityMode vMode);
		
		static bool newActionButton(KConfig *cfg, const QString &id,
			KActionCollection *parent, LapsusFeature::ValidityMode vMode);
	
	private:
		static bool checkModeValid(bool confValid, bool dbusValid, LapsusFeature::ValidityMode vMode);
};

#endif