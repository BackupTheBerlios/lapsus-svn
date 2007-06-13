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

class LapsusSwitch : public LapsusFeature
{
	Q_OBJECT

	public:
		LapsusSwitch(KConfig *cfg, const QString &idConf, const char *idDBus = 0);
		virtual ~LapsusSwitch();

		virtual bool saveFeature();
		
		QString getSwitchValue();
		QStringList getSwitchAllValues();
		
		static bool supportsArgs(const QStringList & args);
		static bool addConfigEntry(const QString &confID, const QString &dbusID, KConfig *cfg);
		static const char *featureType();
		
	signals:
		void switchUpdate(const QString &val);
		void switchNotif(const QString & val);
		
	public slots:
		virtual void setSwitchValue(const QString &val);
	
	protected slots:
		virtual void dbusFeatureUpdate(const QString &id, const QString &val, bool isNotif);
		
	private:
		QString _val;
};

#endif
