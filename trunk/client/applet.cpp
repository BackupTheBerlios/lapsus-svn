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
#include "lapsus_dbus.h"

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
	: KPanelApplet( configFile, t,
	KPanelApplet::About | KPanelApplet::Preferences | KPanelApplet::ReportBug,
	parent, name ),
	_layout(0), _mainWidget(0), _orientation(orientation()),
	_aboutDlg(0), _bugDlg(0),
	_aboutData( "lapsus", I18N_NOOP("Lapsus Panel Applet"),
                         LAPSUS_VERSION,
                         I18N_NOOP("Lapsus provides easy access to additional\nfeatures of ASUS and IBM/Lenovo laptops."),
                         KAboutData::License_GPL,
                         "© 2007 Jakub Schmidtke",
                         0, "http://lapsus.berlios.de", "lapsus-general@lists.berlios.de" )
{
	LapsusDBus::create();
	
	KGlobal::dirs()->addResourceType( "appicon", KStandardDirs::kde_default("data") + "lapsus/pics" );

	setBackgroundMode(X11ParentRelative);

	_layout = new QHBoxLayout(this);

	changeOrientation(_orientation);
	
	_aboutData.setTranslator(
		I18N_NOOP("_: NAME OF TRANSLATORS\\nYour names"),
		I18N_NOOP("_: EMAIL OF TRANSLATORS\\nYour emails"));
	
	_aboutData.addAuthor("Jakub Schmidtke", 0, "sjakub@users.berlios.de");
	
	_aboutData.addCredit("Sebastian Herbord",  I18N_NOOP("Light Sensor support"), "tannin@users.berlios.de");
}

LapsusApplet::~LapsusApplet()
{
	if (_aboutDlg) delete _aboutDlg;
	if (_bugDlg) delete _bugDlg;
	
	/*
	 * It would be removed anyway, but we want to make sure
	 * that when DBus object is removed there is nothing using it
	 * anymore.
	 */
	if (_mainWidget)
	{
		_layout->remove(_mainWidget);
		delete _mainWidget;
	}
	
	LapsusDBus::remove();
}

void LapsusApplet::about()
{
	if (_aboutDlg) delete _aboutDlg;
	
	// Not modal and without parent, so it is centered
	_aboutDlg = new KAboutApplication(&_aboutData, 0, 0, false);
	_aboutDlg->show();
}

void LapsusApplet::help()
{
	// TODO - Maybe someday :)
}

void LapsusApplet::reportBug()
{
	if (_bugDlg) delete _bugDlg;
	
	// Not modal and without parent, so it is centered
	_bugDlg = new KBugReport(0, false, &_aboutData);
	_bugDlg->show();
}

void LapsusApplet::changeOrientation(Qt::Orientation orientation)
{
	_orientation = orientation;

	if (_mainWidget)
	{
		_layout->remove(_mainWidget);
		delete _mainWidget;
	}

	_mainWidget = new LapsusPanelMain(this, _orientation);
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
	if (_mainWidget->appletPreferences())
	{
		changeOrientation(_orientation);
	}
}

void LapsusApplet::resizeEvent( QResizeEvent *)
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
