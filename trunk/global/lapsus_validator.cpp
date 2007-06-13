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

#include "lapsus_validator.h"

LapsusValidator::LapsusValidator()
{
}

LapsusValidator::LapsusValidator(const QStringList &validArgs): _valid(validArgs)
{
}

LapsusValidator::~LapsusValidator()
{
}

bool LapsusValidator::isValid(const QString &str)
{
	if (_valid.count() < 1) return false;
	
	bool isInt = false;
	int intVal = str.toInt(&isInt);
	int minV, maxV;

	for (QStringList::ConstIterator it = _valid.begin(); it != _valid.end(); ++it )
	{
		if (str == *it) return true;
		
		if (isInt && (*it).contains(':'))
		{
			QStringList list = QStringList::split(':', *it);
	
			if (list.size() == 2)
			{
				minV = list[0].toInt();
				maxV = list[1].toInt();
	
				if ( minV <= intVal && intVal <= maxV) return true;
			}
		}
	}
	
	return false;
}
