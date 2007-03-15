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

#include "panel_slider.h"

LapsusPanelSlider::LapsusPanelSlider( const QString &id,
	Qt::Orientation orientation, QWidget *parent, LapsusDBus *dbus, KConfig *cfg) :
		LapsusPanelWidget(id, orientation, parent, dbus, cfg),
		_hasDBus(false), _isValid(false)
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

		if (list.size() > 0)
		{
			list = QStringList::split(':', list[0]);

			if (list.size() > 1)
			{
				// TODO - maybe it should be re-initialized
				// everytime dbus goes up ?
				_isValid = true;
				_hasDBus = true;
				minSlider = list[0].toInt();
				maxSlider = list[1].toInt();
			}
		}

		sliderVal = _dbus->getFeature(_featureId).toInt();
	}

	int idx = loadNewIcon("light_bulb", "", 10);

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

bool LapsusPanelSlider::supportsArgs(const QStringList & args)
{
	if (args.size() == 1)
	{
		QStringList list = QStringList::split(':', args[0]);

		if (list.size() == 2)
		{
			int minV, maxV;

			minV = list[0].toInt();
			maxV = list[1].toInt();

			if ( minV < maxV )
				return true;
		}
	}

	return false;
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
		_slider->setValue(val.toInt());
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

		_slider->resize(_slider->width(), e->size().height());
		_layout->invalidate();
	}
	else
	{
		if (_iconLabel)
		{
			if (e->size().width()<32) _iconLabel->hide();
			else _iconLabel->show();
		}

		_slider->resize(e->size().width(),_slider->height());
		_layout->invalidate();
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
