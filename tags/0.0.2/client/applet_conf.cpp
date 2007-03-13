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

// QT
#include <qlayout.h>
#include <kcolorbutton.h>

#include "applet_conf.h"

AppletConfigDialog::AppletConfigDialog( QWidget * parent, const char * name )
	: KDialogBase( KDialogBase::Plain, QString::null,
		KDialogBase::Ok | KDialogBase::Apply | KDialogBase::Cancel,
		KDialogBase::Ok, parent, name, false, true)
{
	setPlainCaption(i18n("Configure - Lapsus Applet"));
	QFrame* page = plainPage();
	QVBoxLayout *topLayout = new QVBoxLayout(page);
	colorWidget = new ColorWidget(page);
	topLayout->addWidget(colorWidget);
}

void AppletConfigDialog::slotOk()
{
	slotApply();
	KDialogBase::slotOk();
}

void AppletConfigDialog::slotApply()
{
	emit applied();
}

void AppletConfigDialog::setActiveColors(const QColor& high, const QColor& low, const QColor& back)
{
	colorWidget->activeHigh->setColor(high);
	colorWidget->activeLow->setColor(low);
	colorWidget->activeBack->setColor(back);
}

void AppletConfigDialog::activeColors(QColor& high, QColor& low, QColor& back) const
{
	high = colorWidget->activeHigh->color();
	low  = colorWidget->activeLow->color();
	back = colorWidget->activeBack->color();
}
