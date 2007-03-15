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

// KDE
#include <kglobal.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include "applet.h"
#include "lapsus.h"

extern "C"
{
	KDE_EXPORT KPanelApplet* init(QWidget *parent, const QString& configFile)
	{
		KGlobal::locale()->insertCatalogue("lapsus");
		return new LapsusApplet(configFile, KPanelApplet::Normal,
					parent, "lapsusapplet");
	}
}

LapsusApplet::LapsusApplet( const QString& configFile, Type t, QWidget *parent, const char *name )
	: KPanelApplet( configFile, t, 0 , parent, name ),
	_layout(0), _mainWidget(0), _orientation(orientation())
{
	KGlobal::dirs()->addResourceType( "appicon", KStandardDirs::kde_default("data") + "lapsus/pics" );

	setBackgroundMode(X11ParentRelative);

	_layout = new QHBoxLayout(this);

	changeOrientation(_orientation);
}

LapsusApplet::~LapsusApplet()
{
}

void LapsusApplet::about()
{
//TODO
}

void LapsusApplet::help()
{
//TODO
}

void LapsusApplet::reportBug()
{
//TODO
}

void LapsusApplet::changeOrientation(Qt::Orientation orientation)
{
	_orientation = orientation;

	if (_mainWidget)
	{
		_layout->remove(_mainWidget);
		delete _mainWidget;
	}

	_mainWidget = new LapsusPanelMain(this, &_dbus, _orientation);
	_layout->add(_mainWidget);

	_mainWidget->show();
}

int LapsusApplet::widthForHeight(int h) const
{
	if (Qt::Horizontal != _orientation)
	{
		LapsusApplet *mthis = (LapsusApplet *) this;
		mthis->changeOrientation(Qt::Horizontal);
	}

	return _mainWidget->widthForHeight(h);
}

int LapsusApplet::heightForWidth(int w) const
{
	if (Qt::Vertical != _orientation)
	{
		LapsusApplet *mthis = (LapsusApplet *) this;
		mthis->changeOrientation(Qt::Vertical);
	}

	return _mainWidget->heightForWidth(w);
}

void LapsusApplet::preferences()
{
	_mainWidget->appletPreferences();
}

void LapsusApplet::resizeEvent( QResizeEvent *e)
{
}

void LapsusApplet::mousePressEvent( QMouseEvent *e )
{
	if (e->button() == RightButton)
	{
		e->accept();
		_mainWidget->showContextMenu();
	}
}
