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
#include "panel_slider.h"

LapsusPanelSlider::LapsusPanelSlider( const QString &id,
	Qt::Orientation orientation, QWidget *parent, LapsusDBus *dbus, KConfig *cfg) :
		LapsusPanelWidget(id, orientation, parent, dbus, cfg),
		_layout(0), _slider(0), _iconLabel(0),
		_hasDBus(false), _isValid(false), _supportsMute(false)
{
	if ( orientation == Qt::Horizontal )
		_layout = new QVBoxLayout( this );
	else
		_layout = new QHBoxLayout( this );

	_layout->setAlignment(Qt::AlignCenter);

	_cfg->setGroup(id.lower());

	int minSlider = 0;
	int maxSlider = 0;
	int sliderVal = 0;
	QString sliderTip;

	if (_cfg->hasKey("feature_id"))
	{
		_featureId = _cfg->readEntry("feature_id");

		sliderTip = _dbus->getFeatureName(_featureId);

		QStringList list = _dbus->getFeatureArgs(_featureId);

		int minV, maxV;

		if (getMinMaxArgs(list, &minV, &maxV, &_supportsMute))
		{
			// TODO - maybe it should be re-initialized
			// everytime dbus goes up ?
			_isValid = true;
			_hasDBus = true;
			minSlider = minV;
			maxSlider = maxV;
		}

		sliderVal = _dbus->getFeature(_featureId).toInt();
	}

	int idx = loadNewAutoIcon(10);

	installEventFilter(this);

	if (idx >= 0 )
	{
		_iconLabel = new QLabel(this);
		_iconLabel->setBackgroundMode(X11ParentRelative);

		_iconLabel->setPixmap( getIcon(idx) );
		_iconLabel->resize( 10, 10 );

		_layout->add(_iconLabel);
		_layout->addSpacing( 5 );

		_iconLabel->show();
	}

	if (_panelOrientation == Qt::Horizontal)
	{
		_slider = new KSmallSlider(
				minSlider, maxSlider, maxSlider/4, sliderVal,
				Qt::Vertical, this);
	}
	else
	{
		_slider = new KSmallSlider(
				minSlider, maxSlider, maxSlider/4, sliderVal,
				Qt::Horizontal, this);
	}

	_slider->setBackgroundMode(X11ParentRelative);

	if (sliderTip.length() > 0)
		QToolTip::add( this, sliderTip);

	_slider->installEventFilter(this);

	connect ( _slider, SIGNAL(valueChanged(int)),
			this, SLOT(sliderValueChanged(int)) );

	connect ( _dbus, SIGNAL(stateChanged(bool)),
			this, SLOT(dbusStateChanged(bool)) );

	connect(_dbus, SIGNAL(featureChanged(const QString &, const QString &)),
			this, SLOT(featureChanged(const QString &, const QString &)));

	_layout->add(_slider);
	_layout->activate();

	_slider->setColors( "#FFFF00", "#707000", "#000000" );

	if (!_isValid || !_hasDBus)
		_slider->setGray(true);

	const QSize constrainedSize = sizeHint();

	_slider->setGeometry( 0, 0, constrainedSize.width(), constrainedSize.height() );
	resize( constrainedSize.width(), constrainedSize.height() );

	_slider->show();
}

LapsusPanelSlider::~LapsusPanelSlider()
{
}

bool LapsusPanelSlider::getMinMaxArgs(const QStringList & args, int *minV, int *maxV, bool *supportsMute)
{
	// TODO Temporary hack only for (un)mute value of volume feature!
	bool hasMute = false;
	bool hasUnMute = false;
	bool ret = false;
	
	for (uint i = 0; i < args.size(); ++i)
	{
		QStringList list = QStringList::split(':', args[i]);

		if (list.size() == 2)
		{
			*minV = list[0].toInt();
			*maxV = list[1].toInt();

			if ( *minV < *maxV )
				ret = true;
		}
		else if (list.size() < 2)
		{
			if (args[i] == LAPSUS_FEAT_MUTE) hasMute = true;
			else if (args[i] == LAPSUS_FEAT_UNMUTE) hasUnMute = true;
		}
	}
	
	if (hasMute && hasUnMute) *supportsMute = true;

	return ret;
}

bool LapsusPanelSlider::supportsArgs(const QStringList & args)
{
	int minV, maxV;
	bool sM;

	return getMinMaxArgs(args, &minV, &maxV, &sM);
}

void LapsusPanelSlider::sliderValueChanged(int nValue)
{
	if (!_dbus || !_hasDBus) return;

	_dbus->setFeature(_featureId, QString::number(nValue));
}

void LapsusPanelSlider::featureChanged(const QString &id, const QString &val)
{
	if (id == _featureId)
	{
		QStringList args = QStringList::split(",", val);
			
		bool setGray = false;
		bool setVal = false;
		int nVal = 0;
		
		for (uint i = 0; i < args.size(); ++i)
		{
			if (args[i] == LAPSUS_FEAT_MUTE)
			{
				setGray = true;
			}
			else if (args[i] == LAPSUS_FEAT_UNMUTE)
			{
				setGray = false;
			}
			else
			{
				bool ok;
		
				int x = args[i].toInt(&ok);
				
				if (ok)
				{
					setVal = true;
					nVal = x;
				}
			}
		}
		
		_slider->setGray(setGray);
		if (setVal) _slider->setValue(nVal);

	}
}

void LapsusPanelSlider::dbusStateChanged(bool state)
{
	_hasDBus = state;
	_slider->setGray(!state);
}

QSize LapsusPanelSlider::sizeHint() const
{
	if ( _panelOrientation == Qt::Horizontal )
		return QSize( 10, 1000 );
	else
		return QSize( 1000, 10 );
}

QSize LapsusPanelSlider::minimumSize() const
{
    QSize s(10,10);
    return s;
}

QSizePolicy LapsusPanelSlider::sizePolicy() const
{
	if ( _panelOrientation == Qt::Horizontal )
		return QSizePolicy(  QSizePolicy::Fixed, QSizePolicy::Expanding );

	return QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );
}

void LapsusPanelSlider::resizeEvent(QResizeEvent *e)
{
	if (!_slider) return;

	if ( _panelOrientation == Qt::Horizontal)
	{
		if (_iconLabel)
		{
			if (e->size().height()<32) _iconLabel->hide();
			else _iconLabel->show();
		}
	}
	else
	{
		if (_iconLabel)
		{
			if (e->size().width()<32) _iconLabel->hide();
			else _iconLabel->show();
		}
	}
}

void LapsusPanelSlider::wheelEvent( QWheelEvent * e)
{
	// Slider does it too, but we want mouse wheel to work also
	// above the label icon
	if (_slider && _hasDBus && _isValid)
	{
		if (e->delta() > 0)
		{
			_slider->moveUp();
		}
		else
		{
			_slider->moveDown();
		}

		e->accept();
	}
}

bool LapsusPanelSlider::eventFilter( QObject* obj, QEvent* e )
{
	if (e->type() == QEvent::MouseButtonPress)
	{
		QMouseEvent *qme = static_cast<QMouseEvent*>(e);

		if (qme->button() == Qt::RightButton)
		{
			emit rightButtonPressed();
			return true;
		}
		
		if (qme->button() == Qt::MidButton)
		{
			if (_supportsMute && _dbus && _hasDBus)
			{
				_slider->setGray(!_slider->gray());
				
				_dbus->setFeature(_featureId, (_slider->gray())?LAPSUS_FEAT_MUTE:LAPSUS_FEAT_UNMUTE);
			}
			
			return true;
		}

		if (!_isValid || !_hasDBus)
			return true;
	}

	if (!_isValid || !_hasDBus)
	{
		if (e->type() == QEvent::MouseMove
			|| e->type() == QEvent::Wheel)
			return true;
	}

	return QWidget::eventFilter(obj,e);
}
