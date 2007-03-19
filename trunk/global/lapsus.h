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

#define LAPSUS_VERSION			"0.0.4-dev"

// Service name which will be used by server to register to D-BUS system bus.
#define LAPSUS_SERVICE_NAME		"de.berlios.Lapsus"

// This can not be "/" - It won't work with notifications enabled.
// Either this, or notifications will work, depending on which one was registered first.
#define LAPSUS_OBJECT_PATH		"/LapsusDaemon"

#define LAPSUS_INTERFACE		"de.berlios.Lapsus"

#define LAPSUS_DBUS_LIST_FEATURES	"listFeatures"
#define LAPSUS_DBUS_GET_FEATURE_INFO	"getFeatureInfo"
#define LAPSUS_DBUS_GET_FEATURE		"getFeature"
#define LAPSUS_DBUS_SET_FEATURE		"setFeature"

#define LAPSUS_DBUS_ACPI_EVENT		"acpiEvent"
#define LAPSUS_DBUS_FEATURE_CHANGED	"featureChanged"
#define LAPSUS_DBUS_FEATURE_NOTIF	"featureNotif"

// Following values have to be lower-case!
#define LAPSUS_FEAT_LED_ID_PREFIX	"led_"
#define LAPSUS_FEAT_DISPLAY_ID_PREFIX	"display_"
#define LAPSUS_FEAT_BACKLIGHT_ID	"backlight"
#define LAPSUS_FEAT_BLUETOOTH_ID	"bluetooth"
#define LAPSUS_FEAT_WIRELESS_ID		"wireless"
#define LAPSUS_FEAT_VOLUME_ID		"volume"
#define LAPSUS_FEAT_TOUCHPAD_ID		"touchpad"

#define LAPSUS_FEAT_ON			"on"
#define LAPSUS_FEAT_OFF			"off"
#define LAPSUS_FEAT_BLINK		"blink"
#define LAPSUS_FEAT_MUTE		"mute"
#define LAPSUS_FEAT_UNMUTE		"unmute"

#define LAPSUS_FEAT_DISPLAY_LCD		"lcd"
#define LAPSUS_FEAT_DISPLAY_CRT		"crt"
#define LAPSUS_FEAT_DISPLAY_TV		"tv"
#define LAPSUS_FEAT_DISPLAY_DVI		"dvi"

#define LAPSUS_FEAT_PARAM_NOTIF		"notifs"

#endif
