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

#include <qregexp.h>
#include <qstringlist.h>

#include "acpi_event_parser.h"

ACPIEventParser::ACPIEventParser(int acpi_fd)
	: QSocketNotifier(acpi_fd, QSocketNotifier::Read, 0 )
{
	alreadyRead = "";

	connect(this, SIGNAL(activated(int)), this, SLOT(fdActivated(int)));
}

void ACPIEventParser::fdActivated(int fd)
{
	char c;

	while (read(fd, &c, 1)>0)
	{
		if (c=='\n')
		{
			parseEvent(alreadyRead);
			alreadyRead = "";
		}
		else
		{
			alreadyRead.append(c);
		}
	}
}

void ACPIEventParser::parseEvent(const QString &str)
{
	QStringList list = QStringList::split(QRegExp("\\s+"), str, FALSE);

	if (list.size() != 4) return;

	QStringList list2 = QStringList::split("/", list[0], FALSE);

	if (list2.size() < 1) return;

	if (list2.size() == 1)
		list2.append(list2[0]);

	uint acpiId, acpiValue;
	bool ok;

	acpiId = list[2].toUInt(&ok, 16);

	if (!ok) return;

	acpiValue = list[3].toUInt(&ok, 16);

	if (!ok) return;

	emit acpiEvent(list2[0], list2[1], list[1], acpiId, acpiValue);
}
