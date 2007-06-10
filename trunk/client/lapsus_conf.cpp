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

#include <qtabwidget.h>
#include <qlistbox.h>
#include <klocale.h>
#include <kactionselector.h>

#include "lapsus_conf.h"

LapsusConf::LapsusConf(QWidget *parent, KConfig *cfg):
	LapsusConfBase(parent), _osd(0), _cfg(cfg)
{
	connect( tabsConf, SIGNAL( currentChanged(QWidget *) ), this, SLOT( tabChanged(QWidget *) ) );
	
	
}

LapsusConf::~LapsusConf()
{
}

void LapsusConf::tabChanged(QWidget *tab)
{
	if (tab == OSDPage)
	{
		if (!_osd)
		{
			_osd = new LapsusOSD(this);
			_osd->setText(i18n("Drag OSD to desired position"));
			_osd->setDraggingEnabled(true);
		}
	
		_osd->show();
	}
	else
	{
		if (_osd) _osd->hide();
	}
}
