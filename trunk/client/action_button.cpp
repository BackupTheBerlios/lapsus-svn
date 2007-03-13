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

#include <qwmatrix.h>
#include <qpainter.h>
#include <kiconloader.h>

#include "action_button.h"

LapsusActionButton::LapsusActionButton(const QString &id, LapsusDBus *dbus, KConfig *cfg,
			QObject *parent, const KShortcut &cut):
	KAction(id, cut, 0, 0, parent, id),
	_dbus(dbus), _cfg(cfg), _id(id), _hasDBus(false), _isValid(false)
{
	_cfg->setGroup(_id.lower());

	if (_cfg->hasKey("feature_id"))
	{
		_featureId = _cfg->readEntry("feature_id");

		_name = _dbus->getFeatureName(_featureId);

		if (_name.length() > 0)
			setToolTip(_name);

		_vals = _dbus->getFeatureArgs(_featureId);

		if (_vals.size() > 1)
		{
			_isValid = true;
			_hasDBus = true;
		}

		_curVal = _dbus->getFeature(_featureId);
	}

	QPixmap letter;

	if (_featureId.find("bluetooth") >= 0)
	{
		_iconOn = UserIcon("bluetooth");
		_iconOff = UserIcon("bluetooth_gray");
	}
	else if (_featureId.find("wireless") >= 0)
	{
		_iconOn = UserIcon("wifi");
		_iconOff = UserIcon("wifi_gray");
	}
	else if (_featureId.startsWith("asus_led_"))
	{
		_iconOn = UserIcon("green");
		_iconOff = UserIcon("gray");

		if (_featureId.length() > 9)
		{
			letter = UserIcon(_featureId.mid(9, 1));
		}
	}
	else
	{
		_iconOn = UserIcon("green");
		_iconOff = UserIcon("gray");
	}

	if (!_iconOn.isNull())
	{
		if (!letter.isNull())
		{
			QPainter p;

			p.begin( &_iconOn );
			p.drawPixmap (0, 0, letter);
		}

		// scale icon
		QWMatrix t;
		t = t.scale( 16.0/_iconOn.width(), 16.0/_iconOn.height() );
		_iconOn = _iconOn.xForm( t );
	}

	if (!_iconOff.isNull())
	{
		if (!letter.isNull())
		{
			QPainter p;

			p.begin( &_iconOff );
			p.drawPixmap (0, 0, letter);
		}

		// scale icon
		QWMatrix t;
		t = t.scale( 16.0/_iconOff.width(), 16.0/_iconOff.height() );
		_iconOff = _iconOff.xForm( t );
	}

	checkCurVal();

	connect ( _dbus, SIGNAL(stateChanged(bool)),
			this, SLOT(dbusStateChanged(bool)) );

	connect(_dbus, SIGNAL(featureChanged(const QString &, const QString &)),
			this, SLOT(featureChanged(const QString &, const QString &)));

	connect(this, SIGNAL(activated()),
		this, SLOT(actionClicked()));
}

LapsusActionButton::~LapsusActionButton()
{
}

void LapsusActionButton::checkCurVal()
{
	if (_dbus && _hasDBus && _curVal == "on")
	{
		if (!_iconOn.isNull())
		{
			setIconSet(QIconSet(_iconOn));
			setText(_name);
		}
		else
		{
			setText(QString("+ %1").arg(_name));
		}
	}
	else
	{
		if (!_iconOff.isNull())
		{
			setIconSet(QIconSet(_iconOff));
			setText(_name);
		}
		else
		{
			setText(QString("- %1").arg(_name));
		}
	}
}

void LapsusActionButton::featureChanged(const QString &id, const QString &val)
{
	if (id == _featureId)
	{
		_curVal = val;
		checkCurVal();
	}
}

void LapsusActionButton::dbusStateChanged(bool state)
{
	_hasDBus = state;
	checkCurVal();
}

void LapsusActionButton::actionClicked()
{
	if (!_isValid || !_hasDBus)
		return;

	QStringList::Iterator it = _vals.find(_curVal);
	QString nVal;

	if (it != _vals.end()) ++it;

	if (it == _vals.end())
		nVal = *(_vals.begin());
	else
		nVal = *it;

	_dbus->setFeature(_featureId, nVal);
}
