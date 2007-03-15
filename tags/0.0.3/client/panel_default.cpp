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

#include <qwidget.h>
#include <qlabel.h>
#include <qtooltip.h>
#include <qwmatrix.h>

#include <kiconloader.h>

#include "panel_default.h"

LapsusPanelDefault::LapsusPanelDefault( const QString &id,
	Qt::Orientation orientation, QWidget *parent, LapsusDBus *dbus, KConfig *cfg) :
		LapsusPanelWidget(id, orientation, parent, dbus, cfg), _label(0)
{
	if ( orientation == Qt::Horizontal )
		_layout = new QVBoxLayout( this );
	else
		_layout = new QHBoxLayout( this );

	_layout->addStretch();

	_label = new QLabel(this);
	_label->setBackgroundMode(X11ParentRelative);

	QPixmap pic = UserIcon("laptop");

	if (!pic.isNull() )
	{
		// scale icon
		QWMatrix t;
		t = t.scale( 20.0/pic.width(), 20.0/pic.height() );
		pic = pic.xForm( t );

		_label->setPixmap( pic );
		_label->resize( 20, 20 );
	}
	else
	{
		// Desperate fallback... ;)
		_label->setText("Lapsus\nApplet");
	}

	_layout->add(_label);
	_layout->addStretch();

	_label->show();

	QToolTip::add( _label, "Lapsus Panel Applet" );

	_label->show();
}

LapsusPanelDefault::~LapsusPanelDefault()
{
}

QSize LapsusPanelDefault::sizeHint() const
{
	if ( _label ) return _label->sizeHint();

	return size();
}

QSize LapsusPanelDefault::minimumSizeHint() const
{
	if ( _label ) return _label->minimumSizeHint();

	QSize s(20,20);
	return s;
}

QSizePolicy LapsusPanelDefault::sizePolicy() const
{
	if ( _label ) return _label->sizePolicy();

	if ( _panelOrientation == Qt::Horizontal )
		return QSizePolicy(  QSizePolicy::Fixed, QSizePolicy::Expanding );

	return QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );
}

void LapsusPanelDefault::mousePressEvent( QMouseEvent *e )
{
	if (e->button() == RightButton)
	{
		e->accept();
		emit rightButtonPressed();
	}
}
