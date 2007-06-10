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


#ifndef LAPSUS_ACTION_BUTTON_H
#define LAPSUS_ACTION_BUTTON_H

#include <qmap.h>

#include <kactioncollection.h>
#include <kaction.h>
#include <kconfig.h>

#include "lapsus_icons.h"

class LapsusActionButton : public KAction, protected LapsusIcons
{
	Q_OBJECT

	public:
		LapsusActionButton(const QString &id, KConfig *cfg,
			QObject *parent = 0, const KShortcut &cut = KShortcut());
		virtual ~LapsusActionButton();

	private:
		KConfig *_cfg;
		QString _id;
		QString _name;
		QStringList _vals;
		QString _curVal;
		QString _featureId;
		bool _hasDBus;
		bool _isValid;
		QMap<QString, int> _icons;

		void checkCurVal();

	protected slots:
		void actionClicked();
		void featureChanged(const QString &id, const QString &val);
		void dbusStateChanged(bool state);
};

#endif
