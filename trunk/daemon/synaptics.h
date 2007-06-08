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

#ifndef LAPSUS_SYNAPTICS_H
#define LAPSUS_SYNAPTICS_H

#include <qobject.h>
#include <qprocess.h>

#include "lapsus_module.h"

class LapsusSynaptics : public LapsusModule
{
	Q_OBJECT

	public:
		LapsusSynaptics();
		~LapsusSynaptics();

		bool hardwareDetected();
		
		QStringList featureList();
		QStringList featureArgs(const QString &id);
		QString featureRead(const QString &id);
		bool featureWrite(const QString &id, const QString &nVal);
		
		bool getState();
		bool setState(bool nState, bool hardwareTrig = false);
		bool toggleState(bool hardwareTrig = false);

	protected:
		void stateChanged(bool nState);

	protected slots:
		void readFromStdout();
		void getProcessExited();
		void setProcessExited();

	private:
		QProcess* _procGet;
		QProcess* _procSet;
		bool _isValid;
		bool _isOn;
		QString _dataRead;
		bool _notifyChange;

		bool runGetProc();
		bool runSetProc(bool nState);
};

#endif
