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

#ifndef ACPI_EVENT_PARSER_H
#define ACPI_EVENT_PARSER_H

#include <qsocketnotifier.h>
#include <qstring.h>

class ACPIEventParser : public QSocketNotifier
{
	Q_OBJECT

	public:
		ACPIEventParser(int acpi_fd);

	signals:
		void acpiEvent(const QString &group, const QString &action, const QString &device, uint id, uint value);

	private slots:
		void fdActivated(int fd);

	private:
		void parseEvent(const QString &str);
		QString alreadyRead;
};

#endif
