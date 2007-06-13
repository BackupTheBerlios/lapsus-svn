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

#ifndef LAPSUS_FEATURE_H
#define LAPSUS_FEATURE_H

#include <kconfig.h>

#include <qstring.h>
#include <qstringlist.h>

#include "lapsus_validator.h"

#define LAPSUS_CONF_WIDGET_TYPE		"widget_type"
#define LAPSUS_CONF_FEATURE_ID		"feature_id"

class LapsusFeature : public QObject
{
	Q_OBJECT

	public:
		LapsusFeature(KConfig *cfg, const QString &idConf, const char *idDBus = 0);
		virtual ~LapsusFeature();

		bool setFeatureValue(const QString &nVal);
		QString getFeatureValue();
		QString getFeatureName();
		QStringList getFeatureArgs();
		void updateFeatureValue();
		
		QString getFeatureConfID();
		QString getFeatureDBusID();
		
		bool isArgValid(const QString &arg);
	
		bool isValid();
		bool hasDBus();
		
		virtual bool saveFeature();
		
		static QString readFeatureType(const QString &confID, KConfig *cfg);
		
	protected slots:
		virtual void dbusStateUpdate(bool state);
		virtual void dbusFeatureUpdate(const QString &id, const QString &val, bool isNotif) = 0;
		
	protected:
		KConfig* _cfg;
		LapsusValidator* _validator;
		QString _featDBusID;
		QString _featConfID;
		bool _hasDBus;
		bool _isValid;
		bool _blockSendSet;
};

#endif
