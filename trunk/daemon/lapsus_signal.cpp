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

#include <qevent.h>
#include <qapplication.h>

#define SIGNAL_EVENT_ID		(QEvent::User + 15)

bool LapsusSignal::alreadyCreated = false;

static LapsusSignal *signalObj = 0;
static QEvent *eventToSend = 0;

volatile static sig_atomic_t signalReceived = 0;

void unixSignalHandler(int)
{
	if (!signalReceived && signalObj != 0 && eventToSend != 0)
	{
		signalReceived = 1;
		
		QApplication::postEvent(signalObj, eventToSend);
		
		signalObj = 0;
		eventToSend = 0;
	}
}

LapsusSignal::LapsusSignal(): startedExit(false)
{
	if (!alreadyCreated)
	{
		alreadyCreated = true;
		
		if (!signalObj)
		{
			signalObj = this;
		}
		
		if (!eventToSend)
		{
			eventToSend = new QCustomEvent(SIGNAL_EVENT_ID);
		}
		
		signal(SIGINT, unixSignalHandler);
		signal(SIGTERM, unixSignalHandler);
		signal(SIGQUIT, unixSignalHandler);
	}
}

LapsusSignal::~LapsusSignal()
{
}

void LapsusSignal::customEvent(QCustomEvent * e)
{
	if (!startedExit && e->type() == SIGNAL_EVENT_ID )
	{
		startedExit = true;
		QApplication::exit(0);
	}
}
