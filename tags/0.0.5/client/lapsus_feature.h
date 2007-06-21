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

class LapsusPanelWidget;
class LapsusActionButton;
class QListBox;
class KActionCollection;

class LapsusFeature : public QObject
{
	Q_OBJECT

	public:
		enum Place { PlaceUnknown, PlacePanel, PlaceMenu };
		
		LapsusFeature(KConfig *cfg, const QString &dbusID,
				LapsusFeature::Place where, const char *featureType = 0);
		virtual ~LapsusFeature();

		bool setFeatureValue(const QString &nVal);
		QString getFeatureValue();
		QString getFeatureName();
		QStringList getFeatureArgs();
		void updateFeatureValue();
		
		QString getFeatureConfID();
		QString getFeatureDBusID();
		LapsusFeature::Place getFeaturePlace();
		
		bool dbusConnect();
		
		bool confValid();
		bool dbusValid();
		bool dbusActive();
		
		bool isArgValid(const QString &arg);
		
		virtual bool saveFeature();
		
		virtual LapsusPanelWidget* createPanelWidget(Qt::Orientation orientation, QWidget *parent);
		virtual bool createActionButton(KActionCollection *parent);
		
	signals:
		void featureUpdate(const QString &val);
		void featureNotif(const QString & val);
		
	protected slots:
		virtual void dbusStateUpdate(bool state);
                virtual void dbusFeatureUpdate(const QString &id, const QString &val, bool isNotif);
		
	protected:
		KConfig* _cfg;
		QString _featDBusID;
		LapsusFeature::Place _place;
		const char *_featureType;
		LapsusValidator* _validator;
		bool _confValid;
		bool _dbusValid;
		bool _dbusActive;
		bool _blockSendSet;
};

#endif
