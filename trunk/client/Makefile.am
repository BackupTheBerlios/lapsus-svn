SUBDIRS = pics

INCLUDES= $(all_includes) -I$(top_srcdir)/dbus-qt4-qt3backport -I$(top_srcdir)/global

METASOURCES = AUTO

lib_LTLIBRARIES = lapsus_panelapplet.la

lapsus_panelapplet_la_SOURCES = \
lapsus_dbus.cpp ../global/lapsus_validator.cpp \
lapsus_feature.cpp lapsus_switch.cpp lapsus_vol_slider.cpp lapsus_slider.cpp \
lapsus_icons.cpp flowlayout.cpp ksmallslider.cpp osd.cpp \
listbox_switch.cpp listbox_slider.cpp listbox_vol_slider.cpp listbox_feature.cpp \
panel_widget.cpp panel_slider.cpp panel_vol_slider.cpp panel_default.cpp \
panel_button.cpp action_button.cpp applet.cpp panel_main.cpp \
conf_base.ui lapsus_conf.cpp conf_dialog.cpp

lapsus_panelapplet_la_LDFLAGS = -module -avoid-version $(all_libraries)

lapsus_panelapplet_la_LIBADD = $(LIB_KDEUI) -ldbus-1 $(top_builddir)/dbus-qt4-qt3backport/libdbus-1-qt3.la

lapsus_applet_DATA = lapsusapplet.desktop
lapsus_appletdir = $(kde_datadir)/kicker/applets

