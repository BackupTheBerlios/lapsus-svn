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

#ifndef LAPSUS_VOL_SLIDER_H
#define LAPSUS_VOL_SLIDER_H

#include "lapsus_slider.h"

class LapsusVolSlider : public LapsusSlider
{
	Q_OBJECT

	public:
		LapsusVolSlider(KConfig *cfg, const QString &idConf, const char *idDBus = 0);
		virtual ~LapsusVolSlider();

		virtual bool saveFeature();
		
		bool isMuted();
		
		static bool supportsArgs(const QStringList & args);
		static void addConfigEntry(const QString &confID, const QString &dbusID, KConfig *cfg);
		static const char *featureType();
		
	signals:
		void sliderMuteUpdate(bool muted);
		void sliderMuteNotif(bool muted);
		
	public slots:
		virtual void toggleMute();
		virtual void setSliderMute(bool val);
	
	protected slots:
		virtual void dbusFeatureUpdate(const QString &id, const QString &val, bool isNotif = false);
		
	private:
		bool _isMuted;
};

#endif
