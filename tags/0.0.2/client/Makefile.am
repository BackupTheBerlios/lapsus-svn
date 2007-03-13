SUBDIRS = pics

INCLUDES= $(all_includes) -I$(top_srcdir)/dbus-qt4-qt3backport -I$(top_srcdir)/global

METASOURCES = AUTO

lib_LTLIBRARIES = lapsus_panelapplet.la

lapsus_panelapplet_la_SOURCES = colorwidget.ui ldbus.cpp switch.cpp applet_conf.cpp applet.cpp ksmallslider.cpp

lapsus_panelapplet_la_LDFLAGS = -module -avoid-version $(all_libraries)

lapsus_panelapplet_la_LIBADD = $(LIB_KDEUI) -ldbus-1 $(top_builddir)/dbus-qt4-qt3backport/libdbus-1-qt3.la

lapsus_applet_DATA = lapsusapplet.desktop
lapsus_appletdir = $(kde_datadir)/kicker/applets

