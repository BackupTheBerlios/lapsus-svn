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

#ifndef LAPSUS_GLOBAL_H
#define LAPSUS_GLOBAL_H

#define LAPSUS_VERSION			"0.0.2"

// Service name which will be used by server to register to D-BUS system bus.
#define LAPSUS_SERVICE_NAME		"de.berlios.Lapsus"

// This can not be "/" - It won't work with notifications enabled.
// Either this, or notifications will work, depending on which one was registered first.
#define LAPSUS_OBJECT_PATH		"/LapsusDaemon"

#define LAPSUS_INTERFACE		"de.berlios.Lapsus"
#endif
