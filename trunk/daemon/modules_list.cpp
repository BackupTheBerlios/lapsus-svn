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

#include "modules_list.h"

LapsusModulesList::LapsusModulesList()
{
	mixer = 0;
	synaptics = 0;
}

LapsusModulesList::~LapsusModulesList()
{
	LapsusModule *mod;
	
	for (mod = modules.first(); mod; mod = modules.next())
	{
		delete mod;
	}
	
	modules.clear();
	prefixes.clear();
}

uint LapsusModulesList::count()
{
	return modules.count();
}

void LapsusModulesList::addConfig(LapsusConfig *mod)
{
	config = mod;
	addModule(mod);
}

void LapsusModulesList::addMixer(LapsusMixer *mod)
{
	mixer = mod;
	addModule(mod);
}

void LapsusModulesList::addSynaptics(LapsusSynaptics *mod)
{
	synaptics = mod;
	addModule(mod);
}

void LapsusModulesList::addModule(LapsusModule *mod)
{
	if (!mod) return;
	
	modules.append(mod);
	prefixes.insert(mod->modulePrefix(), mod);
}

bool LapsusModulesList::findModule(LapsusModule **mod, QString &id)
{
	int idx = id.find('.');
	
	if (idx < 1) return false;
	
	QString pref = id.left(idx);
	
	if ( (*mod = prefixes.find(pref.ascii())) == 0) return false;
	
	id = id.mid(idx+1);
	
	if (id.length() < 1) return false;
	
	return true;
}

LapsusModulesIterator LapsusModulesList::modulesIterator()
{
	return QPtrListIterator<LapsusModule>( modules );
}
