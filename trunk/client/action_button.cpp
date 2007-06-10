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

#include "lapsus.h"
#include "lapsus_dbus.h"

#include "action_button.h"

LapsusActionButton::LapsusActionButton(const QString &id, KConfig *cfg,
			QObject *parent, const KShortcut &cut):
	KAction(id, cut, 0, 0, parent, id), LapsusIcons(id, cfg),
	_cfg(cfg), _id(id), _hasDBus(false), _isValid(false)
{
	_cfg->setGroup(id);

	if (_cfg->hasKey("feature_id"))
	{
		_featureId = _cfg->readEntry("feature_id");

		_name = LapsusDBus::get()->getFeatureName(_featureId);

		if (_name.length() > 0)
		{
			setToolTip(_name);
		}

		_vals = LapsusDBus::get()->getFeatureArgs(_featureId);

		if (_vals.size() > 1)
		{
			_isValid = true;
			_hasDBus = true;

			for (QStringList::Iterator it = _vals.begin(); it != _vals.end(); ++it)
			{
				int icon = loadNewAutoIcon(*it, 16);

				if (icon >= 0) _icons.insert(*it, icon);
			}
		}

		_curVal = LapsusDBus::get()->getFeature(_featureId);
	}

	checkCurVal();

	connect ( LapsusDBus::get(), SIGNAL(stateChanged(bool)),
			this, SLOT(dbusStateChanged(bool)) );

	connect( LapsusDBus::get(), SIGNAL(featureChanged(const QString &, const QString &)),
			this, SLOT(featureChanged(const QString &, const QString &)));

	connect(this, SIGNAL(activated()),
		this, SLOT(actionClicked()));
}

LapsusActionButton::~LapsusActionButton()
{
}

void LapsusActionButton::checkCurVal()
{
	if (_icons.contains(_curVal))
	{
		int icon = _icons[_curVal];

		if (icon >= 0)
		{
			setIconSet(getIcon(icon));
			setText(_name);
			return;
		}
	}

	setText(QString("%1: %2").arg(_name).arg(_curVal.upper()));
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

	LapsusDBus::get()->setFeature(_featureId, nVal);
}
