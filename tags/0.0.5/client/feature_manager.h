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
		/*
		LapsusFeatureManager(KConfig* cfg);
		~LapsusFeatureManager();
		
		QStringList* getDBusIDs();
		void addFeature(const QString &id, LapsusFeature* feat);
		LapsusFeature* getFeature(const QString &id);
		*/
		static QPtrList<LapsusFeature> autodetectFeatures(KConfig* cfg, LapsusFeature::Place where);
		
		static void writeAutoConfig(KConfig* cfg);
		
		static LapsusFeature* newLapsusFeature(KConfig* cfg, const QString &id,
			LapsusFeature::Place where);
		
		static LapsusPanelWidget* newPanelWidget(KConfig *cfg, const QString &id,
			Qt::Orientation orientation, QWidget* parent);
		
		static bool newActionButton(KConfig* cfg, const QString &id,
			KActionCollection* parent);
	/*
	private:
		KConfig *_cfg;
		QDict<LapsusFeature> _mapFeatures;
		QPtrList<LapsusFeature> _features;
		QStringList _dbusIDs;
	*/
};

#endif
