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

#include "panel_button.h"

LapsusPanelButton::LapsusPanelButton( const QString &id,
	Qt::Orientation orientation, QWidget *parent, LapsusDBus *dbus, KConfig *cfg) :
		LapsusPanelWidget(id, orientation, parent, dbus, cfg),
		_layout(0), _iconLabel(0), _iconOn(-1), _iconOff(-1),
		_hasDBus(false), _isValid(false)
{
	_layout = new QHBoxLayout( this );
	_layout->setAlignment(Qt::AlignCenter);

	_cfg->setGroup(id.lower());

	QString tip;

	if (_cfg->hasKey("feature_id"))
	{
		_featureId = _cfg->readEntry("feature_id");

		tip = _dbus->getFeatureName(_featureId);

		_vals = _dbus->getFeatureArgs(_featureId);

		if (_vals.size() > 1)
		{
			_isValid = true;
			_hasDBus = true;
		}

		_curVal = _dbus->getFeature(_featureId);
	}

	setBackgroundMode(X11ParentRelative);
	_iconLabel = new QLabel(this);
	_iconLabel->setAlignment(Qt::AlignCenter);
	_iconLabel->setBackgroundMode(X11ParentRelative);
	_iconLabel->resize(24, 24);

	installEventFilter(this);

	_iconOn = loadNewAutoIcon("on", 20);
	_iconOff = loadNewAutoIcon("off", 20);

	_layout->add(_iconLabel);
	_layout->addStretch();

	_iconLabel->show();

	if (tip.length() > 0)
		QToolTip::add( _iconLabel, tip);

	checkCurVal();

	connect ( _dbus, SIGNAL(stateChanged(bool)),
			this, SLOT(dbusStateChanged(bool)) );

	connect(_dbus, SIGNAL(featureChanged(const QString &, const QString &)),
			this, SLOT(featureChanged(const QString &, const QString &)));

	_layout->activate();

	const QSize constrainedSize = sizeHint();

	resize( constrainedSize.width(), constrainedSize.height() );
}

LapsusPanelButton::~LapsusPanelButton()
{
}

bool LapsusPanelButton::supportsArgs(const QStringList & args)
{
	if (args.size() > 0)
	{
		for (QStringList::ConstIterator it = args.begin(); it != args.end(); ++it)
		{
			if (QStringList::split(':', *it).size() > 1)
				return false;
		}

		return true;
	}

	return false;
}

void LapsusPanelButton::checkCurVal()
{
	_iconLabel->clear();

	if (_dbus && _hasDBus && _curVal == "on")
	{
		if (_iconOn >= 0)
			_iconLabel->setPixmap(getIcon(_iconOn));
		else
			_iconLabel->setText("+");
	}
	else
	{
		if (_iconOff >= 0)
			_iconLabel->setPixmap(getIcon(_iconOff));
		else
			_iconLabel->setText("-");
	}
}

void LapsusPanelButton::featureChanged(const QString &id, const QString &val)
{
	if (id == _featureId)
	{
		_curVal = val;
		checkCurVal();
	}
}

void LapsusPanelButton::dbusStateChanged(bool state)
{
	_hasDBus = state;
	checkCurVal();
}

QSize LapsusPanelButton::sizeHint() const
{
	if ( _panelOrientation == Qt::Horizontal )
		return QSize( 24, 24 );
	else
		return QSize( 24, 24 );
}

QSize LapsusPanelButton::minimumSizeHint() const
{
    QSize s(24, 24);
    return s;
}

QSizePolicy LapsusPanelButton::sizePolicy() const
{
	if ( _panelOrientation == Qt::Horizontal )
		return QSizePolicy(  QSizePolicy::Fixed, QSizePolicy::Fixed );

	return QSizePolicy(  QSizePolicy::Fixed, QSizePolicy::Expanding );
}

bool LapsusPanelButton::eventFilter( QObject* obj, QEvent* e )
{
	if (e->type() == QEvent::MouseButtonPress)
	{
		QMouseEvent *qme = static_cast<QMouseEvent*>(e);

		if (qme->button() == Qt::RightButton)
		{
			emit rightButtonPressed();
			return true;
		}

		if (!_isValid || !_hasDBus)
			return true;

		if (qme->button() == Qt::LeftButton && _vals.size() > 0)
		{
			QStringList::Iterator it = _vals.find(_curVal);
			QString nVal;

			if (it != _vals.end()) ++it;

			if (it == _vals.end())
				nVal = *(_vals.begin());
			else
				nVal = *it;

			_dbus->setFeature(_featureId, nVal);
		}
	}

	return QWidget::eventFilter(obj,e);
}
