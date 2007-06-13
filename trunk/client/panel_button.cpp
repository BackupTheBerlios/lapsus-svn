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

#include "lapsus.h"
#include "lapsus_dbus.h"

#include "panel_button.h"

LapsusPanelButton::LapsusPanelButton(Qt::Orientation orientation,
			QWidget *parent, LapsusSwitch *feat):
		LapsusPanelWidget(orientation, parent, feat),
		_switchFeat(feat), _layout(0), _iconLabel(0)
{
	if (!feat || !isValid()) return;
	
	_layout = new QHBoxLayout( this );
	_layout->setAlignment(Qt::AlignCenter);

	QString tip = feat->getFeatureName();
	QStringList args = feat->getSwitchAllValues();
	
	for (QStringList::Iterator it = args.begin(); it != args.end(); ++it)
	{
		int icon = loadNewAutoIcon(*it, 20);

		if (icon >= 0)
		{
			_icons.insert(*it, icon);
		}
	}

	setBackgroundMode(X11ParentRelative);
	
	_iconLabel = new QLabel(this);
	_iconLabel->setAlignment(Qt::AlignCenter);
	_iconLabel->setBackgroundMode(X11ParentRelative);
	_iconLabel->resize(24, 24);

	installEventFilter(this);

	_layout->add(_iconLabel);
	_layout->addStretch();

	_iconLabel->show();

	if (tip.length() > 0)
		QToolTip::add( _iconLabel, tip);

	buttonUpdate(feat->getSwitchValue());

	connect( feat, SIGNAL(switchUpdate(const QString &)),
			this, SLOT(buttonUpdate(const QString &)));

	_layout->activate();

	const QSize constrainedSize = sizeHint();

	resize( constrainedSize.width(), constrainedSize.height() );
}

LapsusPanelButton::~LapsusPanelButton()
{
}

void LapsusPanelButton::buttonUpdate(const QString &val)
{
	_iconLabel->clear();

	if (_icons.contains(val))
	{
		int icon = _icons[val];

		if (icon >= 0)
		{
			_iconLabel->setPixmap(getIcon(icon));
			return;
		}
	}

	_iconLabel->setText(QString("%1").arg(val.upper()));
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

		if (!isValid() || !hasDBus() || !_switchFeat)
			return true;

		QStringList args = _switchFeat->getSwitchAllValues();
		QString curVal = _switchFeat->getSwitchValue();
		
		if (qme->button() == Qt::LeftButton && args.size() > 0)
		{
			QStringList::Iterator it = args.find(curVal);
			QString nVal;

			if (it != args.end()) ++it;

			if (it == args.end())
				nVal = *(args.begin());
			else
				nVal = *it;

			_switchFeat->setSwitchValue(nVal);
		}
		
		return true;
	}

	return QWidget::eventFilter(obj,e);
}

LapsusPanelButton* LapsusPanelButton::newPanelWidget(const QString &confID,
			Qt::Orientation orientation, QWidget *parent, KConfig *cfg)
{
	if (LapsusSwitch::readFeatureType(confID, cfg) != LapsusSwitch::featureType()) return 0;
	
	LapsusSwitch *feat = new LapsusSwitch(cfg, confID);
	
	if (feat->isValid())
	{
		return new LapsusPanelButton(orientation, parent, feat);
	}
	
	delete feat;
	
	return 0;
}
