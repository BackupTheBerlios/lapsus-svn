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

#ifndef MODULES_LIST_H
#define MODULES_LIST_H

#include <qptrlist.h>
#include <qasciidict.h>

#include "lapsus_config.h"
#include "lapsus_mixer.h"
#include "synaptics.h"

#include "lapsus_module.h"


typedef QPtrListIterator<LapsusModule> LapsusModulesIterator;

class LapsusModulesList
{
	public:
		LapsusModulesList();
		~LapsusModulesList();
		
		void addModule(LapsusModule *mod);
		void addConfig(LapsusConfig *mod);
		void addMixer(LapsusMixer *mod);
		void addSynaptics(LapsusSynaptics *mod);
		
		bool findModule(LapsusModule **mod, QString &id);
		LapsusModulesIterator modulesIterator();
	
		uint count();
		
		LapsusConfig *config;
		LapsusMixer *mixer;
		LapsusSynaptics *synaptics;
		
	private:
		QPtrList<LapsusModule> _modules;
		QAsciiDict<LapsusModule> _prefixes;
};

#endif
