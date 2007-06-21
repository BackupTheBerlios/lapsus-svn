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

#ifndef LAPSUS_SWITCH_H
#define LAPSUS_SWITCH_H

#include "lapsus_feature.h"

#define LAPSUS_FEATURE_TYPE_SWITCH		"switch"

class LapsusSwitch : public LapsusFeature
{
	Q_OBJECT

	public:
		LapsusSwitch(KConfig *cfg, const QString &dbusID,
				LapsusFeature::Place where,
				const char *featureType = LAPSUS_FEATURE_TYPE_SWITCH);
		virtual ~LapsusSwitch();

		virtual bool saveFeature();
		
		QString getSwitchValue();
		QStringList getSwitchAllValues();
		
		virtual LapsusPanelWidget* createPanelWidget(Qt::Orientation orientation, QWidget *parent);
		virtual bool createActionButton(KActionCollection *parent);
		
	public slots:
		virtual void setSwitchValue(const QString &val);
	
	protected slots:
		virtual void dbusFeatureUpdate(const QString &id, const QString &val, bool isNotif);
		
	private:
		QString _val;
};

#endif
