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


#ifndef LAPSUS_ICONS_H
#define LAPSUS_ICONS_H

#include <qvaluevector.h>
#include <qpixmap.h>

#include "lapsus_feature.h"

class LapsusIcons
{
	public:
		LapsusIcons(LapsusFeature* feat);
		~LapsusIcons();

		int loadNewAutoIcon(int size);
		int loadNewAutoIcon(const QString &val, int size);
		int loadNewIcon(const QString &img, int size);
		int loadNewIcon(const QString &img, const QString &desc, int size);
		QPixmap getIcon(int id) const;

	private:
		LapsusFeature* _feature;
		QString _featureType;
		QValueVector<QPixmap> _cachedIcons;
};

#endif
