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

#include <qlayout.h>

#include <klocale.h>

#include "lapsus.h"
#include "conf_dialog.h"

LapsusConfDialog::LapsusConfDialog(QWidget *parent, KConfig *cfg):
	KDialog(parent)
{
	setCaption(i18n("Lapsus configuration"));
	
	QVBoxLayout *l = new QVBoxLayout(this);
	lapsusConf = new LapsusConf(this, cfg);
	l->addWidget(lapsusConf);
	
	connect(lapsusConf,
		SIGNAL(finished(bool)),
		this,
		SLOT(confClosed(bool)));
}

LapsusConfDialog::~LapsusConfDialog()
{
}

void LapsusConfDialog::confClosed(bool ok)
{
	if (ok) accept();
	else reject();
}
