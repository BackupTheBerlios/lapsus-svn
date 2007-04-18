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

#ifndef FLOW_LAYOUT_H
#define FLOW_LAYOUT_H

#include <qlayout.h>
#include <qptrlist.h>

class FlowLayout : public QLayout
{
	public:
		FlowLayout( QWidget* parent, Qt::Orientation orientation=Qt::Horizontal,
				int border=0, int space=-1, const char* name=0 );
		FlowLayout( QLayout* parent, Qt::Orientation orientation=Qt::Horizontal,
				int space=-1, const char* name=0 );
		FlowLayout( Qt::Orientation=Qt::Horizontal, int space=-1, const char* name=0 );
		~FlowLayout();

		void addItem( QLayoutItem *item);
		bool hasHeightForWidth() const;
		bool hasWidthForHeight() const;
		int heightForWidth( int ) const;
		int widthForHeight( int ) const;

		QSize sizeHint() const;
		QSize minimumSize() const;
		QLayoutIterator iterator();
		QSizePolicy::ExpandData expanding() const;

	protected:
		void setGeometry( const QRect& );

	private:
		Qt::Orientation _orientation;
		QPtrList<QLayoutItem> _list;
		int _cachedW;
		int _cachedH;

		int doLayout( const QRect&, bool testonly = FALSE );
		int doLayoutHorizontal( const QRect&, bool testonly = FALSE );
		int doLayoutVertical( const QRect&, bool testonly = FALSE );
};

#endif
