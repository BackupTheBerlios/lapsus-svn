/***************************************************************************
 *   Copyright (C) 2007, 2008 by Jakub Schmidtke                           *
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

#include "flowlayout.h"

class FlowLayoutIterator :public QGLayoutIterator
{
	public:
		FlowLayoutIterator( QPtrList<QLayoutItem> *l ) :_idx(0), _list(l)  {}
		uint count() const;
		QLayoutItem *current();
		QLayoutItem *next();
		QLayoutItem *takeCurrent();

	private:
		int _idx;
		QPtrList<QLayoutItem> *_list;
};

uint FlowLayoutIterator::count() const
{
    return _list->count();
}

QLayoutItem *FlowLayoutIterator::current()
{
    return _idx < int(count()) ? _list->at(_idx) : 0;
}

QLayoutItem *FlowLayoutIterator::next()
{
    _idx++; return current();
}

QLayoutItem *FlowLayoutIterator::takeCurrent()
{
    return _idx < int(count()) ? _list->take( _idx ) : 0;
}

FlowLayout::FlowLayout( QWidget* parent, Qt::Orientation orientation, int border, int space, const char* name ):
	QLayout( parent, border, space, name ), _orientation(orientation), _cachedW(0), _cachedH(0)
{
}

FlowLayout::FlowLayout( QLayout* parent, Qt::Orientation orientation, int space, const char* name ):
	QLayout( parent, space, name ), _orientation(orientation), _cachedW(0), _cachedH(0)
{
}
FlowLayout::FlowLayout( Qt::Orientation orientation, int space, const char* name ):
	QLayout( space, name ), _orientation(orientation), _cachedW(0), _cachedH(0)
{
}

FlowLayout::~FlowLayout()
{
	deleteAllItems();
}

int FlowLayout::heightForWidth( int w ) const
{
	if ( _cachedW != w )
	{
		//Not all C++ compilers support "mutable" yet:
		FlowLayout * mthis = (FlowLayout*)this;
		int h = mthis->doLayout( QRect(0,0,w,0), true );
		mthis->_cachedH = h;
		mthis->_cachedW = w;
		return h;
	}

	return _cachedH;
}

int FlowLayout::widthForHeight( int h ) const
{
	if ( _cachedH != h )
	{
		//Not all C++ compilers support "mutable" yet:
		FlowLayout * mthis = (FlowLayout*)this;
		int w = mthis->doLayout( QRect(0,0,0,h), true );
		mthis->_cachedH = h;
		mthis->_cachedW = w;
		return w;
	}

	return _cachedW;
}

void FlowLayout::addItem( QLayoutItem *item)
{
	_list.append( item );
}

bool FlowLayout::hasHeightForWidth() const
{
	return _orientation == Qt::Vertical;
}

bool FlowLayout::hasWidthForHeight() const
{
	return _orientation == Qt::Horizontal;
}

QSize FlowLayout::sizeHint() const
{
	QSize s(0,0);
	QPtrListIterator<QLayoutItem> it(_list);
	QLayoutItem *o;

	while ( (o=it.current()) != 0 )
	{
		++it;
		s = s.expandedTo( o->sizeHint() );
	}

	return s;
}

QSizePolicy::ExpandData FlowLayout::expanding() const
{
	return QSizePolicy::NoDirection;
}

QLayoutIterator FlowLayout::iterator()
{
	return QLayoutIterator( new FlowLayoutIterator( &_list ) );
}

void FlowLayout::setGeometry( const QRect &r )
{
	QLayout::setGeometry( r );
	doLayout( r );
}

QSize FlowLayout::minimumSize() const
{
	QSize s(0,0);
	QPtrListIterator<QLayoutItem> it(_list);
	QLayoutItem *o;

	while ( (o=it.current()) != 0 )
	{
		++it;
		s = s.expandedTo( o->minimumSize() );
	}

	return s;
}

int FlowLayout::doLayout( const QRect &r, bool testonly )
{
	if (_orientation == Qt::Horizontal)
		return doLayoutHorizontal(r, testonly);
	else
		return doLayoutVertical(r, testonly);
}

int FlowLayout::doLayoutHorizontal( const QRect &r, bool testonly )
{
	int x = r.x();
	int y = r.y();
	int w = 0;
	QPtrListIterator<QLayoutItem> it(_list);
	QLayoutItem *o;

	while ( (o=it.current()) != 0 )
	{
		++it;
		int nextY = y + o->sizeHint().height() + spacing();

		if ( nextY - spacing() > r.bottom() && w > 0 )
		{
			y = r.y();
			x = x + w + spacing();
			nextY = y + o->sizeHint().height() + spacing();
			w = 0;
		}

		if ( !testonly )
		{
			QRect oR(QPoint( x, y ), o->sizeHint());

			if (oR.bottom() > r.bottom() )
				oR.setBottom(r.bottom());

			o->setGeometry( oR );
		}

		y = nextY;
		w = QMAX( w,  o->sizeHint().width() );
	}

	return x + w - r.x();
}

int FlowLayout::doLayoutVertical( const QRect &r, bool testonly )
{
	int x = r.x();
	int y = r.y();
	int h = 0;
	QPtrListIterator<QLayoutItem> it(_list);
	QLayoutItem *o;

	while ( (o=it.current()) != 0 )
	{
		++it;
		int nextX = x + o->sizeHint().width() + spacing();
		if ( nextX - spacing() > r.right() && h > 0 )
		{
			x = r.x();
			y = y + h + spacing();
			nextX = x + o->sizeHint().width() + spacing();
			h = 0;
		}

		if ( !testonly )
		{
			QRect oR(QPoint( x, y ), o->sizeHint());

			if (oR.right() > r.right() )
				oR.setRight(r.right());

			o->setGeometry( oR );
		}

		x = nextX;
		h = QMAX( h,  o->sizeHint().height() );
	}

	return y + h - r.y();
}
