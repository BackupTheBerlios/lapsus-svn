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
#include "lapsus_switch.h"

LapsusActionButton::LapsusActionButton(const QString &confID,
		KActionCollection *parent, LapsusSwitch *feat):
	KAction(confID, 0, 0, 0, parent, confID), LapsusIcons(feat),
	_switchFeat(feat)
{
	if (!feat || !feat->dbusValid()) return;
	
	feat->dbusConnect();
	
	_name = feat->getFeatureName();

	if (_name.length() > 0) setToolTip(_name);
	
	QStringList args = _switchFeat->getSwitchAllValues();
	
	for (QStringList::Iterator it = args.begin(); it != args.end(); ++it)
	{
		int icon = loadNewAutoIcon(*it, 16);

		if (icon >= 0)
		{
			_icons.insert(*it, icon);
		}
	}
	
	buttonUpdate(feat->getSwitchValue());
	
	connect(feat, SIGNAL(featureUpdate(const QString &)),
			this, SLOT(buttonUpdate(const QString &)));

	connect(this, SIGNAL(activated()),
		this, SLOT(actionClicked()));
}

LapsusActionButton::~LapsusActionButton()
{
}

void LapsusActionButton::buttonUpdate(const QString &val)
{
	if (_icons.contains(val))
	{
		int icon = _icons[val];

		if (icon >= 0)
		{
			setIconSet(getIcon(icon));
			setText(_name);
			return;
		}
	}

	setText(QString("%1: %2").arg(_name).arg(val.upper()));
}

void LapsusActionButton::actionClicked()
{
	if (!_switchFeat || !_switchFeat->dbusValid() || !_switchFeat->dbusActive() )
		return;
	
	QStringList args = _switchFeat->getSwitchAllValues();
	QString curVal = _switchFeat->getSwitchValue();
	
	QStringList::Iterator it = args.find(curVal);
	QString nVal;

	if (it != args.end()) ++it;

	if (it == args.end())
		nVal = *(args.begin());
	else
		nVal = *it;

	_switchFeat->setSwitchValue(nVal);
}
