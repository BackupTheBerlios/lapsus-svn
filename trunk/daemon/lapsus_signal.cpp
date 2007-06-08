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

#include "lapsus_signal.h"

#include <signal.h>

#include <qapplication.h>

volatile sig_atomic_t signalReceived = 0;

void unixSignalHandler(int)
{
	signalReceived = 1;
}

LapsusSignal::LapsusSignal()
{
	startedExit = false;
	
	signal(SIGINT, unixSignalHandler);
	signal(SIGTERM, unixSignalHandler);
	signal(SIGQUIT, unixSignalHandler);
	
	_timer =  new QTimer( this );
	
	connect( _timer, SIGNAL(timeout()), this, SLOT(timeout()));
	
	// 1 sec interval
        _timer->start( 1000, false );
}

LapsusSignal::~LapsusSignal()
{
}

void LapsusSignal::timeout()
{
	if (signalReceived && !startedExit)
	{
		startedExit = true;
		
		QApplication::exit(0);
	}
}
