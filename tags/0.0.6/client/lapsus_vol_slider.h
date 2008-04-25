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

#ifndef LAPSUS_VOL_SLIDER_H
#define LAPSUS_VOL_SLIDER_H

#include "lapsus_slider.h"

#define LAPSUS_FEATURE_TYPE_VOL_SLIDER		"vol_slider"

class LapsusVolSlider : public LapsusSlider
{
	Q_OBJECT

	public:
		LapsusVolSlider(KConfig *cfg, const QString &dbusID,
				LapsusFeature::Place where,
				const char *featureType = LAPSUS_FEATURE_TYPE_VOL_SLIDER);
		virtual ~LapsusVolSlider();

		virtual bool saveFeature();
		
		bool isMuted();
		
		virtual LapsusPanelWidget* createPanelWidget(Qt::Orientation orientation, QWidget *parent);
		
	signals:
		void sliderMuteUpdate(bool muted);
		void sliderMuteNotif(bool muted);
		
	public slots:
		virtual void toggleMute();
		virtual void setSliderMute(bool val);
	
	protected slots:
		virtual void dbusSliderUpdate(const QString &val, bool isNotif = false);
		
	private:
		bool _isMuted;
};

#endif
