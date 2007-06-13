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

#include "panel_slider.h"

LapsusPanelSlider::LapsusPanelSlider(Qt::Orientation orientation, QWidget *parent,
			LapsusSlider *sliderFeat):
		LapsusPanelWidget(orientation, parent, sliderFeat),
		_layout(0), _slider(0), _iconLabel(0)
{
	if (!sliderFeat || !isValid()) return;
	
	if ( orientation == Qt::Horizontal )
		_layout = new QVBoxLayout( this );
	else
		_layout = new QHBoxLayout( this );

	_layout->setAlignment(Qt::AlignCenter);

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

	int minSlider = sliderFeat->getSliderMin();
	int maxSlider = sliderFeat->getSliderMax();
	int sliderVal = sliderFeat->getSliderValue();
	
	if (_panelOrientation == Qt::Horizontal)
	{
		_slider = new KSmallSlider(minSlider, maxSlider,
				maxSlider/4, sliderVal,
				Qt::Vertical, this);
	}
	else
	{
		_slider = new KSmallSlider(
				minSlider, maxSlider, maxSlider/4, sliderVal,
				Qt::Horizontal, this);
	}

	_slider->setBackgroundMode(X11ParentRelative);

	QString sliderTip = sliderFeat->getFeatureName();
	
	if (sliderTip.length() > 0)
		QToolTip::add( this, sliderTip);

	_slider->installEventFilter(this);

	connect ( _slider, SIGNAL(valueChanged(int)),
			sliderFeat, SLOT(setSliderValue(int)) );

	connect( sliderFeat, SIGNAL(sliderUpdate(int)),
			_slider, SLOT(setValue(int)));

	connect ( LapsusDBus::get(), SIGNAL(dbusStateUpdate(bool)),
			this, SLOT(dbusStateUpdate(bool)) );

	_layout->add(_slider);
	_layout->activate();

	_slider->setColors( "#FFFF00", "#707000", "#000000" );

	if (!isValid() || !hasDBus())
		_slider->setGray(true);

	const QSize constrainedSize = sizeHint();

	_slider->setGeometry( 0, 0, constrainedSize.width(), constrainedSize.height() );
	resize( constrainedSize.width(), constrainedSize.height() );

	_slider->show();
}

LapsusPanelSlider::~LapsusPanelSlider()
{
}

void LapsusPanelSlider::dbusStateUpdate(bool state)
{
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
	if (_slider && hasDBus() && isValid())
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
			return true;
		
		if (!isValid() || !hasDBus())
			return true;
	}

	if (!isValid() || !hasDBus())
	{
		if (e->type() == QEvent::MouseMove
			|| e->type() == QEvent::Wheel)
			return true;
	}

	return QWidget::eventFilter(obj,e);
}

LapsusPanelSlider* LapsusPanelSlider::newPanelWidget(const QString &confID,
			Qt::Orientation orientation, QWidget *parent, KConfig *cfg)
{
	if (LapsusSlider::readFeatureType(confID, cfg) != LapsusSlider::featureType()) return 0;
	
	LapsusSlider *feat = new LapsusSlider(cfg, confID);
	
	if (feat->isValid())
	{
		return new LapsusPanelSlider(orientation, parent, feat);
	}
	
	delete feat;
	
	return 0;
}
